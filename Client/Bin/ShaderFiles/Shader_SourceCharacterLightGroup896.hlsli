#ifndef SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight901(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[17]=float4(input.lightColor,1.0);
    source[18].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[12].xxxx
    r3.xy = ((r2.xyxx)*(source[12].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 23: add r1.w, r3.x, l(-0.333300)
    r1.w = ((r3.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[18].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[18].xxxx)) * 0xffffffffu)).w;
    // 27: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 28: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 29: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 31: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 32: else
    } else {
    // 33: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 38: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 39: mul r1.w, r6.x, cb0[14].x
    r1.w = ((r6.xxxx)*(source[14].xxxx)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[14].y
    r1.w = (saturate((r1.wwww)+(source[14].yyyy))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[9].xyzx
    r8.xyz = ((r2.wwww)*(source[9].xyzx)).xyz;
    // 45: mul r9.xyz, cb0[3].xyzx, cb0[3].wwww
    r9.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 46: max r10.xyz, r9.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r9.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 47: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 48: max r9.xyz, r9.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 49: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 50: mul r2.w, r6.y, cb0[12].y
    r2.w = ((r6.yyyy)*(source[12].yyyy)).w;
    // 51: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 52: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 54: add r6.xyw, -r10.xyxz, r9.xyxz
    r6.xyw = ((-(r10.xyxz))+(r9.xyxz)).xyw;
    // 55: mad r6.xyw, r2.wwww, r6.xyxw, r10.xyxz
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r10.xyxz)).xyw;
    // 56: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 57: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 60: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 61: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 62: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 64: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 65: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 66: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 67: max r9.xzw, r7.xxyw, l(0.002170, 0.000000, 0.002170, 0.002170)
    r9.xzw = (max(r7.xxyw,float4(0.002170,0.000000,0.002170,0.002170))).xzw;
    // 68: min r9.xzw, r9.xxzw, l(100.000000, 0.000000, 100.000000, 100.000000)
    r9.xzw = (min(r9.xxzw,float4(100.000000,0.000000,100.000000,100.000000))).xzw;
    // 69: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 70: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 71: add r7.xyw, -r9.xzxw, r7.xyxw
    r7.xyw = ((-(r9.xzxw))+(r7.xyxw)).xyw;
    // 72: mad r7.xyw, r2.wwww, r7.xyxw, r9.xzxw
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xzxw)).xyw;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 76: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 77: mad r6.xyw, cb0[12].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[12].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 78: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 80: mad r6.xyw, cb0[13].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[13].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 81: mad r7.xyw, cb0[7].wwww, cb0[7].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[7].wwww)*(source[7].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 82: mad r9.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 84: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 85: dp3 r3.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 86: add r9.xyz, -r3.yzwy, r3.xxxx
    r9.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 87: mad r3.xyz, cb0[12].wwww, r9.xyzx, r3.yzwy
    r3.xyz = ((source[12].wwww)*(r9.xyzx)+(r3.yzwy)).xyz;
    // 88: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 90: mad r3.xyz, cb0[13].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[13].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 91: mul r9.xyz, r3.xyzx, r6.xywx
    r9.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 92: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 94: mad r3.xyz, cb0[12].wwww, r3.xyzx, r9.xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 95: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 97: mad r3.xyz, cb0[13].xxxx, r6.xywx, r3.xyzx
    r3.xyz = ((source[13].xxxx)*(r6.xywx)+(r3.xyzx)).xyz;
    // 98: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 99: mul r3.w, cb0[6].z, l(1.500000)
    r3.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 100: add r4.w, -cb0[6].w, l(1.000000)
    r4.w = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: mul r4.w, r4.w, cb0[14].z
    r4.w = ((r4.wwww)*(source[14].zzzz)).w;
    // 102: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 103: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 104: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 106: mad r3.w, r3.w, l(0.500000), cb0[6].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 107: frc r4.w, cb0[6].x
    r4.w = (frac(source[6].xxxx)).w;
    // 108: add r5.w, -r4.w, cb0[6].x
    r5.w = ((-(r4.wwww))+(source[6].xxxx)).w;
    // 109: mul r9.z, r5.w, l(0.125000)
    r9.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 110: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 111: mul r9.y, cb0[6].y, cb0[10].y
    r9.y = ((source[6].yyyy)*(source[10].yyyy)).y;
    // 112: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 113: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 114: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 115: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 116: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 117: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 118: mul r6.xyw, r3.wwww, r9.xyxz
    r6.xyw = ((r3.wwww)*(r9.xyxz)).xyw;
    // 119: mul r3.w, r4.w, r9.w
    r3.w = ((r4.wwww)*(r9.wwww)).w;
    // 120: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 121: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 122: add r3.w, r3.y, r3.x
    r3.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 123: add r3.w, r3.z, r3.w
    r3.w = ((r3.zzzz)+(r3.wwww)).w;
    // 124: mul r3.w, r3.w, l(0.333330)
    r3.w = ((r3.wwww)*(float4(0.333330,0.333330,0.333330,0.333330))).w;
    // 125: max r3.w, r3.w, cb0[15].x
    r3.w = (max(r3.wwww,source[15].xxxx)).w;
    // 126: min r3.w, r3.w, cb0[14].w
    r3.w = (min(r3.wwww,source[14].wwww)).w;
    // 127: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mad r3.w, r2.w, r4.w, r3.w
    r3.w = ((r2.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 129: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 130: mad r3.w, cb0[15].z, r3.w, l(1.000000)
    r3.w = ((source[15].zzzz)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r6.xyw, r3.xyxz, r3.wwww
    r6.xyw = ((r3.xyxz)*(r3.wwww)).xyw;
    // 132: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 133: mad r3.xyz, r3.wwww, r3.xyzx, -r6.xywx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(-(r6.xywx))).xyz;
    // 134: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 135: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 136: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 137: mov_sat r1.w, cb0[15].w
    r1.w = (saturate(source[15].wwww)).w;
    // 138: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 139: mul r3.w, r6.z, cb0[16].x
    r3.w = ((r6.zzzz)*(source[16].xxxx)).w;
    // 140: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 141: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 143: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 144: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 146: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 148: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 149: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 150: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 151: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 152: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 154: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 155: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 158: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 159: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 161: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 162: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 163: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 164: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 165: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 167: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 168: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 169: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 170: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 171: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 172: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 173: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 174: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 175: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 176: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 177: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 178: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 179: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 181: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 182: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 183: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 184: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 185: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 187: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 188: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 189: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 190: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 192: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 193: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 194: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 195: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 196: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 197: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 198: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 199: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 200: mul_sat r6.xyz, cb0[11].xyzx, cb0[11].wwww
    r6.xyz = (saturate((source[11].xyzx)*(source[11].wwww))).xyz;
    // 201: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 202: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 203: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 204: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 205: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 206: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 207: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 208: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 209: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 210: mul r0.x, r0.x, cb0[16].y
    r0.x = ((r0.xxxx)*(source[16].yyyy)).x;
    // 211: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 212: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 213: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 215: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 216: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 217: mul o0.xyz, r0.xyzx, cb0[17].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[17].xyzx)).xyz;
    // 218: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 219: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 220: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 221: ret
    return output;
}

// source.character.equipment-native-902.v1 / source program 6e2644a82381184ba5aebab6315fb198
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight902(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16].y=(g_SourceCharacterTime.xxxx).x;
    source[19]=float4(input.lightColor,1.0);
    source[20].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[13].xxxx
    r3.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 23: add r1.w, r3.x, l(-0.333300)
    r1.w = ((r3.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[20].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[20].xxxx)) * 0xffffffffu)).w;
    // 27: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 28: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 29: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 30: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 31: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 32: else
    } else {
    // 33: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 38: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 39: mul r1.w, r6.x, cb0[15].y
    r1.w = ((r6.xxxx)*(source[15].yyyy)).w;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 42: add_sat r1.w, r1.w, cb0[15].z
    r1.w = (saturate((r1.wwww)+(source[15].zzzz))).w;
    // 43: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 45: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 46: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 47: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 49: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 50: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 51: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 52: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 53: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 54: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 55: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 56: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 57: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 58: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 59: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 60: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 61: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 62: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 64: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 65: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 66: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 67: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 68: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 69: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 70: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 71: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 72: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 73: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 74: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 75: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 76: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 77: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 78: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 79: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 80: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 81: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 82: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 83: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 84: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 85: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 86: mad r6.xyw, cb0[14].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 87: dp3 r3.x, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r7.xyw, -r6.xyxw, r3.xxxx
    r7.xyw = ((-(r6.xyxw))+(r3.xxxx)).xyw;
    // 89: mad r6.xyw, cb0[14].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 90: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 91: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 93: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 94: dp3 r3.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 95: add r10.xyz, -r3.yzwy, r3.xxxx
    r10.xyz = ((-(r3.yzwy))+(r3.xxxx)).xyz;
    // 96: mad r3.xyz, cb0[14].xxxx, r10.xyzx, r3.yzwy
    r3.xyz = ((source[14].xxxx)*(r10.xyzx)+(r3.yzwy)).xyz;
    // 97: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 98: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 99: mad r3.xyz, cb0[14].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[14].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 100: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 101: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[14].xxxx, r3.xyzx, r10.xyzx
    r3.xyz = ((source[14].xxxx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 104: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 106: mad r3.xyz, cb0[14].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[14].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 107: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 108: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 109: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r4.w, r4.w, cb0[16].y
    r4.w = ((r4.wwww)*(source[16].yyyy)).w;
    // 111: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 112: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 113: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 115: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 116: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 117: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 118: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 119: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 120: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 121: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 122: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 123: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 124: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 125: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 126: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 127: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 128: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 129: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 130: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 131: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 132: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 133: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 134: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 135: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 136: mov_sat r1.w, cb0[16].z
    r1.w = (saturate(source[16].zzzz)).w;
    // 137: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 138: add r3.w, -cb0[17].z, cb0[17].y
    r3.w = ((-(source[17].zzzz))+(source[17].yyyy)).w;
    // 139: mad r3.w, r9.x, r3.w, cb0[17].z
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[17].zzzz)).w;
    // 140: add r4.w, -r3.w, cb0[18].x
    r4.w = ((-(r3.wwww))+(source[18].xxxx)).w;
    // 141: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 142: add r4.w, -r3.w, cb0[18].z
    r4.w = ((-(r3.wwww))+(source[18].zzzz)).w;
    // 143: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 144: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 145: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 146: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 148: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 149: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 151: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 152: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 153: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 154: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 155: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 156: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 157: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 159: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 160: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 163: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 164: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 166: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 167: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 168: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 169: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 170: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 172: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 173: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 174: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 175: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 176: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 177: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 178: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 179: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 180: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 181: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 182: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 183: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 184: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 186: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 187: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 188: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 189: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 190: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 191: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 192: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 193: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 194: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 195: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 197: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 198: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 199: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 200: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 201: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 202: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 203: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 204: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 205: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 206: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 207: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 208: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 209: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 210: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 211: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 212: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 213: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 214: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 215: mul r0.x, r0.x, cb0[18].w
    r0.x = ((r0.xxxx)*(source[18].wwww)).x;
    // 216: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 217: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 218: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 219: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 220: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 221: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 222: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 223: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 225: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 226: ret
    return output;
}

// source.character.equipment-native-903.v1 / source program 3b8d1e6d4da9b648af3811a94ea1a621
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight903(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12].x=(g_SourceCharacterTime.xxxx).x;
    source[18]=float4(input.lightColor,1.0);
    source[19].x=1.0;
    source[0].y=1.f; // Engine primitive opacity, not a MIC uniform.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r3.xy, r2.xyxx, cb0[11].xxxx
    r3.xy = ((r2.xyxx)*(source[11].xxxx)).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 16: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 17: div r2.xyz, r3.xyzx, r1.wwww
    r2.xyz = ((r3.xyzx)/(r1.wwww)).xyz;
    // 18: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 21: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[19].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[19].xxxx)) * 0xffffffffu)).w;
    // 22: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 23: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 24: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 25: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 26: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 27: else
    } else {
    // 28: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 29: endif
    }
    // 30: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: lt r6.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 33: log r5.xyz, |r5.xzyx|
    r5.xyz = (log2(abs(r5.xzyx))).xyz;
    // 34: mul r1.w, r5.x, cb0[14].x
    r1.w = ((r5.xxxx)*(source[14].xxxx)).w;
    // 35: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 36: movc r1.w, r6.x, l(0), r1.w
    r1.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 37: add_sat r1.w, r1.w, cb0[14].y
    r1.w = (saturate((r1.wwww)+(source[14].yyyy))).w;
    // 38: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r7.xyz, r2.wwww, cb0[9].xyzx
    r7.xyz = ((r2.wwww)*(source[9].xyzx)).xyz;
    // 40: add r2.w, r1.w, l(-1.000000)
    r2.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 41: mad r2.w, cb0[14].w, r2.w, l(1.000000)
    r2.w = ((source[14].wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r8.xyz, cb0[2].xyzx, cb0[2].wwww
    r8.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 43: max r9.xyz, r8.xyzx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r8.xyzx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 44: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 45: max r8.xyz, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 46: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 47: mul r3.w, r5.y, cb0[11].y
    r3.w = ((r5.yyyy)*(source[11].yyyy)).w;
    // 48: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 49: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: movc r3.w, r6.y, l(0), r3.w
    r3.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 51: add r5.xyw, -r9.xyxz, r8.xyxz
    r5.xyw = ((-(r9.xyxz))+(r8.xyxz)).xyw;
    // 52: mad r5.xyw, r3.wwww, r5.xyxw, r9.xyxz
    r5.xyw = ((r3.wwww)*(r5.xyxw)+(r9.xyxz)).xyw;
    // 53: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 54: max r8.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r8.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 55: min r8.xyz, r8.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 56: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 57: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 58: add r6.xyw, -r8.xyxz, r6.xyxw
    r6.xyw = ((-(r8.xyxz))+(r6.xyxw)).xyw;
    // 59: mad r6.xyw, r3.wwww, r6.xyxw, r8.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r8.xyxz)).xyw;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t2.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 61: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 62: mad r5.xyw, r8.xxxx, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.xxxx)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 63: mul r6.xyw, cb0[4].xyxz, cb0[4].wwww
    r6.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 64: max r9.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 65: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 66: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 67: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 68: add r6.xyw, -r9.xyxz, r6.xyxw
    r6.xyw = ((-(r9.xyxz))+(r6.xyxw)).xyw;
    // 69: mad r6.xyw, r3.wwww, r6.xyxw, r9.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r9.xyxz)).xyw;
    // 70: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 71: mad r5.xyw, r8.yyyy, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.yyyy)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 72: mul r6.xyw, cb0[5].xyxz, cb0[5].wwww
    r6.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 73: max r9.xyz, r6.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r6.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 74: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 75: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 76: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 77: add r6.xyw, -r9.xyxz, r6.xyxw
    r6.xyw = ((-(r9.xyxz))+(r6.xyxw)).xyw;
    // 78: mad r6.xyw, r3.wwww, r6.xyxw, r9.xyxz
    r6.xyw = ((r3.wwww)*(r6.xyxw)+(r9.xyxz)).xyw;
    // 79: add r6.xyw, -r5.xyxw, r6.xyxw
    r6.xyw = ((-(r5.xyxw))+(r6.xyxw)).xyw;
    // 80: mad r5.xyw, r8.zzzz, r6.xyxw, r5.xyxw
    r5.xyw = ((r8.zzzz)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 81: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 82: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 83: mad r5.xyw, cb0[13].zzzz, r6.xyxw, r5.xyxw
    r5.xyw = ((source[13].zzzz)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 84: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 86: mad r5.xyw, cb0[13].wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((source[13].wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 87: mad r6.xyw, cb0[7].wwww, cb0[7].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[7].wwww)*(source[7].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 88: mad r9.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 89: mul r6.xyw, r6.xyxw, r9.xyxz
    r6.xyw = ((r6.xyxw)*(r9.xyxz)).xyw;
    // 90: mul r5.xyw, r5.xyxw, r6.xyxw
    r5.xyw = ((r5.xyxw)*(r6.xyxw)).xyw;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.wxyz, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 92: dp3 r4.w, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r10.xyz, -r9.yzwy, r4.wwww
    r10.xyz = ((-(r9.yzwy))+(r4.wwww)).xyz;
    // 94: mad r9.yzw, cb0[13].zzzz, r10.xxyz, r9.yyzw
    r9.yzw = ((source[13].zzzz)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 95: dp3 r4.w, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r10.xyz, -r9.yzwy, r4.wwww
    r10.xyz = ((-(r9.yzwy))+(r4.wwww)).xyz;
    // 97: mad r9.yzw, cb0[13].wwww, r10.xxyz, r9.yyzw
    r9.yzw = ((source[13].wwww)*(r10.xxyz)+(r9.yyzw)).yzw;
    // 98: mul r10.xyz, r5.xywx, r9.yzwy
    r10.xyz = ((r5.xywx)*(r9.yzwy)).xyz;
    // 99: dp3 r4.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r5.xyw, -r5.xyxw, r9.yzyw, r4.wwww
    r5.xyw = ((-(r5.xyxw))*(r9.yzyw)+(r4.wwww)).xyw;
    // 101: mad r5.xyw, cb0[13].zzzz, r5.xyxw, r10.xyxz
    r5.xyw = ((source[13].zzzz)*(r5.xyxw)+(r10.xyxz)).xyw;
    // 102: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r9.yzw, -r5.xxyw, r4.wwww
    r9.yzw = ((-(r5.xxyw))+(r4.wwww)).yzw;
    // 104: mad r5.xyw, cb0[13].wwww, r9.yzyw, r5.xyxw
    r5.xyw = ((source[13].wwww)*(r9.yzyw)+(r5.xyxw)).xyw;
    // 105: mul r5.xyw, r6.xyxw, r5.xyxw
    r5.xyw = ((r6.xyxw)*(r5.xyxw)).xyw;
    // 106: mul r4.w, cb0[6].z, l(1.500000)
    r4.w = ((source[6].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 107: add r6.x, -cb0[6].w, l(1.000000)
    r6.x = ((-(source[6].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r6.x, r6.x, cb0[12].x
    r6.x = ((r6.xxxx)*(source[12].xxxx)).x;
    // 109: mul r6.x, r6.x, l(6.283185)
    r6.x = ((r6.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 110: sincos r6.x, null, r6.x
    r6.x = (sin(r6.xxxx)).x;
    // 111: add r6.x, r6.x, l(1.000000)
    r6.x = ((r6.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 112: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 113: mad r4.w, r4.w, l(0.500000), cb0[6].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 114: frc r6.x, cb0[6].x
    r6.x = (frac(source[6].xxxx)).x;
    // 115: add r6.y, -r6.x, cb0[6].x
    r6.y = ((-(r6.xxxx))+(source[6].xxxx)).y;
    // 116: mul r10.z, r6.y, l(0.125000)
    r10.z = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 117: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 118: mul r10.y, cb0[6].y, cb0[10].y
    r10.y = ((source[6].yyyy)*(source[10].yyyy)).y;
    // 119: frc r6.y, v4.x
    r6.y = (frac(v4.xxxx)).y;
    // 120: mul r11.x, r6.y, l(0.125000)
    r11.x = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 121: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 122: add r6.yw, r10.xxxy, r11.xxxy
    r6.yw = ((r10.xxxy)+(r11.xxxy)).yw;
    // 123: add r6.yw, r6.yyyw, r10.zzzw
    r6.yw = ((r6.yyyw)+(r10.zzzw)).yw;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.ywyy, t4.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 125: mul r9.yzw, r4.wwww, r10.xxyz
    r9.yzw = ((r4.wwww)*(r10.xxyz)).yzw;
    // 126: mul r4.w, r6.x, r10.w
    r4.w = ((r6.xxxx)*(r10.wwww)).w;
    // 127: mad r6.xyw, r9.yzyw, l(2.000000, 2.000000, 0.000000, 2.000000), -r5.xyxw
    r6.xyw = ((r9.yzyw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r5.xyxw))).xyw;
    // 128: mad r5.xyw, r4.wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((r4.wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 129: mul r6.xyw, r2.wwww, r5.xyxw
    r6.xyw = ((r2.wwww)*(r5.xyxw)).xyw;
    // 130: mul r6.xyw, r6.xyxw, r7.xyxz
    r6.xyw = ((r6.xyxw)*(r7.xyxz)).xyw;
    // 131: mad r5.xyw, r2.wwww, r5.xyxw, -r6.xyxw
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(-(r6.xyxw))).xyw;
    // 132: mad r5.xyw, r1.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 133: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 134: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 135: mov_sat r1.w, cb0[15].x
    r1.w = (saturate(source[15].xxxx)).w;
    // 136: mul_sat r2.w, r3.w, cb2[3].w
    r2.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 137: add r3.w, -cb0[16].y, cb0[16].x
    r3.w = ((-(source[16].yyyy))+(source[16].xxxx)).w;
    // 138: mad r3.w, r8.x, r3.w, cb0[16].y
    r3.w = ((r8.xxxx)*(r3.wwww)+(source[16].yyyy)).w;
    // 139: add r4.w, -r3.w, cb0[16].w
    r4.w = ((-(r3.wwww))+(source[16].wwww)).w;
    // 140: mad r3.w, r8.y, r4.w, r3.w
    r3.w = ((r8.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 141: add r4.w, -r3.w, cb0[17].y
    r4.w = ((-(r3.wwww))+(source[17].yyyy)).w;
    // 142: mad r3.w, r8.z, r4.w, r3.w
    r3.w = ((r8.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 143: mul r3.w, r5.z, r3.w
    r3.w = ((r5.zzzz)*(r3.wwww)).w;
    // 144: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 145: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: movc r3.w, r6.z, l(0), r3.w
    r3.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 147: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 148: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 150: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 151: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 152: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 153: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 154: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 155: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 156: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: dp3_sat r1.x, r2.xyzx, r1.xyzx
    r1.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 158: dp3_sat r0.x, r0.xyzx, r5.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 159: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 160: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 161: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 162: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 163: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 164: mad r0.yzw, -r4.xxyz, r2.wwww, r4.xxyz
    r0.yzw = ((-(r4.xxyz))*(r2.wwww)+(r4.xxyz)).yzw;
    // 165: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 166: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 167: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 168: mad r2.x, r4.w, r1.z, -r4.w
    r2.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 169: mad r2.x, r2.x, r4.w, l(1.000000)
    r2.x = ((r2.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 170: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 171: mul r2.x, r2.x, l(3.141593)
    r2.x = ((r2.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 172: div r1.z, r1.z, r2.x
    r1.z = ((r1.zzzz)/(r2.xxxx)).z;
    // 173: mad r2.x, -r3.w, r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 174: mad r2.y, r5.w, r2.x, r1.y
    r2.y = ((r5.wwww)*(r2.xxxx)+(r1.yyyy)).y;
    // 175: mad r1.y, r1.x, r2.x, r1.y
    r1.y = ((r1.xxxx)*(r2.xxxx)+(r1.yyyy)).y;
    // 176: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 177: mad r1.y, r1.x, r2.y, r1.y
    r1.y = ((r1.xxxx)*(r2.yyyy)+(r1.yyyy)).y;
    // 178: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 179: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 180: mul r1.z, r1.w, l(0.080000)
    r1.z = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 181: mad r2.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r2.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 182: mad r2.xyz, r2.wwww, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r1.zzzz)).xyz;
    // 183: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 185: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 186: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 187: mul_sat r1.z, r2.y, l(50.000000)
    r1.z = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 188: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 189: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: max r4.xyz, r2.xyzx, r1.wwww
    r4.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 191: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 192: mad r2.xyz, -r0.xxxx, r2.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xxxx))*(r2.xyzx)+(r2.xyzx)).xyz;
    // 193: mad r2.xyz, r1.zzzz, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 194: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 196: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 197: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 198: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 199: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 200: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 201: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 202: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 204: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 205: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 206: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 207: mov_sat r9.x, r9.x
    r9.x = (saturate(r9.xxxx)).x;
    // 208: mul_sat r0.x, r9.x, cb0[15].y
    r0.x = (saturate((r9.xxxx)*(source[15].yyyy))).x;
    // 209: mul o0.w, r0.x, cb0[0].y
    output.targets[0].w = ((r0.xxxx)*(source[0].yyyy)).w;
    // 210: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 211: ret
    return output;
}

// source.character.equipment-native-904.v1 / source program d3f2392d0ec92742bcb294e4ca17c6d2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight904(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[1]=float4(input.lightColor,1.0);
    source[2].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: dp3 r0.y, v3.xyzx, v3.xyzx
    r0.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 4: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 5: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[2].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[2].xxxx)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 9: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t1.xyzw, s0
    r1.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 16: max r1.w, r0.w, l(0.000000)
    r1.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 17: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: dp3 r3.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 19: mul r3.x, r3.x, cb0[0].x
    r3.x = ((r3.xxxx)*(source[0].xxxx)).x;
    // 20: mad r0.xyz, v5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 21: dp3 r0.x, r0.xyzx, r0.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 22: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 23: div r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)/(r0.xxxx)).x;
    // 24: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 25: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 26: mul r0.x, r0.x, cb0[0].y
    r0.x = ((r0.xxxx)*(source[0].yyyy)).x;
    // 27: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 28: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 30: mul_sat r0.x, r3.x, r0.x
    r0.x = (saturate((r3.xxxx)*(r0.xxxx))).x;
    // 31: mad r0.xyz, r2.wwww, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xxxx)).xyz;
    // 32: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 33: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 34: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 35: mul o0.xyz, r0.xyzx, cb0[1].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 36: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 37: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 38: ret
    return output;
}

// source.character.mokoko-av036-905.v1 / source program c9cc424f33404648b77b8413ec31eb75
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight905(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].y=(g_SourceCharacterTime.xxxx).x;
    source[19].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[20].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[20].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[25].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[25].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.wxyz, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 32: add r1.w, r8.x, l(-0.333300)
    r1.w = ((r8.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 33: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 34: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 37: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 38: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r10.xy, r9.xyxx, cb0[14].xxxx
    r10.xy = ((r9.xyxx)*(source[14].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[14].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[14].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[16].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[16].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 49: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 53: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 54: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: add r1.x, -cb0[15].y, cb0[15].x
    r1.x = ((-(source[15].yyyy))+(source[15].xxxx)).x;
    // 70: mad r1.x, r11.w, r1.x, cb0[15].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[15].yyyy)).x;
    // 71: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 72: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 73: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 74: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 75: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 76: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 77: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 78: add r1.y, -r1.x, cb0[16].y
    r1.y = ((-(r1.xxxx))+(source[16].yyyy)).y;
    // 79: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 80: mul r1.x, r1.x, cb0[16].z
    r1.x = ((r1.xxxx)*(source[16].zzzz)).x;
    // 81: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 82: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 83: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 84: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 85: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[16].w
    r0.x = (1.0/(source[16].wwww)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r12.xyz, r4.xyzx, cb0[16].wwww
    r12.xyz = ((r4.xyzx)*(source[16].wwww)).xyz;
    // 100: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r12.xyzx, cb0[16].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[16].wwww)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[16].w, l(1.000000)
    r0.x = ((source[16].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r13.xyz, r12.xyzx, cb0[20].wwww
    r13.xyz = ((r12.xyzx)*(source[20].wwww)).xyz;
    // 129: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 130: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 132: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 133: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 134: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 135: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 136: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 137: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 138: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 139: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 140: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 142: dp3 r6.w, cb0[13].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[13].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 143: add r7.xyz, r6.wwww, -cb0[13].xyzx
    r7.xyz = ((r6.wwww)+(-(source[13].xyzx))).xyz;
    // 144: mad r7.xyz, r5.wwww, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[13].xyzx)).xyz;
    // 145: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 146: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 147: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 148: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 149: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 150: add_sat r4.z, r11.w, cb0[21].x
    r4.z = (saturate((r11.wwww)+(source[21].xxxx))).z;
    // 151: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 153: mul_sat r6.xy, r6.xzxx, cb0[17].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[17].zzzz))).xy;
    // 154: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 155: add_sat r6.y, r6.y, -cb0[17].w
    r6.y = (saturate((r6.yyyy)+(-(source[17].wwww)))).y;
    // 156: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 157: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 158: mul r6.y, r6.y, cb0[18].x
    r6.y = ((r6.yyyy)*(source[18].xxxx)).y;
    // 159: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 160: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 161: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 162: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 163: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 164: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 165: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 166: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 167: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 168: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 169: mul r8.x, r7.w, cb0[21].y
    r8.x = ((r7.wwww)*(source[21].yyyy)).x;
    // 170: mad r0.z, -r7.w, cb0[21].y, r0.z
    r0.z = ((-(r7.wwww))*(source[21].yyyy)+(r0.zzzz)).z;
    // 171: mad r0.z, r11.w, r0.z, r8.x
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.xxxx)).z;
    // 172: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 173: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 174: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 175: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 176: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 177: mad r6.yzw, -cb0[20].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 178: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 179: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 180: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 181: mad r10.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r10.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 182: mad r7.xyz, cb0[15].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 183: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r7.xyz, cb0[15].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[15].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 186: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 189: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 190: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[6].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[6].xyzx)).xyz;
    // 191: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 192: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r12.xyz, -r8.yzwy, r0.zzzz
    r12.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 194: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.yzwy
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 195: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r8.xyz, cb0[15].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 198: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 199: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 201: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 202: add r1.yzw, -cb0[7].xxyz, cb0[8].xxyz
    r1.yzw = ((-(source[7].xxyz))+(source[8].xxyz)).yzw;
    // 203: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[7].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[7].xyzx)).xyz;
    // 204: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 205: mul r1.xyz, r1.xyzx, cb0[17].xxxx
    r1.xyz = ((r1.xyzx)*(source[17].xxxx)).xyz;
    // 206: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 207: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r14.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 210: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 211: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 212: mad r14.xyz, cb0[15].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[15].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 213: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 214: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 215: mul r15.xyz, r15.xyzx, cb0[17].yyyy
    r15.xyz = ((r15.xyzx)*(source[17].yyyy)).xyz;
    // 216: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 217: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 218: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 219: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 220: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 221: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 222: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 223: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 224: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 225: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 226: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 227: add r1.w, -cb0[18].z, cb0[18].y
    r1.w = ((-(source[18].zzzz))+(source[18].yyyy)).w;
    // 228: mad r1.w, r11.w, r1.w, cb0[18].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[18].zzzz)).w;
    // 229: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 230: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 231: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 232: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 234: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 235: div r1.w, cb0[18].w, r1.w
    r1.w = ((source[18].wwww)/(r1.wwww)).w;
    // 236: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 237: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 238: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r1.w, r1.w, cb0[19].x
    r1.w = ((r1.wwww)*(source[19].xxxx)).w;
    // 240: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 241: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 242: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 243: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 244: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 245: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 246: mad r1.xyz, cb0[15].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 247: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 248: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 249: mad r1.xyz, cb0[15].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[15].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 251: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 252: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 253: mul r4.z, r4.z, cb0[19].y
    r4.z = ((r4.zzzz)*(source[19].yyyy)).z;
    // 254: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 255: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 256: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 258: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 259: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 260: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 261: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 262: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 263: mul r10.y, cb0[2].y, cb0[9].y
    r10.y = ((source[2].yyyy)*(source[9].yyyy)).y;
    // 264: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 265: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 266: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 267: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 268: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 271: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 272: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 273: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 274: mul r1.w, cb0[10].y, cb0[19].y
    r1.w = ((source[10].yyyy)*(source[19].yyyy)).w;
    // 275: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 276: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 277: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 278: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 279: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 280: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 281: mad r3.xy, cb0[10].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[10].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 282: mul r3.z, cb0[10].x, l(0.001000)
    r3.z = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 283: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 284: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 285: dp2 r3.z, cb0[11].xyxx, r3.xyxx
    r3.z = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 286: dp2 r3.y, cb0[12].xyxx, r3.xyxx
    r3.y = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 287: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 288: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 289: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 290: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 291: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 292: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 293: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 294: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 295: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 296: mul r10.xyz, r3.xyzx, cb0[10].zzzz
    r10.xyz = ((r3.xyzx)*(source[10].zzzz)).xyz;
    // 297: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 298: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 299: mad r3.xyz, cb0[10].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[10].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 300: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 301: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 302: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 303: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 304: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 305: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 306: mul r0.z, r2.w, cb0[21].z
    r0.z = ((r2.wwww)*(source[21].zzzz)).z;
    // 307: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 308: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 309: min r0.w, r0.w, cb0[21].z
    r0.w = (min(r0.wwww,source[21].zzzz)).w;
    // 310: add r1.w, -cb0[22].y, cb0[22].x
    r1.w = ((-(source[22].yyyy))+(source[22].xxxx)).w;
    // 311: mad r1.w, cb0[21].w, r1.w, cb0[22].y
    r1.w = ((source[21].wwww)*(r1.wwww)+(source[22].yyyy)).w;
    // 312: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 313: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 314: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 315: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 316: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 317: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 318: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 319: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 320: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 321: add r0.y, cb0[22].w, -cb0[23].x
    r0.y = ((source[22].wwww)+(-(source[23].xxxx))).y;
    // 322: mad r0.y, cb0[22].z, r0.y, cb0[23].x
    r0.y = ((source[22].zzzz)*(r0.yyyy)+(source[23].xxxx)).y;
    // 323: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 324: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t6.xwyz, s7, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 325: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 326: add r0.w, -cb0[23].y, l(2.000000)
    r0.w = ((-(source[23].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 327: mad r0.w, r4.x, r0.w, cb0[23].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[23].yyyy)).w;
    // 328: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 329: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 330: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 331: mul r0.xyz, r0.xyzx, cb0[23].zzzz
    r0.xyz = ((r0.xyzx)*(source[23].zzzz)).xyz;
    // 332: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 333: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 334: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 335: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 336: mul r0.xyz, r0.xyzx, cb0[23].wwww
    r0.xyz = ((r0.xyzx)*(source[23].wwww)).xyz;
    // 337: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 338: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 339: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 340: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 341: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 342: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 343: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 344: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 345: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 346: ret
    return output;
}

// source.character.mokoko-av036-906.v1 / source program ec95e757c223da4a9cbbd256b4eb6495
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight906(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].y=(g_SourceCharacterTime.xxxx).x;
    source[15].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[15].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[16].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[16].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[18]=float4(input.lightColor,1.0);
    source[24].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].yyyy)) * 0xffffffffu)).w;
    // 6: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 7: mul r2.xyzw, v8.yyyy, cb1[1].xyzw
    r2.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // 8: mad r2.xyzw, cb1[0].xyzw, v8.xxxx, r2.xyzw
    r2.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r2.xyzw)).xyzw;
    // 9: mad r2.xyzw, cb1[2].xyzw, v8.zzzz, r2.xyzw
    r2.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r2.xyzw)).xyzw;
    // 10: mad r2.xyzw, cb1[3].xyzw, v8.wwww, r2.xyzw
    r2.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r2.xyzw)).xyzw;
    // 11: mul r3.xyzw, r2.yyyy, cb0[20].xyzw
    r3.xyzw = ((r2.yyyy)*(source[20].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[19].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[19].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[21].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[21].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[22].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[22].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[23].wwzw
    r4.yz = (source[23].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[23].zwzz
    r4.xy = ((r2.xyxx)+(source[23].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[23].xyxx
    r2.xy = ((r2.xyxx)*(source[23].xyxx)).xy;
    // 27: frc r2.xy, r2.xyxx
    r2.xy = (frac(r2.xyxx)).xy;
    // 28: movc r2.zw, r3.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r2.zw = ((asuint(r3.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 29: add r2.zw, r2.zzzw, r4.zzzw
    r2.zw = ((r2.zzzw)+(r4.zzzw)).zw;
    // 30: mad r2.xz, r2.xxxx, r2.zzwz, r4.xxyx
    r2.xz = ((r2.xxxx)*(r2.zzwz)+(r4.xxyx)).xz;
    // 31: add r0.w, -r2.x, r2.z
    r0.w = ((-(r2.xxxx))+(r2.zzzz)).w;
    // 32: mad r0.w, r2.y, r0.w, r2.x
    r0.w = ((r2.yyyy)*(r0.wwww)+(r2.xxxx)).w;
    // 33: mul r2.xyz, r0.wwww, cb0[24].xxxx
    r2.xyz = ((r0.wwww)*(source[24].xxxx)).xyz;
    // 34: else
    } else {
    // 35: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 36: endif
    }
    // 37: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 38: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 39: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 40: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 41: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 42: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 43: add r5.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r5.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[13].xxxx
    r5.xyz = ((r5.xyzx)*(source[13].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r5.xyz, r6.yyyy, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r6.yyyy)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 47: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 49: mad r5.xyz, cb0[13].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 50: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 52: mad r5.xyz, cb0[13].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 53: mad r7.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r8.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 56: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 57: mul r9.xyz, r6.xxxx, r8.xyzx
    r9.xyz = ((r6.xxxx)*(r8.xyzx)).xyz;
    // 58: mad r8.xyz, r6.xxxx, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = ((r6.xxxx)*(r8.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 59: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 60: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 61: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 62: mul r8.xyz, r8.xyzx, cb0[7].xyzx
    r8.xyz = ((r8.xyzx)*(source[7].xyzx)).xyz;
    // 63: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 65: mad r10.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 66: dp2 r2.w, r10.xyxx, r10.xyxx
    r2.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 67: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 69: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 70: add r10.z, r2.w, l(0.000010)
    r10.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 71: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 72: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 73: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 74: dp3 r2.w, r10.xyzx, r3.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 75: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 77: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 78: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 79: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.w, cb0[8].z, l(1.500000)
    r2.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 83: add r3.w, -cb0[8].w, l(1.000000)
    r3.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r3.w, r3.w, cb0[15].y
    r3.w = ((r3.wwww)*(source[15].yyyy)).w;
    // 85: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 86: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 87: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 89: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 90: frc r3.w, cb0[8].x
    r3.w = (frac(source[8].xxxx)).w;
    // 91: add r4.w, -r3.w, cb0[8].x
    r4.w = ((-(r3.wwww))+(source[8].xxxx)).w;
    // 92: mul r11.z, r4.w, l(0.125000)
    r11.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 93: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 94: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 95: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 96: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 97: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 98: add r6.xy, r6.xyxx, r11.xyxx
    r6.xy = ((r6.xyxx)+(r11.xyxx)).xy;
    // 99: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 102: mul r2.w, r3.w, r11.w
    r2.w = ((r3.wwww)*(r11.wwww)).w;
    // 103: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 104: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 105: mul r2.w, cb0[10].y, cb0[15].y
    r2.w = ((source[10].yyyy)*(source[15].yyyy)).w;
    // 106: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 107: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 108: mul r6.y, r2.w, l(0.020000)
    r6.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 109: add r11.xyzw, r1.yzxy, -cb0[1].yzxy
    r11.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 110: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 111: add r11.xy, -r11.zwzz, r11.xyxx
    r11.xy = ((-(r11.zwzz))+(r11.xyxx)).xy;
    // 112: mad r11.xy, cb0[10].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[10].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 113: mul r3.w, cb0[10].x, l(0.001000)
    r3.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 114: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 115: mad r6.xy, r3.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r3.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 116: dp2 r3.w, cb0[11].xyxx, r6.xyxx
    r3.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 117: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 118: frc r3.w, r3.w
    r3.w = (frac(r3.wwww)).w;
    // 119: mul r6.x, r3.w, l(0.125000)
    r6.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 120: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 121: mul r3.w, r11.w, l(0.900000)
    r3.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 122: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 123: mad r11.xyz, r3.wwww, r11.xyzx, r9.xyzx
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 124: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 126: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 127: mul r12.xyz, r11.xyzx, cb0[10].zzzz
    r12.xyz = ((r11.xyzx)*(source[10].zzzz)).xyz;
    // 128: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 130: mad r11.xyz, cb0[10].zzzz, r11.xyzx, -r9.xyzx
    r11.xyz = ((source[10].zzzz)*(r11.xyzx)+(-(r9.xyzx))).xyz;
    // 131: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 132: dp3 r2.w, r10.xyzx, r4.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: max r3.w, r2.w, l(0.000000)
    r3.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r4.w, r3.w, l(1.000000)
    r4.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r11.xyz, r2.xyzx, r4.wwww
    r11.xyz = ((r2.xyzx)*(r4.wwww)).xyz;
    // 136: mad r12.xyz, -r4.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r4.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mad r11.yzw, cb0[16].zzzz, r12.xxyz, r11.xxyz
    r11.yzw = ((source[16].zzzz)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 138: dp3 r4.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r12.xyz, r4.wwww, -cb0[2].xyzx
    r12.xyz = ((r4.wwww)+(-(source[2].xyzx))).xyz;
    // 140: mad r12.xyz, cb0[13].yyyy, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[13].yyyy)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 141: dp3 r4.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r13.xyz, -r12.xyzx, r4.wwww
    r13.xyz = ((-(r12.xyzx))+(r4.wwww)).xyz;
    // 143: mad r12.xyz, cb0[13].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[13].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 144: mul r13.xyz, r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r13.xyz = ((r12.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 145: mad r14.xyz, -r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 146: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 148: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 149: mul r13.xyz, r13.xyzx, r2.wwww
    r13.xyz = ((r13.xyzx)*(r2.wwww)).xyz;
    // 150: mad r11.yzw, r11.yyzw, r14.xxyz, r13.xxyz
    r11.yzw = ((r11.yyzw)*(r14.xxyz)+(r13.xxyz)).yzw;
    // 151: mov_sat r2.w, r4.z
    r2.w = (saturate(r4.zzzz)).w;
    // 152: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 153: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 154: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add r13.xyz, -r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r12.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mad r12.xyz, r2.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r2.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 157: mad r12.xyz, r11.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r12.xyzx
    r12.xyz = ((r11.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r12.xyzx)).xyz;
    // 158: mul_sat r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r12.xyzx))).xyz;
    // 159: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 160: mad r5.xyz, r5.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 161: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 162: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 163: div r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 164: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 165: mov_sat r2.w, r3.z
    r2.w = (saturate(r3.zzzz)).w;
    // 166: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 167: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 168: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 169: mul r5.xyz, r5.xyzx, cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(source[13].wwww)).xyz;
    // 170: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 171: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 172: dp3 r4.w, r0.xyzx, r10.xyzx
    r4.w = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 173: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 174: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 175: mul r4.xyz, r3.zzzz, r1.xyzx
    r4.xyz = ((r3.zzzz)*(r1.xyzx)).xyz;
    // 176: mad r1.xyz, r4.xyzx, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 177: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 178: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 179: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 180: add r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)+(source[6].zzzz)).y;
    // 181: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 182: add r0.x, r0.x, r4.w
    r0.x = ((r0.xxxx)+(r4.wwww)).x;
    // 183: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 184: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 185: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 186: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mad r0.y, cb0[14].z, l(4.500000), l(0.500000)
    r0.y = ((source[14].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 188: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 189: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 190: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 191: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 192: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 193: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 194: mul r0.x, r2.w, r0.x
    r0.x = ((r2.wwww)*(r0.xxxx)).x;
    // 195: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 196: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 197: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 198: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 199: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 201: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 202: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 204: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 205: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 206: mad r0.xyz, r9.xyzx, r11.yzwy, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r11.yzwy)+(r0.xyzx)).xyz;
    // 207: mad r0.xyz, r1.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 208: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 210: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 211: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 212: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 213: ret
    return output;
}

// source.character.mokoko-av036-907.v1 / source program 6e68c38a2961bd4289357c8c85b062fa
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight907(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[14]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[17]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[18].w=(g_SourceCharacterTime.xxxx).x;
    source[21].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[21].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
    // 1: add r0.xyzw, v6.yzxy, cb0[0].yzxy
    r0.xyzw = ((v6.yzxy)+(source[0].yzxy)).xyzw;
    // 2: mul r1.xyz, v6.yyyy, cb1[1].xywx
    r1.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 3: mad r1.xyz, cb1[0].xywx, v6.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v6.xxxx)+(r1.xyzx)).xyz;
    // 4: mad r1.xyz, cb1[2].xywx, v6.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v6.zzzz)+(r1.xyzx)).xyz;
    // 5: mad r1.xyz, cb1[3].xywx, v6.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v6.wwww)+(r1.xyzx)).xyz;
    // 6: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
    // 7: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 8: div r2.xy, r1.xyxx, r1.zzzz
    r2.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 9: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t6.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 16: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 17: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 18: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r4.xyz, r1.wwww, v3.xyzx
    r4.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v2.xyxx, t1.wxyz, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r5.x, r5.x
    r5.x = (saturate(r5.xxxx)).x;
    // 23: add r2.w, r5.x, l(-0.333300)
    r2.w = ((r5.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v2.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: add r2.w, -r6.w, l(1.000000)
    r2.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 29: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 30: mul r2.w, r2.w, cb0[21].x
    r2.w = ((r2.wwww)*(source[21].xxxx)).w;
    // 31: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 32: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r7.xyz, v3.xyzx, r1.wwww, r3.xyzx
    r7.xyz = ((v3.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 34: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 35: sqrt r4.w, r1.w
    r4.w = (sqrt(r1.wwww)).w;
    // 36: div r7.xyz, r7.xyzx, r4.wwww
    r7.xyz = ((r7.xyzx)/(r4.wwww)).xyz;
    // 37: dp3 r4.w, r7.xyzx, r3.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 38: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: lt r5.x, |r4.w|, l(0.000001)
    r5.x = (asfloat((uint4)((abs(r4.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r6.w, |r4.w|, |r4.w|
    r6.w = ((abs(r4.wwww))*(abs(r4.wwww))).w;
    // 41: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 42: mul r4.w, |r4.w|, r6.w
    r4.w = ((abs(r4.wwww))*(r6.wwww)).w;
    // 43: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 44: add r5.x, r4.w, l(-0.027778)
    r5.x = ((r4.wwww)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).x;
    // 45: mad r4.w, r4.w, r5.x, l(0.027778)
    r4.w = ((r4.wwww)*(r5.xxxx)+(float4(0.027778,0.027778,0.027778,0.027778))).w;
    // 46: div_sat r1.w, r4.w, r1.w
    r1.w = (saturate((r4.wwww)/(r1.wwww))).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 49: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: mul r2.w, r1.w, l(0.043153)
    r2.w = ((r1.wwww)*(float4(0.043153,0.043153,0.043153,0.043153))).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 52: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 53: mul r8.xy, r7.xyxx, cb0[18].xxxx
    r8.xy = ((r7.xyxx)*(source[18].xxxx)).xy;
    // 54: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 55: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 57: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 58: add r8.z, r3.w, l(0.000010)
    r8.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 59: dp3 r3.w, r8.xyzx, r8.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: div r7.xyz, r8.xyzx, r3.wwww
    r7.xyz = ((r8.xyzx)/(r3.wwww)).xyz;
    // 62: dp3 r3.w, r7.xyzx, r4.xyzx
    r3.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 64: min r4.x, r3.w, l(1.000000)
    r4.x = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 66: mul r2.xyz, r2.xyzx, r4.xxxx
    r2.xyz = ((r2.xyzx)*(r4.xxxx)).xyz;
    // 67: mad r4.x, -r1.w, l(0.043153), l(1.000000)
    r4.x = ((-(r1.wwww))*(float4(0.043153,0.043153,0.043153,0.043153))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 68: mad r2.xyz, r2.xyzx, r4.xxxx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r4.xxxx)+(r2.wwww)).xyz;
    // 69: dp3 r2.w, cb0[4].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((source[4].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r4.xyz, r2.wwww, -cb0[4].xyzx
    r4.xyz = ((r2.wwww)+(-(source[4].xyzx))).xyz;
    // 71: mad r4.xyz, cb0[18].yyyy, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((source[18].yyyy)*(r4.xyzx)+(source[4].xyzx)).xyz;
    // 72: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r8.xyz, -r4.xyzx, r2.wwww
    r8.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 74: mad r4.xyz, cb0[18].zzzz, r8.xyzx, r4.xyzx
    r4.xyz = ((source[18].zzzz)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 75: mad r8.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mad r9.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 78: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 79: dp3 r2.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: add r9.xyz, -r5.yzwy, r2.wwww
    r9.xyz = ((-(r5.yzwy))+(r2.wwww)).xyz;
    // 81: mad r5.xyz, cb0[18].yyyy, r9.xyzx, r5.yzwy
    r5.xyz = ((source[18].yyyy)*(r9.xyzx)+(r5.yzwy)).xyz;
    // 82: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r9.xyz, -r5.xyzx, r2.wwww
    r9.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 84: mad r5.xyz, cb0[18].zzzz, r9.xyzx, r5.xyzx
    r5.xyz = ((source[18].zzzz)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 85: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 86: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 87: add r9.xy, v2.xyxx, cb0[8].xyxx
    r9.xy = ((v2.xyxx)+(source[8].xyxx)).xy;
    // 88: add r9.xy, -r1.xyxx, r9.xyxx
    r9.xy = ((-(r1.xyxx))+(r9.xyxx)).xy;
    // 89: mad r9.xy, cb0[8].zzzz, r9.xyxx, r1.xyxx
    r9.xy = ((source[8].zzzz)*(r9.xyxx)+(r1.xyxx)).xy;
    // 90: mul r9.xy, r9.xyxx, cb0[7].xyxx
    r9.xy = ((r9.xyxx)*(source[7].xyxx)).xy;
    // 91: mad r9.xy, cb0[18].wwww, cb0[7].zwzz, r9.xyxx
    r9.xy = ((source[18].wwww)*(source[7].zwzz)+(r9.xyxx)).xy;
    // 92: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r9.xyxx, t2.yzxw, s3, l(0.000000)
    r1.z = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 93: mul r9.xyz, r1.zzzz, cb0[9].xyzx
    r9.xyz = ((r1.zzzz)*(source[9].xyzx)).xyz;
    // 94: mul r10.xyz, r9.xyzx, cb0[9].wwww
    r10.xyz = ((r9.xyzx)*(source[9].wwww)).xyz;
    // 95: add r11.xy, v2.xyxx, cb0[11].xyxx
    r11.xy = ((v2.xyxx)+(source[11].xyxx)).xy;
    // 96: add r11.xy, -r1.xyxx, r11.xyxx
    r11.xy = ((-(r1.xyxx))+(r11.xyxx)).xy;
    // 97: mad r1.xy, cb0[11].zzzz, r11.xyxx, r1.xyxx
    r1.xy = ((source[11].zzzz)*(r11.xyxx)+(r1.xyxx)).xy;
    // 98: mul r1.xy, r1.xyxx, cb0[10].xyxx
    r1.xy = ((r1.xyxx)*(source[10].xyxx)).xy;
    // 99: mad r1.xy, cb0[18].wwww, cb0[10].zwzz, r1.xyxx
    r1.xy = ((source[18].wwww)*(source[10].zwzz)+(r1.xyxx)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.yxzw, s3, l(0.000000)
    r1.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).x;
    // 101: mul r1.xyz, r1.xxxx, cb0[12].xyzx
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)).xyz;
    // 102: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 103: mad r9.xyz, cb0[9].wwww, r9.xyzx, r1.xyzx
    r9.xyz = ((source[9].wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 104: mad r1.xyz, r10.xyzx, r1.xyzx, -r9.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)+(-(r9.xyzx))).xyz;
    // 105: mad r1.xyz, cb0[19].xxxx, r1.xyzx, r9.xyzx
    r1.xyz = ((source[19].xxxx)*(r1.xyzx)+(r9.xyzx)).xyz;
    // 106: add r9.xy, r0.ywyy, -cb0[2].zyzz
    r9.xy = ((r0.ywyy)+(-(source[2].zyzz))).xy;
    // 107: add r2.w, -r9.x, l(1.000000)
    r2.w = ((-(r9.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: add r2.w, -r9.y, r2.w
    r2.w = ((-(r9.yyyy))+(r2.wwww)).w;
    // 109: mad r2.w, cb0[13].z, r2.w, r9.y
    r2.w = ((source[13].zzzz)*(r2.wwww)+(r9.yyyy)).w;
    // 110: mul r2.w, r2.w, cb0[13].x
    r2.w = ((r2.wwww)*(source[13].xxxx)).w;
    // 111: mul r2.w, r2.w, l(0.000314)
    r2.w = ((r2.wwww)*(float4(0.000314,0.000314,0.000314,0.000314))).w;
    // 112: mad r2.w, cb0[13].y, cb0[18].w, r2.w
    r2.w = ((source[13].yyyy)*(source[18].wwww)+(r2.wwww)).w;
    // 113: add r9.xyz, r2.wwww, l(0.000000, 0.330000, 0.660000, 0.000000)
    r9.xyz = ((r2.wwww)+(float4(0.000000,0.330000,0.660000,0.000000))).xyz;
    // 114: mul r9.xyz, r9.xyzx, l(6.283185, 6.283185, 6.283185, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(6.283185,6.283185,6.283185,0.000000))).xyz;
    // 115: sincos null, r9.xyz, r9.xyzx
    r9.xyz = (cos(r9.xyzx)).xyz;
    // 116: mad r9.xyz, r9.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(0.500000, 0.500000, 0.500000, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 117: mad r1.xyz, cb0[13].wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((source[13].wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 118: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r9.xyz, -r1.xyzx, r2.wwww
    r9.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 120: mad r1.xyz, cb0[18].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[18].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 121: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r9.xyz, -r1.xyzx, r2.wwww
    r9.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 123: mad r1.xyz, cb0[18].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 124: mul r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(r1.xyzx)).xyz;
    // 125: sample_b_indexable(texture2d)(float,float,float,float) r2.w, v2.xyxx, t2.xyzw, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 126: mad r1.xyz, r5.xyzx, r1.xyzx, -r5.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)+(-(r5.xyzx))).xyz;
    // 127: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 128: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 129: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: add r4.xyz, -r6.xyzx, r2.wwww
    r4.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 131: mad r4.xyz, cb0[18].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[18].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 132: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 134: mad r4.xyz, cb0[18].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[18].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 135: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 136: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 137: mul r5.xyz, r5.xyzx, cb0[19].yyyy
    r5.xyz = ((r5.xyzx)*(source[19].yyyy)).xyz;
    // 138: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v2.xyxx, t4.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 139: mov_sat r6.xyz, r6.xyzx
    r6.xyz = (saturate(r6.xyzx)).xyz;
    // 140: add r2.w, r6.y, r6.x
    r2.w = ((r6.yyyy)+(r6.xxxx)).w;
    // 141: add r2.w, r6.z, r2.w
    r2.w = ((r6.zzzz)+(r2.wwww)).w;
    // 142: add_sat r2.w, r6.w, r2.w
    r2.w = (saturate((r6.wwww)+(r2.wwww))).w;
    // 143: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 144: dp3 r2.w, r7.xyzx, r3.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 145: mul_sat r2.w, r2.w, cb0[19].z
    r2.w = (saturate((r2.wwww)*(source[19].zzzz))).w;
    // 146: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul_sat r3.x, r3.z, cb0[19].z
    r3.x = (saturate((r3.zzzz)*(source[19].zzzz))).x;
    // 148: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 149: add_sat r3.x, r3.x, -cb0[19].w
    r3.x = (saturate((r3.xxxx)+(-(source[19].wwww)))).x;
    // 150: lt r3.y, r3.x, l(0.000001)
    r3.y = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 151: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 152: mul r3.x, r3.x, cb0[20].x
    r3.x = ((r3.xxxx)*(source[20].xxxx)).x;
    // 153: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 154: mul r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)*(r3.xxxx)).w;
    // 155: movc r2.w, r3.y, l(0), r2.w
    r2.w = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 156: max r3.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 157: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 158: mul r3.xyz, r3.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 159: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 160: dp3 r3.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 161: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 162: mul r3.x, r3.x, cb0[20].y
    r3.x = ((r3.xxxx)*(source[20].yyyy)).x;
    // 163: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 164: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 165: mad r3.y, -r3.x, r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 166: max r3.y, r3.y, l(0.001000)
    r3.y = (max(r3.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 167: div r3.y, cb0[20].z, r3.y
    r3.y = ((source[20].zzzz)/(r3.yyyy)).y;
    // 168: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 169: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 170: mul r3.y, r3.y, cb0[20].w
    r3.y = ((r3.yyyy)*(source[20].wwww)).y;
    // 171: mad r4.xyz, r2.wwww, r4.xyzx, -r1.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 172: mad r4.xyz, r3.yyyy, r4.xyzx, r1.xyzx
    r4.xyz = ((r3.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 173: mad r4.xyz, r1.wwww, r4.xyzx, -r1.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 174: mad r1.xyz, r3.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 175: mul r1.w, cb0[3].z, l(1.500000)
    r1.w = ((source[3].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 176: add r2.w, -cb0[3].w, l(1.000000)
    r2.w = ((-(source[3].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r2.w, r2.w, cb0[18].w
    r2.w = ((r2.wwww)*(source[18].wwww)).w;
    // 178: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 179: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 180: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 182: mad r1.w, r1.w, l(0.500000), cb0[3].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[3].zzzz)).w;
    // 183: frc r2.w, cb0[3].x
    r2.w = (frac(source[3].xxxx)).w;
    // 184: add r3.x, -r2.w, cb0[3].x
    r3.x = ((-(r2.wwww))+(source[3].xxxx)).x;
    // 185: mul r4.z, r3.x, l(0.125000)
    r4.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 186: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 187: mul r4.y, cb0[3].y, cb0[14].y
    r4.y = ((source[3].yyyy)*(source[14].yyyy)).y;
    // 188: frc r3.x, v2.x
    r3.x = (frac(v2.xxxx)).x;
    // 189: mul r3.x, r3.x, l(0.125000)
    r3.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 190: mov r3.y, v2.y
    r3.y = (v2.yyyy).y;
    // 191: add r3.xy, r3.xyxx, r4.xyxx
    r3.xy = ((r3.xyxx)+(r4.xyxx)).xy;
    // 192: add r3.xy, r3.xyxx, r4.zwzz
    r3.xy = ((r3.xyxx)+(r4.zwzz)).xy;
    // 193: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 194: mul r3.xyz, r1.wwww, r4.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 195: mul r1.w, r2.w, r4.w
    r1.w = ((r2.wwww)*(r4.wwww)).w;
    // 196: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 197: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 198: mul r1.w, cb0[15].y, cb0[18].w
    r1.w = ((source[15].yyyy)*(source[18].wwww)).w;
    // 199: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 200: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 201: mul r3.y, r1.w, l(0.020000)
    r3.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 202: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 203: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 204: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 205: mad r0.xy, cb0[15].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[15].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 206: mul r0.z, cb0[15].x, l(0.001000)
    r0.z = ((source[15].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 207: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 208: mad r0.xy, r0.zzzz, r0.xyxx, r3.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r3.xyxx)).xy;
    // 209: dp2 r0.z, cb0[16].xyxx, r0.xyxx
    r0.z = (dot((source[16].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 210: dp2 r0.y, cb0[17].xyxx, r0.xyxx
    r0.y = (dot((source[17].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 211: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 212: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 213: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t5.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 214: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 215: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 216: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 217: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 219: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 220: mul r3.xyz, r0.xyzx, cb0[15].zzzz
    r3.xyz = ((r0.xyzx)*(source[15].zzzz)).xyz;
    // 221: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 222: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 223: mad r0.xyz, cb0[15].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[15].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 224: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 225: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 226: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 227: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 228: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 229: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 230: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 231: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 232: ret
    return output;
}

// source.character.mokoko-av036-908.v1 / source program 6cf25ec40f73404185757ea06ccb3996
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight908(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].w=(g_SourceCharacterTime.xxxx).x;
    source[28]=float4(input.lightColor,1.0);
    source[29].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[29].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[29].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.wxyz, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 32: add r1.w, r8.x, l(-0.333300)
    r1.w = ((r8.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 33: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 34: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 37: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 38: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[19].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 49: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 53: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 54: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: add r0.z, -cb0[20].y, cb0[20].x
    r0.z = ((-(source[20].yyyy))+(source[20].xxxx)).z;
    // 68: mad r0.z, r11.x, r0.z, cb0[20].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[20].yyyy)).z;
    // 69: add r1.x, -r0.z, cb0[20].z
    r1.x = ((-(r0.zzzz))+(source[20].zzzz)).x;
    // 70: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 71: add r1.x, -r0.z, cb0[20].w
    r1.x = ((-(r0.zzzz))+(source[20].wwww)).x;
    // 72: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 74: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 75: add r1.y, -cb0[18].y, cb0[18].x
    r1.y = ((-(source[18].yyyy))+(source[18].xxxx)).y;
    // 76: mad r1.y, r11.x, r1.y, cb0[18].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[18].yyyy)).y;
    // 77: add r1.w, -r1.y, cb0[18].z
    r1.w = ((-(r1.yyyy))+(source[18].zzzz)).w;
    // 78: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 79: add r1.w, -r1.y, cb0[18].w
    r1.w = ((-(r1.yyyy))+(source[18].wwww)).w;
    // 80: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 81: add r1.w, -r1.y, cb0[19].x
    r1.w = ((-(r1.yyyy))+(source[19].xxxx)).w;
    // 82: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 83: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 84: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 85: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 86: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 87: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 88: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 89: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 90: add r1.w, -r1.y, cb0[21].x
    r1.w = ((-(r1.yyyy))+(source[21].xxxx)).w;
    // 91: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 92: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 93: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 94: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 95: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 96: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 97: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 98: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 99: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 100: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 101: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 102: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 103: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 104: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 105: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 106: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 107: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 108: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 109: rcp r1.y, cb0[21].y
    r1.y = (1.0/(source[21].yyyy)).y;
    // 110: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 111: mul r12.xyz, r4.xyzx, cb0[21].yyyy
    r12.xyz = ((r4.xyzx)*(source[21].yyyy)).xyz;
    // 112: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 113: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 114: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 115: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 116: mad r4.xyz, r12.xyzx, cb0[21].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[21].yyyy)+(r4.xyzx)).xyz;
    // 117: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 119: add r1.y, cb0[21].y, l(1.000000)
    r1.y = ((source[21].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 120: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 121: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 122: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 124: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 125: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 126: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 127: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 128: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 129: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 130: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 131: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 132: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 133: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 134: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 135: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 136: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 137: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 139: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 140: mul r13.xyz, r12.xyzx, cb0[24].yyyy
    r13.xyz = ((r12.xyzx)*(source[24].yyyy)).xyz;
    // 141: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 142: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 144: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 145: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 146: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 147: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 148: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 149: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 150: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 151: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 152: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 156: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 157: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 158: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 160: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 161: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 162: add_sat r2.w, r11.w, cb0[24].z
    r2.w = (saturate((r11.wwww)+(source[24].zzzz))).w;
    // 163: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 165: mul_sat r6.xy, r6.xzxx, cb0[22].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[22].xxxx))).xy;
    // 166: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 167: add_sat r6.y, r6.y, -cb0[22].y
    r6.y = (saturate((r6.yyyy)+(-(source[22].yyyy)))).y;
    // 168: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 169: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 170: mul r6.y, r6.y, cb0[22].z
    r6.y = ((r6.yyyy)*(source[22].zzzz)).y;
    // 171: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 172: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 173: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 174: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 175: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 176: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 177: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 178: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 179: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 180: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 181: mul r8.x, r7.w, cb0[24].w
    r8.x = ((r7.wwww)*(source[24].wwww)).x;
    // 182: mad r1.x, -r7.w, cb0[24].w, r1.x
    r1.x = ((-(r7.wwww))*(source[24].wwww)+(r1.xxxx)).x;
    // 183: mad r1.x, r11.w, r1.x, r8.x
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.xxxx)).x;
    // 184: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 185: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 186: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 187: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 188: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 189: mad r6.yzw, -cb0[24].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[24].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 190: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 191: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 192: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 193: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 194: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 195: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 196: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 197: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 199: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 200: mad r7.xyz, cb0[19].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 201: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 203: mad r7.xyz, cb0[19].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[19].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 204: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 205: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 206: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 207: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 208: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 209: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 210: dp3 r1.x, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 211: add r12.xyz, -r8.yzwy, r1.xxxx
    r12.xyz = ((-(r8.yzwy))+(r1.xxxx)).xyz;
    // 212: mad r8.xyz, cb0[19].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[19].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 213: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 214: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 215: mad r8.xyz, cb0[19].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[19].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 216: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 217: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 219: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 220: add r13.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r13.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 221: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 222: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 223: mul r0.xyz, r0.xyzx, cb0[21].zzzz
    r0.xyz = ((r0.xyzx)*(source[21].zzzz)).xyz;
    // 224: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 225: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 227: mad r2.yzw, cb0[19].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[19].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 228: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 230: mad r2.yzw, cb0[19].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[19].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 231: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 232: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 233: mul r14.xyz, r14.xyzx, cb0[21].wwww
    r14.xyz = ((r14.xyzx)*(source[21].wwww)).xyz;
    // 234: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 235: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 236: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 237: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 238: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 239: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 240: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 241: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 242: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 243: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 244: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 245: add r1.z, cb0[22].w, -cb0[23].x
    r1.z = ((source[22].wwww)+(-(source[23].xxxx))).z;
    // 246: mad r1.z, r11.w, r1.z, cb0[23].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[23].xxxx)).z;
    // 247: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 248: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 249: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 250: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 251: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 252: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 253: div r1.z, cb0[23].y, r1.z
    r1.z = ((source[23].yyyy)/(r1.zzzz)).z;
    // 254: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 255: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 256: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r1.z, r1.z, cb0[23].z
    r1.z = ((r1.zzzz)*(source[23].zzzz)).z;
    // 258: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 260: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 261: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 262: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 263: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 264: mad r0.xyz, cb0[19].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 265: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 266: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 267: mad r0.xyz, cb0[19].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 268: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 269: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 270: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mul r2.w, r2.w, cb0[23].w
    r2.w = ((r2.wwww)*(source[23].wwww)).w;
    // 272: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 273: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 274: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 275: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 276: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 277: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 278: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 279: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 280: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 281: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 282: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 283: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 284: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 285: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 286: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 289: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 290: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 291: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 292: mul r1.z, cb0[13].y, cb0[23].w
    r1.z = ((source[13].yyyy)*(source[23].wwww)).z;
    // 293: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 294: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 295: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 296: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 297: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 298: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 299: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 300: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 301: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 302: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 303: dp2 r2.w, cb0[14].xyxx, r3.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 304: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 305: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 306: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 307: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 308: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 309: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 310: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 311: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 312: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 313: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 314: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 315: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 316: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 317: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 318: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 319: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 320: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 321: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 322: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 323: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 324: mul r4.z, r0.w, cb0[25].x
    r4.z = ((r0.wwww)*(source[25].xxxx)).z;
    // 325: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 326: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 327: min r0.w, r0.w, cb0[25].x
    r0.w = (min(r0.wwww,source[25].xxxx)).w;
    // 328: add r1.x, -cb0[25].w, cb0[25].z
    r1.x = ((-(source[25].wwww))+(source[25].zzzz)).x;
    // 329: mad r1.x, cb0[25].y, r1.x, cb0[25].w
    r1.x = ((source[25].yyyy)*(r1.xxxx)+(source[25].wwww)).x;
    // 330: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 331: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 332: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 333: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 334: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 335: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 336: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 337: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 338: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 339: add r0.w, -cb0[26].z, cb0[26].y
    r0.w = ((-(source[26].zzzz))+(source[26].yyyy)).w;
    // 340: mad r0.w, cb0[26].x, r0.w, cb0[26].z
    r0.w = ((source[26].xxxx)*(r0.wwww)+(source[26].zzzz)).w;
    // 341: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 342: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 343: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 344: add r0.w, -cb0[26].w, l(2.000000)
    r0.w = ((-(source[26].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 345: mad r0.w, r1.y, r0.w, cb0[26].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[26].wwww)).w;
    // 346: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 347: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 348: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 349: mul r1.xyz, r1.xyzx, cb0[27].xxxx
    r1.xyz = ((r1.xyzx)*(source[27].xxxx)).xyz;
    // 350: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 351: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 352: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 353: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 354: mul r1.xyz, r1.xyzx, cb0[27].yyyy
    r1.xyz = ((r1.xyzx)*(source[27].yyyy)).xyz;
    // 355: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 356: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 357: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 358: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 359: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 360: mul o0.xyz, r0.xyzx, cb0[28].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[28].xyzx)).xyz;
    // 361: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 362: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 363: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 364: ret
    return output;
}

// source.character.mokoko-av036-909.v1 / source program b710ee0925a6c44fa230b8ff8567f5fc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight909(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[17]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[19]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[20]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].w=(g_SourceCharacterTime.xxxx).x;
    source[24].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[24].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[25]=float4(input.lightColor,1.0);
    source[26].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
    // 1: add r0.xyzw, v8.yzxy, cb0[0].yzxy
    r0.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 2: mul r1.xyz, v8.yyyy, cb1[1].xywx
    r1.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 3: mad r1.xyz, cb1[0].xywx, v8.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v8.xxxx)+(r1.xyzx)).xyz;
    // 4: mad r1.xyz, cb1[2].xywx, v8.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v8.zzzz)+(r1.xyzx)).xyz;
    // 5: mad r1.xyz, cb1[3].xywx, v8.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v8.wwww)+(r1.xyzx)).xyz;
    // 6: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[26].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[26].xxxx)) * 0xffffffffu)).w;
    // 7: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 8: div r2.xy, r1.xyxx, r1.zzzz
    r2.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 9: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t6.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 16: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 17: mul r3.xyz, r1.wwww, v7.xyzx
    r3.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 18: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r5.x, r5.x
    r5.x = (saturate(r5.xxxx)).x;
    // 23: add r2.w, r5.x, l(-0.333300)
    r2.w = ((r5.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: add r2.w, -r6.w, l(1.000000)
    r2.w = ((-(r6.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 28: lt r3.w, |r2.w|, l(0.000001)
    r3.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 29: log r2.w, |r2.w|
    r2.w = (log2(abs(r2.wwww))).w;
    // 30: mul r2.w, r2.w, cb0[24].y
    r2.w = ((r2.wwww)*(source[24].yyyy)).w;
    // 31: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 32: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r7.xyz, v5.xyzx, r1.wwww, r3.xyzx
    r7.xyz = ((v5.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 34: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 35: sqrt r4.w, r1.w
    r4.w = (sqrt(r1.wwww)).w;
    // 36: div r7.xyz, r7.xyzx, r4.wwww
    r7.xyz = ((r7.xyzx)/(r4.wwww)).xyz;
    // 37: dp3 r4.w, r7.xyzx, r3.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 38: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: lt r5.x, |r4.w|, l(0.000001)
    r5.x = (asfloat((uint4)((abs(r4.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 40: mul r6.w, |r4.w|, |r4.w|
    r6.w = ((abs(r4.wwww))*(abs(r4.wwww))).w;
    // 41: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 42: mul r4.w, |r4.w|, r6.w
    r4.w = ((abs(r4.wwww))*(r6.wwww)).w;
    // 43: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 44: add r5.x, r4.w, l(-0.027778)
    r5.x = ((r4.wwww)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).x;
    // 45: mad r4.w, r4.w, r5.x, l(0.027778)
    r4.w = ((r4.wwww)*(r5.xxxx)+(float4(0.027778,0.027778,0.027778,0.027778))).w;
    // 46: div_sat r1.w, r4.w, r1.w
    r1.w = (saturate((r4.wwww)/(r1.wwww))).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 49: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: mul r2.w, r1.w, l(0.043153)
    r2.w = ((r1.wwww)*(float4(0.043153,0.043153,0.043153,0.043153))).w;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 52: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 53: mul r8.xy, r7.xyxx, cb0[21].xxxx
    r8.xy = ((r7.xyxx)*(source[21].xxxx)).xy;
    // 54: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 55: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 57: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 58: add r8.z, r3.w, l(0.000010)
    r8.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 59: dp3 r3.w, r8.xyzx, r8.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 60: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 61: div r7.xyz, r8.xyzx, r3.wwww
    r7.xyz = ((r8.xyzx)/(r3.wwww)).xyz;
    // 62: dp3 r3.w, r7.xyzx, r4.xyzx
    r3.w = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 64: min r4.x, r3.w, l(1.000000)
    r4.x = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: sqrt r4.x, r4.x
    r4.x = (sqrt(r4.xxxx)).x;
    // 66: mul r2.xyz, r2.xyzx, r4.xxxx
    r2.xyz = ((r2.xyzx)*(r4.xxxx)).xyz;
    // 67: mad r4.x, -r1.w, l(0.043153), l(1.000000)
    r4.x = ((-(r1.wwww))*(float4(0.043153,0.043153,0.043153,0.043153))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 68: mad r2.xyz, r2.xyzx, r4.xxxx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r4.xxxx)+(r2.wwww)).xyz;
    // 69: dp3 r2.w, cb0[6].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((source[6].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: add r4.xyz, r2.wwww, -cb0[6].xyzx
    r4.xyz = ((r2.wwww)+(-(source[6].xyzx))).xyz;
    // 71: mad r4.xyz, cb0[21].yyyy, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((source[21].yyyy)*(r4.xyzx)+(source[6].xyzx)).xyz;
    // 72: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r9.xyz, -r4.xyzx, r2.wwww
    r9.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 74: mad r4.xyz, cb0[21].zzzz, r9.xyzx, r4.xyzx
    r4.xyz = ((source[21].zzzz)*(r9.xyzx)+(r4.xyzx)).xyz;
    // 75: mad r9.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mad r10.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 78: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 79: dp3 r2.w, r5.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: add r10.xyz, -r5.yzwy, r2.wwww
    r10.xyz = ((-(r5.yzwy))+(r2.wwww)).xyz;
    // 81: mad r5.xyz, cb0[21].yyyy, r10.xyzx, r5.yzwy
    r5.xyz = ((source[21].yyyy)*(r10.xyzx)+(r5.yzwy)).xyz;
    // 82: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: add r10.xyz, -r5.xyzx, r2.wwww
    r10.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 84: mad r5.xyz, cb0[21].zzzz, r10.xyzx, r5.xyzx
    r5.xyz = ((source[21].zzzz)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 85: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 86: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 87: add r10.xy, v4.xyxx, cb0[10].xyxx
    r10.xy = ((v4.xyxx)+(source[10].xyxx)).xy;
    // 88: add r10.xy, -r1.xyxx, r10.xyxx
    r10.xy = ((-(r1.xyxx))+(r10.xyxx)).xy;
    // 89: mad r10.xy, cb0[10].zzzz, r10.xyxx, r1.xyxx
    r10.xy = ((source[10].zzzz)*(r10.xyxx)+(r1.xyxx)).xy;
    // 90: mul r10.xy, r10.xyxx, cb0[9].xyxx
    r10.xy = ((r10.xyxx)*(source[9].xyxx)).xy;
    // 91: mad r10.xy, cb0[21].wwww, cb0[9].zwzz, r10.xyxx
    r10.xy = ((source[21].wwww)*(source[9].zwzz)+(r10.xyxx)).xy;
    // 92: sample_b_indexable(texture2d)(float,float,float,float) r1.z, r10.xyxx, t2.yzxw, s3, l(0.000000)
    r1.z = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).z;
    // 93: mul r10.xyz, r1.zzzz, cb0[11].xyzx
    r10.xyz = ((r1.zzzz)*(source[11].xyzx)).xyz;
    // 94: mul r11.xyz, r10.xyzx, cb0[11].wwww
    r11.xyz = ((r10.xyzx)*(source[11].wwww)).xyz;
    // 95: add r12.xy, v4.xyxx, cb0[13].xyxx
    r12.xy = ((v4.xyxx)+(source[13].xyxx)).xy;
    // 96: add r12.xy, -r1.xyxx, r12.xyxx
    r12.xy = ((-(r1.xyxx))+(r12.xyxx)).xy;
    // 97: mad r1.xy, cb0[13].zzzz, r12.xyxx, r1.xyxx
    r1.xy = ((source[13].zzzz)*(r12.xyxx)+(r1.xyxx)).xy;
    // 98: mul r1.xy, r1.xyxx, cb0[12].xyxx
    r1.xy = ((r1.xyxx)*(source[12].xyxx)).xy;
    // 99: mad r1.xy, cb0[21].wwww, cb0[12].zwzz, r1.xyxx
    r1.xy = ((source[21].wwww)*(source[12].zwzz)+(r1.xyxx)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.yxzw, s3, l(0.000000)
    r1.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).x;
    // 101: mul r1.xyz, r1.xxxx, cb0[14].xyzx
    r1.xyz = ((r1.xxxx)*(source[14].xyzx)).xyz;
    // 102: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 103: mad r10.xyz, cb0[11].wwww, r10.xyzx, r1.xyzx
    r10.xyz = ((source[11].wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 104: mad r1.xyz, r11.xyzx, r1.xyzx, -r10.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)+(-(r10.xyzx))).xyz;
    // 105: mad r1.xyz, cb0[22].xxxx, r1.xyzx, r10.xyzx
    r1.xyz = ((source[22].xxxx)*(r1.xyzx)+(r10.xyzx)).xyz;
    // 106: mad r10.xy, cb0[21].wwww, cb0[16].zwzz, v4.xyxx
    r10.xy = ((source[21].wwww)*(source[16].zwzz)+(v4.xyxx)).xy;
    // 107: add r11.xyz, -r8.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 108: mad r8.xyz, r11.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r8.xyzx
    r8.xyz = ((r11.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r8.xyzx)).xyz;
    // 109: dp3 r2.w, r8.xyzx, r8.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 110: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 111: div r8.xyz, r8.xyzx, r2.wwww
    r8.xyz = ((r8.xyzx)/(r2.wwww)).xyz;
    // 112: add r11.xyz, -r8.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 113: mad r11.xyz, r11.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), r8.xyzx
    r11.xyz = ((r11.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(r8.xyzx)).xyz;
    // 114: mul r12.xyz, r11.yyyy, cb0[3].xyzx
    r12.xyz = ((r11.yyyy)*(source[3].xyzx)).xyz;
    // 115: mad r11.xyw, cb0[2].xyxz, r11.xxxx, r12.xyxz
    r11.xyw = ((source[2].xyxz)*(r11.xxxx)+(r12.xyxz)).xyw;
    // 116: mad r11.xyz, cb0[4].xyzx, r11.zzzz, r11.xywx
    r11.xyz = ((source[4].xyzx)*(r11.zzzz)+(r11.xywx)).xyz;
    // 117: dp3 r2.w, r3.xyzx, r11.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 118: mul r8.xy, r8.xyxx, r2.wwww
    r8.xy = ((r8.xyxx)*(r2.wwww)).xy;
    // 119: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), -r3.xyxx
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r3.xyxx))).xy;
    // 120: mad r8.xy, r10.xyxx, cb0[16].xyxx, r8.xyxx
    r8.xy = ((r10.xyxx)*(source[16].xyxx)+(r8.xyxx)).xy;
    // 121: add r8.xy, r8.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 122: sample_b_indexable(texture2d)(float,float,float,float) r2.w, r8.xyxx, t2.xywz, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).w;
    // 123: mul_sat r2.w, r2.w, cb0[15].w
    r2.w = (saturate((r2.wwww)*(source[15].wwww))).w;
    // 124: add r8.xyz, -r1.xyzx, cb0[15].xyzx
    r8.xyz = ((-(r1.xyzx))+(source[15].xyzx)).xyz;
    // 125: mad r1.xyz, r2.wwww, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.wwww)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 126: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 127: add r8.xyz, -r1.xyzx, r2.wwww
    r8.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 128: mad r1.xyz, cb0[21].yyyy, r8.xyzx, r1.xyzx
    r1.xyz = ((source[21].yyyy)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 129: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: add r8.xyz, -r1.xyzx, r2.wwww
    r8.xyz = ((-(r1.xyzx))+(r2.wwww)).xyz;
    // 131: mad r1.xyz, cb0[21].zzzz, r8.xyzx, r1.xyzx
    r1.xyz = ((source[21].zzzz)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 132: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 133: sample_b_indexable(texture2d)(float,float,float,float) r2.w, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 134: mad r1.xyz, r5.xyzx, r1.xyzx, -r5.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)+(-(r5.xyzx))).xyz;
    // 135: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 136: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 137: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: add r4.xyz, -r6.xyzx, r2.wwww
    r4.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 139: mad r4.xyz, cb0[21].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[21].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 140: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 141: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 142: mad r4.xyz, cb0[21].zzzz, r5.xyzx, r4.xyzx
    r4.xyz = ((source[21].zzzz)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 143: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 144: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 145: mul r5.xyz, r5.xyzx, cb0[22].yyyy
    r5.xyz = ((r5.xyzx)*(source[22].yyyy)).xyz;
    // 146: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 147: mov_sat r6.xyz, r6.xyzx
    r6.xyz = (saturate(r6.xyzx)).xyz;
    // 148: add r2.w, r6.y, r6.x
    r2.w = ((r6.yyyy)+(r6.xxxx)).w;
    // 149: add r2.w, r6.z, r2.w
    r2.w = ((r6.zzzz)+(r2.wwww)).w;
    // 150: add_sat r2.w, r6.w, r2.w
    r2.w = (saturate((r6.wwww)+(r2.wwww))).w;
    // 151: mad r4.xyz, r2.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 152: dp3 r2.w, r7.xyzx, r3.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 153: mul_sat r2.w, r2.w, cb0[22].z
    r2.w = (saturate((r2.wwww)*(source[22].zzzz))).w;
    // 154: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: mul_sat r3.x, r3.z, cb0[22].z
    r3.x = (saturate((r3.zzzz)*(source[22].zzzz))).x;
    // 156: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 157: add_sat r3.x, r3.x, -cb0[22].w
    r3.x = (saturate((r3.xxxx)+(-(source[22].wwww)))).x;
    // 158: lt r3.y, r3.x, l(0.000001)
    r3.y = (asfloat((uint4)((r3.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 159: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 160: mul r3.x, r3.x, cb0[23].x
    r3.x = ((r3.xxxx)*(source[23].xxxx)).x;
    // 161: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 162: mul r2.w, r2.w, r3.x
    r2.w = ((r2.wwww)*(r3.xxxx)).w;
    // 163: movc r2.w, r3.y, l(0), r2.w
    r2.w = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 164: max r3.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 165: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 166: mul r3.xyz, r3.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 167: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 168: dp3 r3.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 169: log r3.x, r3.x
    r3.x = (log2(r3.xxxx)).x;
    // 170: mul r3.x, r3.x, cb0[23].y
    r3.x = ((r3.xxxx)*(source[23].yyyy)).x;
    // 171: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 172: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 173: mad r3.y, -r3.x, r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 174: max r3.y, r3.y, l(0.001000)
    r3.y = (max(r3.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 175: div r3.y, cb0[23].z, r3.y
    r3.y = ((source[23].zzzz)/(r3.yyyy)).y;
    // 176: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 177: add r3.y, -r3.x, l(1.000000)
    r3.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 178: mul r3.y, r3.y, cb0[23].w
    r3.y = ((r3.yyyy)*(source[23].wwww)).y;
    // 179: mad r4.xyz, r2.wwww, r4.xyzx, -r1.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 180: mad r4.xyz, r3.yyyy, r4.xyzx, r1.xyzx
    r4.xyz = ((r3.yyyy)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 181: mad r4.xyz, r1.wwww, r4.xyzx, -r1.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 182: mad r1.xyz, r3.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 183: mul r1.w, cb0[5].z, l(1.500000)
    r1.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 184: add r2.w, -cb0[5].w, l(1.000000)
    r2.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: mul r2.w, r2.w, cb0[21].w
    r2.w = ((r2.wwww)*(source[21].wwww)).w;
    // 186: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 187: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 188: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 190: mad r1.w, r1.w, l(0.500000), cb0[5].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 191: frc r2.w, cb0[5].x
    r2.w = (frac(source[5].xxxx)).w;
    // 192: add r3.x, -r2.w, cb0[5].x
    r3.x = ((-(r2.wwww))+(source[5].xxxx)).x;
    // 193: mul r4.z, r3.x, l(0.125000)
    r4.z = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 194: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 195: mul r4.y, cb0[5].y, cb0[17].y
    r4.y = ((source[5].yyyy)*(source[17].yyyy)).y;
    // 196: frc r3.x, v4.x
    r3.x = (frac(v4.xxxx)).x;
    // 197: mul r3.x, r3.x, l(0.125000)
    r3.x = ((r3.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 198: mov r3.y, v4.y
    r3.y = (v4.yyyy).y;
    // 199: add r3.xy, r3.xyxx, r4.xyxx
    r3.xy = ((r3.xyxx)+(r4.xyxx)).xy;
    // 200: add r3.xy, r3.xyxx, r4.zwzz
    r3.xy = ((r3.xyxx)+(r4.zwzz)).xy;
    // 201: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 202: mul r3.xyz, r1.wwww, r4.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 203: mul r1.w, r2.w, r4.w
    r1.w = ((r2.wwww)*(r4.wwww)).w;
    // 204: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 205: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 206: mul r1.w, cb0[18].y, cb0[21].w
    r1.w = ((source[18].yyyy)*(source[21].wwww)).w;
    // 207: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 208: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 209: mul r3.y, r1.w, l(0.020000)
    r3.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 210: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 211: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 212: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 213: mad r0.xy, cb0[18].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[18].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 214: mul r0.z, cb0[18].x, l(0.001000)
    r0.z = ((source[18].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 215: mov r3.x, l(0)
    r3.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 216: mad r0.xy, r0.zzzz, r0.xyxx, r3.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r3.xyxx)).xy;
    // 217: dp2 r0.z, cb0[19].xyxx, r0.xyxx
    r0.z = (dot((source[19].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 218: dp2 r0.y, cb0[20].xyxx, r0.xyxx
    r0.y = (dot((source[20].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 219: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 220: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 221: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t5.xyzw, s6, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 222: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 223: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 224: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 225: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 227: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 228: mul r3.xyz, r0.xyzx, cb0[18].zzzz
    r3.xyz = ((r0.xyzx)*(source[18].zzzz)).xyz;
    // 229: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 230: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 231: mad r0.xyz, cb0[18].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[18].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 232: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 233: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 234: mul r1.xyz, r3.wwww, cb2[3].xyzx
    r1.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 235: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 236: mul o0.xyz, r0.xyzx, cb0[25].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)).xyz;
    // 237: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 238: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 239: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 240: ret
    return output;
}

// source.character.mokoko-av036-910.v1 / source program cf1d5c684db76243a7f08565c9b8b72b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight910(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].w=(g_SourceCharacterTime.xxxx).x;
    source[19]=float4(input.lightColor,1.0);
    source[20].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[20].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[20].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s7, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[12].xyzw
    r9.xyzw = ((r8.xyzw)*(source[12].xyzw)).xyzw;
    // 32: add r9.xy, r9.ywyy, r9.xzxx
    r9.xy = ((r9.ywyy)+(r9.xzxx)).xy;
    // 33: add r1.w, r9.y, r9.x
    r1.w = ((r9.yyyy)+(r9.xxxx)).w;
    // 34: add r8.xy, r8.ywyy, r8.xzxx
    r8.xy = ((r8.ywyy)+(r8.xzxx)).xy;
    // 35: add r2.w, r8.y, r8.x
    r2.w = ((r8.yyyy)+(r8.xxxx)).w;
    // 36: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 37: mad_sat r1.w, r2.w, r1.w, l(1.000000)
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 40: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 41: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 42: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 44: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r10.xy, r9.xyxx, cb0[13].xxxx
    r10.xy = ((r9.xyxx)*(source[13].xxxx)).xy;
    // 46: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 49: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 50: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 51: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: mad r9.xyz, cb0[13].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[13].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 53: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 54: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 55: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 56: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 57: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 58: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 59: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 60: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 61: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 63: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 64: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 65: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 66: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 67: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 68: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 69: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 70: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[14].x
    r0.z = ((r0.zzzz)*(source[14].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[14].y
    r1.x = ((r1.xxxx)*(source[14].yyyy)).x;
    // 81: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 82: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 83: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 84: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 85: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 86: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 93: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 94: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 95: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[14].z
    r0.x = (1.0/(source[14].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[14].zzzz
    r9.xyz = ((r4.xyzx)*(source[14].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[14].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[14].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[14].z, l(1.000000)
    r0.x = ((source[14].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 108: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 109: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 110: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 111: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 112: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 113: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 114: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 115: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 117: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 118: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 119: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 120: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 121: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 122: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 123: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 125: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 127: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 128: mul r11.xyz, r9.xyzx, cb0[17].yyyy
    r11.xyz = ((r9.xyzx)*(source[17].yyyy)).xyz;
    // 129: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 130: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 131: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 132: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 133: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 137: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 138: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[15].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[15].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[15].z
    r6.y = (saturate((r6.yyyy)+(-(source[15].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[15].w
    r6.y = ((r6.yyyy)*(source[15].wwww)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 151: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 152: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 155: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 156: mul r0.z, r0.z, cb0[17].w
    r0.z = ((r0.zzzz)*(source[17].wwww)).z;
    // 157: mad r6.y, cb0[17].z, r6.y, -r4.z
    r6.y = ((source[17].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[17].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 166: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 167: mad r7.xyz, cb0[13].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[13].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 168: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 170: mad r7.xyz, cb0[13].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[13].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 171: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 174: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 175: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 177: mad r8.xyz, cb0[13].yyyy, r11.xyzx, r8.xyzx
    r8.xyz = ((source[13].yyyy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 178: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 180: mad r8.xyz, cb0[13].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[13].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 181: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 182: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 184: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 185: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 186: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 187: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 188: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 189: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 190: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 192: mad r2.xyz, cb0[13].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[13].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 193: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 194: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 195: mad r2.xyz, cb0[13].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[13].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 196: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 198: mul r13.xyz, r13.xyzx, cb0[15].xxxx
    r13.xyz = ((r13.xyzx)*(source[15].xxxx)).xyz;
    // 199: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 200: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 201: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 202: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 203: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 204: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 205: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 206: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 207: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 208: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 209: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 210: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 211: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 212: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 213: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 215: div r1.w, cb0[16].y, r1.w
    r1.w = ((source[16].yyyy)/(r1.wwww)).w;
    // 216: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 217: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 218: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 219: mul r1.w, r1.w, cb0[16].z
    r1.w = ((r1.wwww)*(source[16].zzzz)).w;
    // 220: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 221: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 222: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 223: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 224: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 225: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 226: mad r1.xyz, cb0[13].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[13].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 227: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 228: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 229: mad r1.xyz, cb0[13].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[13].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 230: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 231: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 232: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r4.z, r4.z, cb0[16].w
    r4.z = ((r4.zzzz)*(source[16].wwww)).z;
    // 234: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 235: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 236: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 238: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 239: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 240: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 241: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 242: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 243: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 244: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 245: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 246: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 247: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 248: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 249: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 250: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 251: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 252: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 253: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 254: mul r1.w, cb0[9].y, cb0[16].w
    r1.w = ((source[9].yyyy)*(source[16].wwww)).w;
    // 255: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 256: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 257: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 258: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 259: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 260: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 261: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 262: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 263: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 264: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 265: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 266: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 267: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 268: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 270: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 271: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 272: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 274: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 275: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 276: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 277: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 278: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 279: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 280: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 281: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 282: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 283: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 284: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 285: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 286: mul r0.y, r2.w, cb0[18].x
    r0.y = ((r2.wwww)*(source[18].xxxx)).y;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 288: add r0.w, -cb0[18].y, l(2.000000)
    r0.w = ((-(source[18].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 289: mad r0.w, r4.x, r0.w, cb0[18].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].yyyy)).w;
    // 290: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 291: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 292: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 293: mul r0.xyz, r0.xyzx, cb0[18].zzzz
    r0.xyz = ((r0.xyzx)*(source[18].zzzz)).xyz;
    // 294: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 297: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 298: mul r0.xyz, r0.xyzx, cb0[18].wwww
    r0.xyz = ((r0.xyzx)*(source[18].wwww)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 301: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 302: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 303: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 304: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 305: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 306: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 307: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 308: ret
    return output;
}

// source.character.mokoko-av036-911.v1 / source program 275d8c40a741a9468b9d366278929ce2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight911(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].y=(g_SourceCharacterTime.xxxx).x;
    source[15].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[15].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[16].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[16].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[18]=float4(input.lightColor,1.0);
    source[24].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 4: add r1.xyz, v8.xyzx, cb0[0].xyzx
    r1.xyz = ((v8.xyzx)+(source[0].xyzx)).xyz;
    // 5: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].yyyy)) * 0xffffffffu)).w;
    // 6: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 7: mul r2.xyzw, v8.yyyy, cb1[1].xyzw
    r2.xyzw = ((v8.yyyy)*(projection[1].xyzw)).xyzw;
    // 8: mad r2.xyzw, cb1[0].xyzw, v8.xxxx, r2.xyzw
    r2.xyzw = ((projection[0].xyzw)*(v8.xxxx)+(r2.xyzw)).xyzw;
    // 9: mad r2.xyzw, cb1[2].xyzw, v8.zzzz, r2.xyzw
    r2.xyzw = ((projection[2].xyzw)*(v8.zzzz)+(r2.xyzw)).xyzw;
    // 10: mad r2.xyzw, cb1[3].xyzw, v8.wwww, r2.xyzw
    r2.xyzw = ((projection[3].xyzw)*(v8.wwww)+(r2.xyzw)).xyzw;
    // 11: mul r3.xyzw, r2.yyyy, cb0[20].xyzw
    r3.xyzw = ((r2.yyyy)*(source[20].xyzw)).xyzw;
    // 12: mad r3.xyzw, cb0[19].xyzw, r2.xxxx, r3.xyzw
    r3.xyzw = ((source[19].xyzw)*(r2.xxxx)+(r3.xyzw)).xyzw;
    // 13: mad r3.xyzw, cb0[21].xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = ((source[21].xyzw)*(r2.zzzz)+(r3.xyzw)).xyzw;
    // 14: mad r2.xyzw, cb0[22].xyzw, r2.wwww, r3.xyzw
    r2.xyzw = ((source[22].xyzw)*(r2.wwww)+(r3.xyzw)).xyzw;
    // 15: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t0.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 17: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 18: mov r4.yz, cb0[23].wwzw
    r4.yz = (source[23].wwzw).yz;
    // 19: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t0.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t0.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 22: add r4.xy, r2.xyxx, cb0[23].zwzz
    r4.xy = ((r2.xyxx)+(source[23].zwzz)).xy;
    // 23: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t0.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 24: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 25: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 26: mul r2.xy, r2.xyxx, cb0[23].xyxx
    r2.xy = ((r2.xyxx)*(source[23].xyxx)).xy;
    // 27: frc r2.xy, r2.xyxx
    r2.xy = (frac(r2.xyxx)).xy;
    // 28: movc r2.zw, r3.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r2.zw = ((asuint(r3.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 29: add r2.zw, r2.zzzw, r4.zzzw
    r2.zw = ((r2.zzzw)+(r4.zzzw)).zw;
    // 30: mad r2.xz, r2.xxxx, r2.zzwz, r4.xxyx
    r2.xz = ((r2.xxxx)*(r2.zzwz)+(r4.xxyx)).xz;
    // 31: add r0.w, -r2.x, r2.z
    r0.w = ((-(r2.xxxx))+(r2.zzzz)).w;
    // 32: mad r0.w, r2.y, r0.w, r2.x
    r0.w = ((r2.yyyy)*(r0.wwww)+(r2.xxxx)).w;
    // 33: mul r2.xyz, r0.wwww, cb0[24].xxxx
    r2.xyz = ((r0.wwww)*(source[24].xxxx)).xyz;
    // 34: else
    } else {
    // 35: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 36: endif
    }
    // 37: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 38: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 39: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 40: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 41: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 42: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 43: add r5.xyz, -cb0[2].xyzx, cb0[3].xyzx
    r5.xyz = ((-(source[2].xyzx))+(source[3].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[13].xxxx
    r5.xyz = ((r5.xyzx)*(source[13].xxxx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s1, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mad r5.xyz, r6.yyyy, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r6.yyyy)*(r5.xyzx)+(source[2].xyzx)).xyz;
    // 47: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 48: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 49: mad r5.xyz, cb0[13].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 50: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 51: add r7.xyz, -r5.xyzx, r1.wwww
    r7.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 52: mad r5.xyz, cb0[13].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[13].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 53: mad r7.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mad r8.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 56: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 57: mul r9.xyz, r6.xxxx, r8.xyzx
    r9.xyz = ((r6.xxxx)*(r8.xyzx)).xyz;
    // 58: mad r8.xyz, r6.xxxx, r8.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r8.xyz = ((r6.xxxx)*(r8.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 59: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 60: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 61: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 62: mul r8.xyz, r8.xyzx, cb0[7].xyzx
    r8.xyz = ((r8.xyzx)*(source[7].xyzx)).xyz;
    // 63: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 65: mad r10.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 66: dp2 r2.w, r10.xyxx, r10.xyxx
    r2.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 67: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 69: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 70: add r10.z, r2.w, l(0.000010)
    r10.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 71: dp3 r2.w, r10.xyzx, r10.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 72: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 73: div r10.xyz, r10.xyzx, r2.wwww
    r10.xyz = ((r10.xyzx)/(r2.wwww)).xyz;
    // 74: dp3 r2.w, r10.xyzx, r3.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 75: add r2.w, -|r2.w|, l(1.000000)
    r2.w = ((-(abs(r2.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 77: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 78: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 79: mul r1.w, r1.w, cb0[15].x
    r1.w = ((r1.wwww)*(source[15].xxxx)).w;
    // 80: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 81: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 82: mul r2.w, cb0[8].z, l(1.500000)
    r2.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 83: add r4.w, -cb0[8].w, l(1.000000)
    r4.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 84: mul r4.w, r4.w, cb0[15].y
    r4.w = ((r4.wwww)*(source[15].yyyy)).w;
    // 85: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 86: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 87: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 89: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 90: frc r4.w, cb0[8].x
    r4.w = (frac(source[8].xxxx)).w;
    // 91: add r5.w, -r4.w, cb0[8].x
    r5.w = ((-(r4.wwww))+(source[8].xxxx)).w;
    // 92: mul r11.z, r5.w, l(0.125000)
    r11.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 93: mov r11.xw, l(0,0,0,0)
    r11.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 94: mul r11.y, cb0[8].y, cb0[9].y
    r11.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 95: mul r12.xz, v4.xxyx, l(0.500000, 0.000000, 0.500000, 0.000000)
    r12.xz = ((v4.xxyx)*(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 96: frc r5.w, r12.x
    r5.w = (frac(r12.xxxx)).w;
    // 97: mul r12.y, r5.w, l(0.125000)
    r12.y = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).y;
    // 98: add r6.xy, r11.xyxx, r12.yzyy
    r6.xy = ((r11.xyxx)+(r12.yzyy)).xy;
    // 99: add r6.xy, r6.xyxx, r11.zwzz
    r6.xy = ((r6.xyxx)+(r11.zwzz)).xy;
    // 100: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 101: mul r11.xyz, r2.wwww, r11.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)).xyz;
    // 102: mul r2.w, r4.w, r11.w
    r2.w = ((r4.wwww)*(r11.wwww)).w;
    // 103: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 104: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 105: mul r2.w, cb0[10].y, cb0[15].y
    r2.w = ((source[10].yyyy)*(source[15].yyyy)).w;
    // 106: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 107: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 108: mul r6.y, r2.w, l(0.020000)
    r6.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 109: add r11.xyzw, r1.yzxy, -cb0[1].yzxy
    r11.xyzw = ((r1.yzxy)+(-(source[1].yzxy))).xyzw;
    // 110: add r11.xy, -r11.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r11.xy = ((-(r11.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 111: add r11.xy, -r11.zwzz, r11.xyxx
    r11.xy = ((-(r11.zwzz))+(r11.xyxx)).xy;
    // 112: mad r11.xy, cb0[10].wwww, r11.xyxx, r11.zwzz
    r11.xy = ((source[10].wwww)*(r11.xyxx)+(r11.zwzz)).xy;
    // 113: mul r4.w, cb0[10].x, l(0.001000)
    r4.w = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 114: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 115: mad r6.xy, r4.wwww, r11.xyxx, r6.xyxx
    r6.xy = ((r4.wwww)*(r11.xyxx)+(r6.xyxx)).xy;
    // 116: dp2 r4.w, cb0[11].xyxx, r6.xyxx
    r4.w = (dot((source[11].xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 117: dp2 r6.y, cb0[12].xyxx, r6.xyxx
    r6.y = (dot((source[12].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 118: frc r4.w, r4.w
    r4.w = (frac(r4.wwww)).w;
    // 119: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 120: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 121: mul r4.w, r11.w, l(0.900000)
    r4.w = ((r11.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 122: mad r11.xyz, r11.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r9.xyzx))).xyz;
    // 123: mad r11.xyz, r4.wwww, r11.xyzx, r9.xyzx
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 124: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 126: mul_sat r11.xyz, r11.xyzx, r2.wwww
    r11.xyz = (saturate((r11.xyzx)*(r2.wwww))).xyz;
    // 127: mul r12.xyz, r11.xyzx, cb0[10].zzzz
    r12.xyz = ((r11.xyzx)*(source[10].zzzz)).xyz;
    // 128: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mul r2.w, r2.w, l(3.000000)
    r2.w = ((r2.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 130: mad r11.xyz, cb0[10].zzzz, r11.xyzx, -r9.xyzx
    r11.xyz = ((source[10].zzzz)*(r11.xyzx)+(-(r9.xyzx))).xyz;
    // 131: mad r9.xyz, r2.wwww, r11.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r11.xyzx)+(r9.xyzx)).xyz;
    // 132: dp3 r2.w, r10.xyzx, r4.xyzx
    r2.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 133: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mul r11.xyz, r2.xyzx, r5.wwww
    r11.xyz = ((r2.xyzx)*(r5.wwww)).xyz;
    // 136: mad r12.xyz, -r5.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r5.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: mad r11.yzw, cb0[16].zzzz, r12.xxyz, r11.xxyz
    r11.yzw = ((source[16].zzzz)*(r12.xxyz)+(r11.xxyz)).yzw;
    // 138: dp3 r5.w, cb0[2].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((source[2].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 139: add r12.xyz, r5.wwww, -cb0[2].xyzx
    r12.xyz = ((r5.wwww)+(-(source[2].xyzx))).xyz;
    // 140: mad r12.xyz, cb0[13].yyyy, r12.xyzx, cb0[2].xyzx
    r12.xyz = ((source[13].yyyy)*(r12.xyzx)+(source[2].xyzx)).xyz;
    // 141: dp3 r5.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 142: add r13.xyz, -r12.xyzx, r5.wwww
    r13.xyz = ((-(r12.xyzx))+(r5.wwww)).xyz;
    // 143: mad r12.xyz, cb0[13].zzzz, r13.xyzx, r12.xyzx
    r12.xyz = ((source[13].zzzz)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 144: mul r13.xyz, r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r13.xyz = ((r12.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 145: mad r14.xyz, -r12.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 146: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r2.w, r2.w, l(0.500000)
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 148: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 149: mul r13.xyz, r13.xyzx, r2.wwww
    r13.xyz = ((r13.xyzx)*(r2.wwww)).xyz;
    // 150: mad r11.yzw, r11.yyzw, r14.xxyz, r13.xxyz
    r11.yzw = ((r11.yyzw)*(r14.xxyz)+(r13.xxyz)).yzw;
    // 151: mov_sat r2.w, r4.z
    r2.w = (saturate(r4.zzzz)).w;
    // 152: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 153: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 154: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: add r13.xyz, -r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r12.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mad r12.xyz, r2.wwww, r13.xyzx, r12.xyzx
    r12.xyz = ((r2.wwww)*(r13.xyzx)+(r12.xyzx)).xyz;
    // 157: mad r12.xyz, r11.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r12.xyzx
    r12.xyz = ((r11.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r12.xyzx)).xyz;
    // 158: mul_sat r9.xyz, r9.xyzx, r12.xyzx
    r9.xyz = (saturate((r9.xyzx)*(r12.xyzx))).xyz;
    // 159: max r2.xyz, r2.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 160: mad r5.xyz, r5.xyzx, r7.xyzx, l(0.001000, 0.001000, 0.001000, 0.000000)
    r5.xyz = ((r5.xyzx)*(r7.xyzx)+(float4(0.001000,0.001000,0.001000,0.000000))).xyz;
    // 161: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 162: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 163: div r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 164: mul r5.xyz, r5.xyzx, r6.zzzz
    r5.xyz = ((r5.xyzx)*(r6.zzzz)).xyz;
    // 165: mov_sat r2.w, r3.z
    r2.w = (saturate(r3.zzzz)).w;
    // 166: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 167: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 168: mul r5.xyz, r5.xyzx, r2.wwww
    r5.xyz = ((r5.xyzx)*(r2.wwww)).xyz;
    // 169: mul r5.xyz, r5.xyzx, cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(source[13].wwww)).xyz;
    // 170: max r2.w, r11.x, l(0.500000)
    r2.w = (max(r11.xxxx,float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 171: min r2.xyzw, r2.xyzw, l(1.000000, 1.000000, 1.000000, 1.000000)
    r2.xyzw = (min(r2.xyzw,float4(1.000000,1.000000,1.000000,1.000000))).xyzw;
    // 172: dp3 r5.w, r0.xyzx, r10.xyzx
    r5.w = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 173: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 174: add r1.xyz, -r1.xyzx, cb0[0].xyzx
    r1.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 175: mul r3.w, r3.z, r1.z
    r3.w = ((r3.zzzz)*(r1.zzzz)).w;
    // 176: mad r1.xyz, r3.zzwz, l(0.000000, 0.000000, -0.990000, 0.000000), r1.xyzx
    r1.xyz = ((r3.zzwz)*(float4(0.000000,0.000000,-0.990000,0.000000))+(r1.xyzx)).xyz;
    // 177: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 178: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 179: div r0.y, r1.z, r0.y
    r0.y = ((r1.zzzz)/(r0.yyyy)).y;
    // 180: add r0.y, r0.y, cb0[6].z
    r0.y = ((r0.yyyy)+(source[6].zzzz)).y;
    // 181: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 182: add r0.x, r0.x, r5.w
    r0.x = ((r0.xxxx)+(r5.wwww)).x;
    // 183: frc r0.x, r0.x
    r0.x = (frac(r0.xxxx)).x;
    // 184: add r0.x, r0.x, l(-0.500000)
    r0.x = ((r0.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 185: add r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)+(r0.xxxx)).x;
    // 186: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: mad r0.y, cb0[14].z, l(4.500000), l(0.500000)
    r0.y = ((source[14].zzzz)*(float4(4.500000,4.500000,4.500000,4.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 188: mul r0.y, r0.y, cb0[16].w
    r0.y = ((r0.yyyy)*(source[16].wwww)).y;
    // 189: mul r0.y, r0.y, l(0.050000)
    r0.y = ((r0.yyyy)*(float4(0.050000,0.050000,0.050000,0.050000))).y;
    // 190: lt r0.z, r0.x, l(0.000001)
    r0.z = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 191: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 192: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 193: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 194: mul r0.x, r2.w, r0.x
    r0.x = ((r2.wwww)*(r0.xxxx)).x;
    // 195: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 196: mad r0.yzw, v5.xxyz, r0.wwww, r3.xxyz
    r0.yzw = ((v5.xxyz)*(r0.wwww)+(r3.xxyz)).yzw;
    // 197: dp3 r0.y, r0.yzwy, r0.yzwy
    r0.y = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).y;
    // 198: mad r0.y, r0.y, r0.y, l(0.100000)
    r0.y = ((r0.yyyy)*(r0.yyyy)+(float4(0.100000,0.100000,0.100000,0.100000))).y;
    // 199: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 200: div r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)/(r0.yyyy)).x;
    // 201: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 202: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 203: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 204: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 205: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 206: mad r0.xyz, r9.xyzx, r11.yzwy, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r11.yzwy)+(r0.xyzx)).xyz;
    // 207: mad r0.xyz, r1.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 208: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 209: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 210: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 211: mul o0.w, r6.w, cb0[1].w
    output.targets[0].w = ((r6.wwww)*(source[1].wwww)).w;
    // 212: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 213: ret
    return output;
}

// source.character.mokoko-av036-912.v1 / source program aa069d352b32824090dc0a09bef61bb0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight912(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].w=(g_SourceCharacterTime.xxxx).x;
    source[18]=float4(input.lightColor,1.0);
    source[19].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[19].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[19].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mov_sat r8.x, r8.x
    r8.x = (saturate(r8.xxxx)).x;
    // 32: add r1.w, r8.x, l(-0.333300)
    r1.w = ((r8.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 33: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 34: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 38: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 39: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 44: mad r9.xyz, cb0[12].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[12].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 45: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 46: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 47: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 48: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 49: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 50: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 51: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 52: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 53: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 54: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 55: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 56: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 57: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 58: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 59: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 60: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 61: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 62: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 67: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 71: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 72: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 73: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 74: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 75: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 76: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 77: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 78: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 79: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 80: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 81: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 82: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 83: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 84: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 85: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 86: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 87: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t3.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 89: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 90: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 91: mul r9.xyz, r4.xyzx, cb0[13].zzzz
    r9.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 92: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 93: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 94: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 95: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 96: mad r4.xyz, r9.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 97: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 98: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 99: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 100: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 101: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 102: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 103: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 104: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 105: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 106: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 107: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 108: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 109: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 110: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 111: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 112: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 113: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 114: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 115: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 116: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 117: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 119: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 120: mul r11.xyz, r9.xyzx, cb0[16].yyyy
    r11.xyz = ((r9.xyzx)*(source[16].yyyy)).xyz;
    // 121: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 122: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 123: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 124: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 125: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 126: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 128: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 129: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 130: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 132: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 133: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 134: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 135: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 136: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 137: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 138: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 139: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 140: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 141: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 142: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 143: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 144: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 145: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 147: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 148: mul r0.z, r0.z, cb0[16].w
    r0.z = ((r0.zzzz)*(source[16].wwww)).z;
    // 149: mad r6.y, cb0[16].z, r6.y, -r4.z
    r6.y = ((source[16].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 150: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 151: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 152: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 153: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 154: mad r6.yzw, -cb0[16].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[16].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 155: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 156: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 157: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 158: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 159: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 160: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 161: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 162: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 163: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 166: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 167: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 168: add r11.xyz, -r8.yzwy, r0.zzzz
    r11.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 169: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.yzwy
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.yzwy)).xyz;
    // 170: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 173: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 176: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 177: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 178: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 179: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 180: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 181: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 182: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 183: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 184: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 185: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 186: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 187: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 189: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 190: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 191: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 192: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 193: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 194: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 195: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 196: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 197: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 198: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 199: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 200: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 201: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 202: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 203: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 204: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 205: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 207: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 208: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 209: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 210: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 212: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 213: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 214: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 215: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 216: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 217: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 218: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 219: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 220: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 221: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 222: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 223: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 224: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r4.z, r4.z, cb0[15].w
    r4.z = ((r4.zzzz)*(source[15].wwww)).z;
    // 226: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 227: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 228: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 230: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 231: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 232: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 233: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 235: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 236: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 237: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 238: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 239: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 240: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 242: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 243: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 244: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.w, cb0[9].y, cb0[15].w
    r1.w = ((source[9].yyyy)*(source[15].wwww)).w;
    // 247: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 248: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 249: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 250: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 251: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 252: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 253: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 254: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 255: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 256: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 257: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 258: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 259: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 260: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 262: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 263: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 264: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 265: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 266: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 267: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 268: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 269: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 270: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 271: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 272: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 274: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 275: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 276: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 277: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 278: mul r0.y, r2.w, cb0[17].x
    r0.y = ((r2.wwww)*(source[17].xxxx)).y;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 280: add r0.w, -cb0[17].y, l(2.000000)
    r0.w = ((-(source[17].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 281: mad r0.w, r4.x, r0.w, cb0[17].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[17].yyyy)).w;
    // 282: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 283: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 284: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 285: mul r0.xyz, r0.xyzx, cb0[17].zzzz
    r0.xyz = ((r0.xyzx)*(source[17].zzzz)).xyz;
    // 286: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 287: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 288: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 289: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 290: mul r0.xyz, r0.xyzx, cb0[17].wwww
    r0.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 293: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 294: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 295: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 296: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 297: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 298: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 299: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 300: ret
    return output;
}

// source.character.mokoko-av036-913.v1 / source program ce2128581b5ffc4e94d7ace43cc503b9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight913(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12].x=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
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
    // 10: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
    // 11: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 12: div r3.xy, v8.xyxx, v8.wwww
    r3.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 13: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 14: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t7.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 15: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 16: else
    } else {
    // 17: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 18: endif
    }
    // 19: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r4.xyz, r0.wwww, v7.zxyz
    r4.xyz = ((r0.wwww)*(v7.zxyz)).xyz;
    // 22: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r5.xyz, r0.wwww, v5.xyzx
    r5.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r6.xyzw, r6.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r6.xyzw = ((r6.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 27: dp2 r1.w, r6.zwzz, r6.zwzz
    r1.w = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).w;
    // 28: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 32: mul r7.xy, r6.xyxx, cb0[11].xxxx
    r7.xy = ((r6.xyxx)*(source[11].xxxx)).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mad r6.xy, cb0[11].wwww, r6.zwzz, -r7.xyxx
    r6.xy = ((source[11].wwww)*(r6.zwzz)+(-(r7.xyxx))).xy;
    // 35: mov r6.z, l(0)
    r6.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 36: mad r6.xyz, r8.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r8.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 37: add r7.xyz, -r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r6.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 38: mad r9.xyz, cb0[13].zzzz, r7.xyzx, r6.xyzx
    r9.xyz = ((source[13].zzzz)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 39: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 42: dp3 r10.x, r1.xyzx, r9.xyzx
    r10.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 43: dp3 r10.y, r2.xyzx, r9.xyzx
    r10.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 44: dp3 r10.z, r0.xyzx, r9.xyzx
    r10.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 45: dp3 r1.x, r1.zxyz, r4.xyzx
    r1.x = (dot((r1.zxyz).xyz,(r4.xyzx).xyz).xxxx).x;
    // 46: dp3 r1.y, r2.zxyz, r4.xyzx
    r1.y = (dot((r2.zxyz).xyz,(r4.xyzx).xyz).xxxx).y;
    // 47: dp3 r1.z, r0.zxyz, r4.xyzx
    r1.z = (dot((r0.zxyz).xyz,(r4.xyzx).xyz).xxxx).z;
    // 48: dp3 r0.x, r10.xyzx, r1.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 49: mul r0.xyz, r10.xyzx, r0.xxxx
    r0.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 50: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 51: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 52: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 53: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 54: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 55: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 56: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 57: add r0.z, cb0[13].w, -cb0[14].x
    r0.z = ((source[13].wwww)+(-(source[14].xxxx))).z;
    // 58: mad r0.z, r8.x, r0.z, cb0[14].x
    r0.z = ((r8.xxxx)*(r0.zzzz)+(source[14].xxxx)).z;
    // 59: add r1.x, -r0.z, cb0[14].y
    r1.x = ((-(r0.zzzz))+(source[14].yyyy)).x;
    // 60: mad r0.z, r8.y, r1.x, r0.z
    r0.z = ((r8.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 61: mul r1.x, cb0[0].z, cb0[12].x
    r1.x = ((source[0].zzzz)*(source[12].xxxx)).x;
    // 62: sincos r1.x, r2.x, r1.x
    r1.x = (sin(r1.xxxx)).x; r2.x = (cos(r1.xxxx)).x;
    // 63: add r1.yw, v4.xxxy, -cb0[0].xxxy
    r1.yw = ((v4.xxxy)+(-(source[0].xxxy))).yw;
    // 64: mov r9.x, -r1.x
    r9.x = (-(r1.xxxx)).x;
    // 65: mov r9.y, r2.x
    r9.y = (r2.xxxx).y;
    // 66: dp2 r2.x, r1.wyww, r9.xyxx
    r2.x = (dot((r1.wyww).xy,(r9.xyxx).xy).xxxx).x;
    // 67: mov r9.z, r1.x
    r9.z = (r1.xxxx).z;
    // 68: dp2 r2.y, r1.wyww, r9.yzyy
    r2.y = (dot((r1.wyww).xy,(r9.yzyy).xy).xxxx).y;
    // 69: add r1.xy, r2.xyxx, cb0[0].xyxx
    r1.xy = ((r2.xyxx)+(source[0].xyxx)).xy;
    // 70: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r1.xyxx, t1.xywz, s2, l(0.000000)
    r1.w = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).w;
    // 71: add r1.w, -r8.z, r1.w
    r1.w = ((-(r8.zzzz))+(r1.wwww)).w;
    // 72: mad r1.w, r8.y, r1.w, r8.z
    r1.w = ((r8.yyyy)*(r1.wwww)+(r8.zzzz)).w;
    // 73: add r2.x, -r0.z, cb0[14].z
    r2.x = ((-(r0.zzzz))+(source[14].zzzz)).x;
    // 74: mad r0.z, r1.w, r2.x, r0.z
    r0.z = ((r1.wwww)*(r2.xxxx)+(r0.zzzz)).z;
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r1.xyxx, t2.xyzw, s3, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 77: add r9.xyzw, -r2.wxyz, r9.wxyz
    r9.xyzw = ((-(r2.wxyz))+(r9.wxyz)).xyzw;
    // 78: mad r9.xyzw, r8.yyyy, r9.xyzw, r2.wxyz
    r9.xyzw = ((r8.yyyy)*(r9.xyzw)+(r2.wxyz)).xyzw;
    // 79: add r2.y, -r9.x, l(1.000000)
    r2.y = ((-(r9.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 80: add r2.z, -cb0[12].z, cb0[12].y
    r2.z = ((-(source[12].zzzz))+(source[12].yyyy)).z;
    // 81: mad r2.z, r8.x, r2.z, cb0[12].z
    r2.z = ((r8.xxxx)*(r2.zzzz)+(source[12].zzzz)).z;
    // 82: add r2.w, -r2.z, cb0[12].w
    r2.w = ((-(r2.zzzz))+(source[12].wwww)).w;
    // 83: mad r2.z, r8.y, r2.w, r2.z
    r2.z = ((r8.yyyy)*(r2.wwww)+(r2.zzzz)).z;
    // 84: add r2.w, -r2.z, cb0[13].x
    r2.w = ((-(r2.zzzz))+(source[13].xxxx)).w;
    // 85: mad r2.z, r1.w, r2.w, r2.z
    r2.z = ((r1.wwww)*(r2.wwww)+(r2.zzzz)).z;
    // 86: add r2.w, -r2.z, cb0[13].y
    r2.w = ((-(r2.zzzz))+(source[13].yyyy)).w;
    // 87: mad r2.z, r8.w, r2.w, r2.z
    r2.z = ((r8.wwww)*(r2.wwww)+(r2.zzzz)).z;
    // 88: lt r2.w, |r2.y|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: log r2.y, |r2.y|
    r2.y = (log2(abs(r2.yyyy))).y;
    // 90: mul r2.y, r2.y, r2.z
    r2.y = ((r2.yyyy)*(r2.zzzz)).y;
    // 91: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 92: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 93: movc r2.y, r2.w, l(0), r2.y
    r2.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).y;
    // 94: sqrt r2.z, r2.y
    r2.z = (sqrt(r2.yyyy)).z;
    // 95: add r2.w, -r2.z, cb0[14].w
    r2.w = ((-(r2.zzzz))+(source[14].wwww)).w;
    // 96: mad r2.z, r8.w, r2.w, r2.z
    r2.z = ((r8.wwww)*(r2.wwww)+(r2.zzzz)).z;
    // 97: mul r0.z, r0.z, r2.z
    r0.z = ((r0.zzzz)*(r2.zzzz)).z;
    // 98: div r2.zw, v8.xxxy, v8.wwww
    r2.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 99: mad r2.zw, r2.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r2.zw = ((r2.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 100: mul r2.zw, r2.zzzw, l(0.000000, 0.000000, 700.000000, 700.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,700.000000,700.000000))).zw;
    // 101: deriv_rtx_coarse r10.xy, r2.zwzz
    r10.xy = (ddx_coarse(r2.zwzz)).xy;
    // 102: deriv_rty_coarse r2.zw, r2.zzzw
    r2.zw = (ddy_coarse(r2.zzzw)).zw;
    // 103: dp2 r3.w, r10.xyxx, r10.xyxx
    r3.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 104: dp2 r2.z, r2.zwzz, r2.zwzz
    r2.z = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).z;
    // 105: max r2.z, r2.z, r3.w
    r2.z = (max(r2.zzzz,r3.wwww)).z;
    // 106: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 107: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 108: rcp r2.w, |r2.z|
    r2.w = (1.0/(abs(r2.zzzz))).w;
    // 109: mul r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)*(r2.wwww)).z;
    // 110: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 111: add r0.z, r0.z, |r2.z|
    r0.z = ((r0.zzzz)+(abs(r2.zzzz))).z;
    // 112: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 113: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 114: rcp r2.z, cb0[15].x
    r2.z = (1.0/(source[15].xxxx)).z;
    // 115: log r10.xyz, r0.xyzx
    r10.xyz = (log2(r0.xyzx)).xyz;
    // 116: mul r11.xyz, r10.xyzx, cb0[15].xxxx
    r11.xyz = ((r10.xyzx)*(source[15].xxxx)).xyz;
    // 117: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 118: mul r10.xyz, r2.zzzz, r10.xyzx
    r10.xyz = ((r2.zzzz)*(r10.xyzx)).xyz;
    // 119: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 120: mul r10.xyz, r2.zzzz, r10.xyzx
    r10.xyz = ((r2.zzzz)*(r10.xyzx)).xyz;
    // 121: mad r10.xyz, r11.xyzx, cb0[15].xxxx, r10.xyzx
    r10.xyz = ((r11.xyzx)*(source[15].xxxx)+(r10.xyzx)).xyz;
    // 122: add r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)+(r10.xyzx)).xyz;
    // 123: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 124: add r2.z, cb0[15].x, l(1.000000)
    r2.z = ((source[15].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 125: mul r0.xyz, r0.xyzx, r2.zzzz
    r0.xyz = ((r0.xyzx)*(r2.zzzz)).xyz;
    // 126: mad r10.xyz, v5.xyzx, r0.wwww, r4.yzxy
    r10.xyz = ((v5.xyzx)*(r0.wwww)+(r4.yzxy)).xyz;
    // 127: dp3 r0.w, r10.xyzx, r10.xyzx
    r0.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 128: sqrt r2.z, r0.w
    r2.z = (sqrt(r0.wwww)).z;
    // 129: div r10.xyz, r10.xyzx, r2.zzzz
    r10.xyz = ((r10.xyzx)/(r2.zzzz)).xyz;
    // 130: dp3 r2.z, r10.zxyz, r4.xyzx
    r2.z = (dot((r10.zxyz).xyz,(r4.xyzx).xyz).xxxx).z;
    // 131: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 132: lt r2.w, |r2.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 133: mul r3.w, |r2.z|, |r2.z|
    r3.w = ((abs(r2.zzzz))*(abs(r2.zzzz))).w;
    // 134: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 135: mul r2.z, |r2.z|, r3.w
    r2.z = ((abs(r2.zzzz))*(r3.wwww)).z;
    // 136: movc r2.z, r2.w, l(0), r2.z
    r2.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).z;
    // 137: add r2.w, r2.z, l(-0.027778)
    r2.w = ((r2.zzzz)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 138: mad r2.z, r2.z, r2.w, l(0.027778)
    r2.z = ((r2.zzzz)*(r2.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).z;
    // 139: div r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)/(r0.wwww)).w;
    // 140: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 141: min r2.zw, r0.wwww, l(0.000000, 0.000000, 1.000000, 3.000000)
    r2.zw = (min(r0.wwww,float4(0.000000,0.000000,1.000000,3.000000))).zw;
    // 142: add r0.w, -r2.z, l(1.000000)
    r0.w = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r2.z, r0.w, r2.y
    r2.z = ((r0.wwww)*(r2.yyyy)).z;
    // 144: mul r11.xyz, r0.xyzx, r2.zzzz
    r11.xyz = ((r0.xyzx)*(r2.zzzz)).xyz;
    // 145: mul r12.xyz, r11.xyzx, cb0[17].xxxx
    r12.xyz = ((r11.xyzx)*(source[17].xxxx)).xyz;
    // 146: mul r3.w, r8.w, l(0.500000)
    r3.w = ((r8.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 147: add r4.w, -|r5.z|, l(1.000000)
    r4.w = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 149: mul r4.w, r2.y, r4.w
    r4.w = ((r2.yyyy)*(r4.wwww)).w;
    // 150: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 151: mad r6.xyz, r3.wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((r3.wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 152: dp3 r3.w, r6.xyzx, r6.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 153: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 154: div r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)/(r3.wwww)).xyz;
    // 155: dp3 r3.w, r6.xyzx, r5.xyzx
    r3.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 156: max r4.w, r3.w, l(0.000000)
    r4.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 157: min r5.x, r4.w, l(1.000000)
    r5.x = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 158: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: dp3 r5.y, cb0[10].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.y = (dot((source[10].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 160: add r5.yzw, r5.yyyy, -cb0[10].xxyz
    r5.yzw = ((r5.yyyy)+(-(source[10].xxyz))).yzw;
    // 161: mad r5.yzw, r5.xxxx, r5.yyzw, cb0[10].xxyz
    r5.yzw = ((r5.xxxx)*(r5.yyzw)+(source[10].xxyz)).yzw;
    // 162: add r5.yzw, r5.yyzw, l(0.000000, -1.000000, -1.000000, -1.000000)
    r5.yzw = ((r5.yyzw)+(float4(0.000000,-1.000000,-1.000000,-1.000000))).yzw;
    // 163: mad r5.yzw, r8.wwww, r5.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r5.yzw = ((r8.wwww)*(r5.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 164: mul r5.yzw, r2.yyyy, r5.yyzw
    r5.yzw = ((r2.yyyy)*(r5.yyzw)).yzw;
    // 165: mad r3.w, r3.w, l(0.500000), -r5.x
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.xxxx))).w;
    // 166: mad r5.yzw, r5.yyzw, r3.wwww, r5.xxxx
    r5.yzw = ((r5.yyzw)*(r3.wwww)+(r5.xxxx)).yzw;
    // 167: add_sat r3.w, r8.w, cb0[17].y
    r3.w = (saturate((r8.wwww)+(source[17].yyyy))).w;
    // 168: mad r0.w, -r2.y, r0.w, l(1.000000)
    r0.w = ((-(r2.yyyy))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: dp3_sat r4.y, r6.zxyz, r4.xyzx
    r4.y = (saturate(dot((r6.zxyz).xyz,(r4.xyzx).xyz).xxxx)).y;
    // 170: mov_sat r4.x, r4.x
    r4.x = (saturate(r4.xxxx)).x;
    // 171: add r4.xy, -r4.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r4.xy = ((-(r4.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 172: mul r4.z, r4.x, r4.y
    r4.z = ((r4.xxxx)*(r4.yyyy)).z;
    // 173: mad r4.x, -r4.y, r4.x, l(1.000000)
    r4.x = ((-(r4.yyyy))*(r4.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 174: mul r7.xyz, r5.yzwy, r4.xxxx
    r7.xyz = ((r5.yzwy)*(r4.xxxx)).xyz;
    // 175: mad r4.x, r0.w, l(2.000000), -r2.z
    r4.x = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r2.zzzz))).x;
    // 176: mad r7.xyz, r7.xyzx, r4.xxxx, r2.zzzz
    r7.xyz = ((r7.xyzx)*(r4.xxxx)+(r2.zzzz)).xyz;
    // 177: add r7.xyz, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul_sat r7.xyz, r5.yzwy, r7.xyzx
    r7.xyz = (saturate((r5.yzwy)*(r7.xyzx))).xyz;
    // 179: mul r4.x, r2.y, r2.y
    r4.x = ((r2.yyyy)*(r2.yyyy)).x;
    // 180: mul r4.y, r4.x, cb0[17].z
    r4.y = ((r4.xxxx)*(source[17].zzzz)).y;
    // 181: mad r2.y, -r4.x, cb0[17].z, r2.y
    r2.y = ((-(r4.xxxx))*(source[17].zzzz)+(r2.yyyy)).y;
    // 182: mad r2.y, r8.w, r2.y, r4.y
    r2.y = ((r8.wwww)*(r2.yyyy)+(r4.yyyy)).y;
    // 183: mad r7.xyz, r3.wwww, r7.xyzx, -r5.yzwy
    r7.xyz = ((r3.wwww)*(r7.xyzx)+(-(r5.yzwy))).xyz;
    // 184: mad r5.yzw, r2.yyyy, r7.xxyz, r5.yyzw
    r5.yzw = ((r2.yyyy)*(r7.xxyz)+(r5.yyzw)).yzw;
    // 185: sqrt r2.y, r5.x
    r2.y = (sqrt(r5.xxxx)).y;
    // 186: mul r3.xyz, r3.xyzx, r2.yyyy
    r3.xyz = ((r3.xyzx)*(r2.yyyy)).xyz;
    // 187: mul r3.xyz, r3.xyzx, r5.yzwy
    r3.xyz = ((r3.xyzx)*(r5.yzwy)).xyz;
    // 188: mad r5.yzw, -cb0[17].xxxx, r11.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r5.yzw = ((-(source[17].xxxx))*(r11.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 189: mad r5.yzw, r3.xxyz, r5.yyzw, r12.xxyz
    r5.yzw = ((r3.xxyz)*(r5.yyzw)+(r12.xxyz)).yzw;
    // 190: mul r7.xyz, cb0[2].xyzx, cb0[2].wwww
    r7.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 191: mad r11.xyz, cb0[3].wwww, cb0[3].xyzx, -r7.xyzx
    r11.xyz = ((source[3].wwww)*(source[3].xyzx)+(-(r7.xyzx))).xyz;
    // 192: mad r7.xyz, r8.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r8.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 193: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 194: mad r7.xyz, r8.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r8.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 195: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 196: mad r7.xyz, r1.wwww, r11.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 197: add r11.xyz, -r7.xyzx, cb0[6].xyzx
    r11.xyz = ((-(r7.xyzx))+(source[6].xyzx)).xyz;
    // 198: mad r7.xyz, r8.wwww, r11.xyzx, r7.xyzx
    r7.xyz = ((r8.wwww)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 199: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 200: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 201: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 202: mad r11.xyz, r8.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r8.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 203: mul r12.xyz, r7.xyzx, r11.xyzx
    r12.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 204: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 205: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 206: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 207: add r1.xyz, -cb0[7].xyzx, cb0[8].xyzx
    r1.xyz = ((-(source[7].xyzx))+(source[8].xyzx)).xyz;
    // 208: mad r1.xyz, r0.yyyy, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(source[7].xyzx)).xyz;
    // 209: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 210: mul r0.xyz, r0.xyzx, cb0[15].yyyy
    r0.xyz = ((r0.xyzx)*(source[15].yyyy)).xyz;
    // 211: mul r1.xyz, r0.xyzx, r12.xyzx
    r1.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 212: dp3 r2.y, r9.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r9.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 213: add r13.xyz, -r9.yzwy, r2.yyyy
    r13.xyz = ((-(r9.yzwy))+(r2.yyyy)).xyz;
    // 214: mul r13.xyz, r13.xyzx, cb0[15].zzzz
    r13.xyz = ((r13.xyzx)*(source[15].zzzz)).xyz;
    // 215: add r2.y, r8.y, r8.x
    r2.y = ((r8.yyyy)+(r8.xxxx)).y;
    // 216: add r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)+(r2.yyyy)).w;
    // 217: add_sat r1.w, r8.w, r1.w
    r1.w = (saturate((r8.wwww)+(r1.wwww))).w;
    // 218: mad r8.xyz, r1.wwww, r13.xyzx, r9.yzwy
    r8.xyz = ((r1.wwww)*(r13.xyzx)+(r9.yzwy)).xyz;
    // 219: add r9.xyz, r2.xxxx, -r8.xyzx
    r9.xyz = ((r2.xxxx)+(-(r8.xyzx))).xyz;
    // 220: mad r8.xyz, r8.wwww, r9.xyzx, r8.xyzx
    r8.xyz = ((r8.wwww)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 221: max r9.xyz, |r8.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r8.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 222: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 223: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 224: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 225: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 226: add r2.x, cb0[15].w, -cb0[16].x
    r2.x = ((source[15].wwww)+(-(source[16].xxxx))).x;
    // 227: mad r2.x, r8.w, r2.x, cb0[16].x
    r2.x = ((r8.wwww)*(r2.xxxx)+(source[16].xxxx)).x;
    // 228: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 229: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 230: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 231: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 232: mad r2.x, -r1.w, r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 233: max r2.x, r2.x, l(0.001000)
    r2.x = (max(r2.xxxx,float4(0.001000,0.001000,0.001000,0.001000))).x;
    // 234: div r2.x, cb0[16].y, r2.x
    r2.x = ((source[16].yyyy)/(r2.xxxx)).x;
    // 235: mul r2.x, r2.x, r4.z
    r2.x = ((r2.xxxx)*(r4.zzzz)).x;
    // 236: mul r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 237: add r2.x, -r1.w, l(1.000000)
    r2.x = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 238: mul r2.x, r2.x, cb0[16].z
    r2.x = ((r2.xxxx)*(source[16].zzzz)).x;
    // 239: mad r0.xyz, r8.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 240: mad r0.xyz, r2.xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 241: mad r0.xyz, r2.zzzz, r0.xyzx, -r12.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 242: mad r0.xyz, r1.wwww, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 243: mul r1.x, cb0[1].z, l(1.500000)
    r1.x = ((source[1].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 244: add r1.y, -cb0[1].w, l(1.000000)
    r1.y = ((-(source[1].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 245: mul r1.y, r1.y, cb0[12].x
    r1.y = ((r1.yyyy)*(source[12].xxxx)).y;
    // 246: mul r1.y, r1.y, l(6.283185)
    r1.y = ((r1.yyyy)*(float4(6.283185,6.283185,6.283185,6.283185))).y;
    // 247: sincos r1.y, null, r1.y
    r1.y = (sin(r1.yyyy)).y;
    // 248: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 249: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 250: mad r1.x, r1.x, l(0.500000), cb0[1].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[1].zzzz)).x;
    // 251: frc r1.y, cb0[1].x
    r1.y = (frac(source[1].xxxx)).y;
    // 252: add r1.z, -r1.y, cb0[1].x
    r1.z = ((-(r1.yyyy))+(source[1].xxxx)).z;
    // 253: mul r9.z, r1.z, l(0.125000)
    r9.z = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 254: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 255: mul r9.y, cb0[1].y, cb0[9].y
    r9.y = ((source[1].yyyy)*(source[9].yyyy)).y;
    // 256: frc r1.z, v4.x
    r1.z = (frac(v4.xxxx)).z;
    // 257: mul r2.x, r1.z, l(0.125000)
    r2.x = ((r1.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 258: mov r2.y, v4.y
    r2.y = (v4.yyyy).y;
    // 259: add r2.xy, r2.xyxx, r9.xyxx
    r2.xy = ((r2.xyxx)+(r9.xyxx)).xy;
    // 260: add r2.xy, r2.xyxx, r9.zwzz
    r2.xy = ((r2.xyxx)+(r9.zwzz)).xy;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r2.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 262: mul r4.xyz, r1.xxxx, r9.xyzx
    r4.xyz = ((r1.xxxx)*(r9.xyzx)).xyz;
    // 263: mul r1.x, r1.y, r9.w
    r1.x = ((r1.yyyy)*(r9.wwww)).x;
    // 264: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 265: mad r0.xyz, r1.xxxx, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 266: max r1.xyz, r3.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = (max(r3.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 267: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 268: mad r3.xyz, r7.xyzx, r11.xyzx, -r8.xyzx
    r3.xyz = ((r7.xyzx)*(r11.xyzx)+(-(r8.xyzx))).xyz;
    // 269: mad r3.xyz, r1.wwww, r3.xyzx, r8.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r8.xyzx)).xyz;
    // 270: dp3 r4.x, r10.xyzx, r6.xyzx
    r4.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 271: mul r4.z, r0.w, cb0[17].w
    r4.z = ((r0.wwww)*(source[17].wwww)).z;
    // 272: mul r0.w, r8.w, r4.z
    r0.w = ((r8.wwww)*(r4.zzzz)).w;
    // 273: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 274: min r0.w, r0.w, cb0[17].w
    r0.w = (min(r0.wwww,source[17].wwww)).w;
    // 275: add r1.w, -cb0[18].z, cb0[18].y
    r1.w = ((-(source[18].zzzz))+(source[18].yyyy)).w;
    // 276: mad r1.w, cb0[18].x, r1.w, cb0[18].z
    r1.w = ((source[18].xxxx)*(r1.wwww)+(source[18].zzzz)).w;
    // 277: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 278: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 279: mad r1.w, r8.w, r1.w, l(1.000000)
    r1.w = ((r8.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 280: lt r2.x, |r0.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 281: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 282: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 283: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 284: movc r4.y, r2.x, l(0), r0.w
    r4.y = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 285: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 286: add r0.w, -cb0[19].y, cb0[19].x
    r0.w = ((-(source[19].yyyy))+(source[19].xxxx)).w;
    // 287: mad r0.w, cb0[18].w, r0.w, cb0[19].y
    r0.w = ((source[18].wwww)*(r0.wwww)+(source[19].yyyy)).w;
    // 288: mul r0.w, r0.w, r8.w
    r0.w = ((r0.wwww)*(r8.wwww)).w;
    // 289: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 290: mad r4.xyz, r0.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 291: add r0.w, -cb0[19].z, l(2.000000)
    r0.w = ((-(source[19].zzzz))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 292: mad r0.w, r2.z, r0.w, cb0[19].z
    r0.w = ((r2.zzzz)*(r0.wwww)+(source[19].zzzz)).w;
    // 293: mul r2.xyz, r4.xyzx, r0.wwww
    r2.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 294: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 295: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 296: mul r2.xyz, r2.xyzx, cb0[19].wwww
    r2.xyz = ((r2.xyzx)*(source[19].wwww)).xyz;
    // 297: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 298: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 299: mul r2.xyz, r2.xyzx, r5.xxxx
    r2.xyz = ((r2.xyzx)*(r5.xxxx)).xyz;
    // 300: min r2.xyz, r2.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 301: mul r2.xyz, r2.xyzx, cb0[20].xxxx
    r2.xyz = ((r2.xyzx)*(source[20].xxxx)).xyz;
    // 302: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 303: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 304: mad r0.xyz, r5.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r5.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 305: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 306: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 307: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 308: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 309: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 310: ret
    return output;
}

// source.character.mokoko-av036-914.v1 / source program 59ebc3be033e8447a6bee04e6edb0a66
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight914(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[20].y=(g_SourceCharacterTime.xxxx).x;
    source[20].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[20].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[21].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[21].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[24]=float4(input.lightColor,1.0);
    source[25].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[25].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[25].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[15].xxxx
    r9.xy = ((r8.xyxx)*(source[15].xxxx)).xy;
    // 33: dp2 r1.w, r8.xyxx, r8.xyxx
    r1.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 34: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: add r9.z, r1.w, l(0.000010)
    r9.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: add r8.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r8.xyz, cb0[15].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[15].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 40: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 43: dp3 r10.x, r1.xyzx, r8.xyzx
    r10.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 44: dp3 r10.y, r2.xyzx, r8.xyzx
    r10.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 45: dp3 r10.z, r0.xyzx, r8.xyzx
    r10.z = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 46: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 47: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 48: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 49: dp3 r0.x, r10.xyzx, r1.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: mul r0.xyz, r10.xyzx, r0.xxxx
    r0.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 51: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 52: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 53: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 54: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 55: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 56: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 57: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: add r0.z, -cb0[16].y, cb0[16].x
    r0.z = ((-(source[16].yyyy))+(source[16].xxxx)).z;
    // 60: mad r0.z, r2.x, r0.z, cb0[16].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[16].yyyy)).z;
    // 61: add r1.x, -r0.z, cb0[16].z
    r1.x = ((-(r0.zzzz))+(source[16].zzzz)).x;
    // 62: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 63: add r1.x, -r0.z, cb0[16].w
    r1.x = ((-(r0.zzzz))+(source[16].wwww)).x;
    // 64: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 66: add r1.x, -r8.w, l(1.000000)
    r1.x = ((-(r8.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: add r1.y, -cb0[17].y, cb0[17].x
    r1.y = ((-(source[17].yyyy))+(source[17].xxxx)).y;
    // 68: mad r1.y, r2.x, r1.y, cb0[17].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[17].yyyy)).y;
    // 69: add r1.w, -r1.y, cb0[17].z
    r1.w = ((-(r1.yyyy))+(source[17].zzzz)).w;
    // 70: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 71: add r1.w, -r1.y, cb0[17].w
    r1.w = ((-(r1.yyyy))+(source[17].wwww)).w;
    // 72: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 73: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 74: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 75: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 76: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 77: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 78: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 79: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 80: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 81: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 82: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 83: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 84: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 85: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 86: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 87: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 88: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 89: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 90: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 91: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 92: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 93: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 94: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 95: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 97: rcp r1.y, cb0[18].x
    r1.y = (1.0/(source[18].xxxx)).y;
    // 98: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 99: mul r10.xyz, r4.xyzx, cb0[18].xxxx
    r10.xyz = ((r4.xyzx)*(source[18].xxxx)).xyz;
    // 100: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 101: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r10.xyzx, cb0[18].xxxx, r4.xyzx
    r4.xyz = ((r10.xyzx)*(source[18].xxxx)+(r4.xyzx)).xyz;
    // 105: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 106: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 107: add r1.y, cb0[18].x, l(1.000000)
    r1.y = ((source[18].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 109: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 110: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 111: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 112: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 113: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 114: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 115: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 117: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 118: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 119: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 120: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 121: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 122: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 123: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 124: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 125: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 127: mul r10.xyz, r0.xyzx, r1.yyyy
    r10.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 128: mul r11.xyz, r10.xyzx, cb0[22].xxxx
    r11.xyz = ((r10.xyzx)*(source[22].xxxx)).xyz;
    // 129: dp3 r4.w, r9.xyzx, r9.xyzx
    r4.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 130: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 131: div r9.xyz, r9.xyzx, r4.wwww
    r9.xyz = ((r9.xyzx)/(r4.wwww)).xyz;
    // 132: dp3 r4.w, r9.xyzx, r7.xyzx
    r4.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 133: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 134: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 137: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 138: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 140: mul_sat r6.xy, r6.xzxx, cb0[18].wwww
    r6.xy = (saturate((r6.xzxx)*(source[18].wwww))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[19].x
    r6.y = (saturate((r6.yyyy)+(-(source[19].xxxx)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[19].y
    r6.y = ((r6.yyyy)*(source[19].yyyy)).y;
    // 146: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 147: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 148: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 149: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 151: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 152: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 153: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 154: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 155: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 156: mul r1.x, r1.x, cb0[22].z
    r1.x = ((r1.xxxx)*(source[22].zzzz)).x;
    // 157: mad r6.y, cb0[22].y, r6.y, -r4.w
    r6.y = ((source[22].yyyy)*(r6.yyyy)+(-(r4.wwww))).y;
    // 158: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 159: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 160: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 161: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 162: mad r7.xyz, -cb0[22].xxxx, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[22].xxxx))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 163: mad r7.xyz, r5.xyzx, r7.xyzx, r11.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 164: mul r10.xyz, cb0[3].xyzx, cb0[3].wwww
    r10.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r10.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r10.xyzx))).xyz;
    // 166: mad r10.xyz, r2.xxxx, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.xxxx)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 167: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r10.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r10.xyzx))).xyz;
    // 168: mad r10.xyz, r2.yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 169: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, -r10.xyzx
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r10.xyzx))).xyz;
    // 170: mad r10.xyz, r2.zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((r2.zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 171: dp3 r1.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 172: add r11.xyz, -r10.xyzx, r1.xxxx
    r11.xyz = ((-(r10.xyzx))+(r1.xxxx)).xyz;
    // 173: mad r10.xyz, cb0[15].yyyy, r11.xyzx, r10.xyzx
    r10.xyz = ((source[15].yyyy)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 174: dp3 r1.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 175: add r11.xyz, -r10.xyzx, r1.xxxx
    r11.xyz = ((-(r10.xyzx))+(r1.xxxx)).xyz;
    // 176: mad r10.xyz, cb0[15].zzzz, r11.xyzx, r10.xyzx
    r10.xyz = ((source[15].zzzz)*(r11.xyzx)+(r10.xyzx)).xyz;
    // 177: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 180: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 181: sample_b_indexable(texture2d)(float,float,float,float) r12.xyzw, v4.xyxx, t2.wxyz, s3, l(0.000000)
    r12.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 182: dp3 r1.x, r12.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r12.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 183: add r13.xyz, -r12.yzwy, r1.xxxx
    r13.xyz = ((-(r12.yzwy))+(r1.xxxx)).xyz;
    // 184: mad r12.yzw, cb0[15].yyyy, r13.xxyz, r12.yyzw
    r12.yzw = ((source[15].yyyy)*(r13.xxyz)+(r12.yyzw)).yzw;
    // 185: dp3 r1.x, r12.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r12.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 186: add r13.xyz, -r12.yzwy, r1.xxxx
    r13.xyz = ((-(r12.yzwy))+(r1.xxxx)).xyz;
    // 187: mad r12.yzw, cb0[15].zzzz, r13.xxyz, r12.yyzw
    r12.yzw = ((source[15].zzzz)*(r13.xxyz)+(r12.yyzw)).yzw;
    // 188: mul r13.xyz, r10.xyzx, r12.yzwy
    r13.xyz = ((r10.xyzx)*(r12.yzwy)).xyz;
    // 189: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 191: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 192: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 193: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 194: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 195: mul r0.xyz, r0.xyzx, cb0[18].yyyy
    r0.xyz = ((r0.xyzx)*(source[18].yyyy)).xyz;
    // 196: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 197: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 199: mad r8.xyz, cb0[15].yyyy, r15.xyzx, r8.xyzx
    r8.xyz = ((source[15].yyyy)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 200: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 201: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 202: mad r8.xyz, cb0[15].zzzz, r15.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 203: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 204: add r15.xyz, -r8.xyzx, r1.xxxx
    r15.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 205: mul r15.xyz, r15.xyzx, cb0[18].zzzz
    r15.xyz = ((r15.xyzx)*(source[18].zzzz)).xyz;
    // 206: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 207: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 208: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 209: mad r2.xyz, r1.xxxx, r15.xyzx, r8.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r8.xyzx)).xyz;
    // 210: max r8.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r8.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 211: log r8.xyz, r8.xyzx
    r8.xyz = (log2(r8.xyzx)).xyz;
    // 212: mul r8.xyz, r8.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 213: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 214: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 215: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 216: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 217: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 218: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 219: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 220: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 221: div r1.z, cb0[19].w, r1.z
    r1.z = ((source[19].wwww)/(r1.zzzz)).z;
    // 222: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 223: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 224: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r1.z, r1.z, cb0[20].x
    r1.z = ((r1.zzzz)*(source[20].xxxx)).z;
    // 226: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 227: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 228: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 229: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 230: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 231: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 232: mad r0.xyz, cb0[15].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 233: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 234: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 235: mad r0.xyz, cb0[15].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[15].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 236: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 237: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 238: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 239: mul r2.w, r2.w, cb0[20].y
    r2.w = ((r2.wwww)*(source[20].yyyy)).w;
    // 240: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 241: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 242: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 244: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 245: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 246: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 247: mul r8.z, r4.w, l(0.125000)
    r8.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 248: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 249: mul r8.y, cb0[2].y, cb0[11].y
    r8.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 250: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 251: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 252: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 253: add r6.xy, r6.xyxx, r8.xyxx
    r6.xy = ((r6.xyxx)+(r8.xyxx)).xy;
    // 254: add r6.xy, r6.xyxx, r8.zwzz
    r6.xy = ((r6.xyxx)+(r8.zwzz)).xy;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r6.xyxx, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 256: mul r6.xyz, r1.zzzz, r8.xyzx
    r6.xyz = ((r1.zzzz)*(r8.xyzx)).xyz;
    // 257: mul r1.z, r2.w, r8.w
    r1.z = ((r2.wwww)*(r8.wwww)).z;
    // 258: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 259: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 260: mul r1.z, cb0[12].y, cb0[20].y
    r1.z = ((source[12].yyyy)*(source[20].yyyy)).z;
    // 261: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 262: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 263: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 264: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 265: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 266: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 267: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 268: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 269: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 270: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 271: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 272: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 273: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 274: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 275: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 276: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 277: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 278: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 279: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 280: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 281: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 282: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 283: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 284: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 285: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 286: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 287: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 288: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 289: mad r5.xyz, r10.xyzx, r12.yzwy, -r2.xyzx
    r5.xyz = ((r10.xyzx)*(r12.yzwy)+(-(r2.xyzx))).xyz;
    // 290: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 291: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 292: mul r4.y, r0.w, cb0[22].w
    r4.y = ((r0.wwww)*(source[22].wwww)).y;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s7, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 294: add r0.w, -cb0[23].x, l(2.000000)
    r0.w = ((-(source[23].xxxx))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 295: mad r0.w, r1.y, r0.w, cb0[23].x
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[23].xxxx)).w;
    // 296: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 297: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 298: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 299: mul r1.xyz, r1.xyzx, cb0[23].yyyy
    r1.xyz = ((r1.xyzx)*(source[23].yyyy)).xyz;
    // 300: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 301: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 302: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 303: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 304: mul r1.xyz, r1.xyzx, cb0[23].zzzz
    r1.xyz = ((r1.xyzx)*(source[23].zzzz)).xyz;
    // 305: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 306: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 307: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 308: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 309: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 310: mul o0.xyz, r0.xyzx, cb0[24].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[24].xyzx)).xyz;
    // 311: mov_sat r12.x, r12.x
    r12.x = (saturate(r12.xxxx)).x;
    // 312: mul_sat r0.x, r12.x, cb0[21].z
    r0.x = (saturate((r12.xxxx)*(source[21].zzzz))).x;
    // 313: mul o0.w, r0.x, cb0[1].w
    output.targets[0].w = ((r0.xxxx)*(source[1].wwww)).w;
    // 314: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 315: ret
    return output;
}

// source.character.mokoko-av036-915.v1 / source program 338091daad420b468fe726ae042b88f1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight915(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[17].w=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t7.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r8.xy, r8.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r8.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r9.xy, r8.xyxx, cb0[14].xxxx
    r9.xy = ((r8.xyxx)*(source[14].xxxx)).xy;
    // 33: dp2 r1.w, r8.xyxx, r8.xyxx
    r1.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 34: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: add r9.z, r1.w, l(0.000010)
    r9.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: add r8.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r8.xyz, cb0[14].wwww, r8.xyzx, r9.xyzx
    r8.xyz = ((source[14].wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 40: dp3 r1.w, r8.xyzx, r8.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 41: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 42: div r8.xyz, r8.xyzx, r1.wwww
    r8.xyz = ((r8.xyzx)/(r1.wwww)).xyz;
    // 43: dp3 r10.x, r1.xyzx, r8.xyzx
    r10.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 44: dp3 r10.y, r2.xyzx, r8.xyzx
    r10.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 45: dp3 r10.z, r0.xyzx, r8.xyzx
    r10.z = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 46: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 47: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 48: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 49: dp3 r0.x, r10.xyzx, r1.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: mul r0.xyz, r10.xyzx, r0.xxxx
    r0.xyz = ((r10.xyzx)*(r0.xxxx)).xyz;
    // 51: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 52: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 53: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 54: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 55: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 56: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 57: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 61: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 62: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 63: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 64: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 66: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 67: mul r1.x, r1.x, cb0[15].y
    r1.x = ((r1.xxxx)*(source[15].yyyy)).x;
    // 68: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 69: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 70: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 71: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 72: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 73: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 74: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 75: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 76: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 77: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 78: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 79: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 80: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 81: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 82: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 83: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 84: rcp r0.x, cb0[15].z
    r0.x = (1.0/(source[15].zzzz)).x;
    // 85: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 86: mul r8.xyz, r4.xyzx, cb0[15].zzzz
    r8.xyz = ((r4.xyzx)*(source[15].zzzz)).xyz;
    // 87: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 88: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 89: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 90: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 91: mad r4.xyz, r8.xyzx, cb0[15].zzzz, r4.xyzx
    r4.xyz = ((r8.xyzx)*(source[15].zzzz)+(r4.xyzx)).xyz;
    // 92: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 93: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 94: add r0.x, cb0[15].z, l(1.000000)
    r0.x = ((source[15].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 96: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 97: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 98: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 99: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 100: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 101: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 102: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 103: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 104: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 105: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 106: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 107: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 108: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 109: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 110: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 111: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 112: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 114: mul r8.xyz, r1.xywx, r4.xxxx
    r8.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 115: mul r10.xyz, r8.xyzx, cb0[18].yyyy
    r10.xyz = ((r8.xyzx)*(source[18].yyyy)).xyz;
    // 116: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 117: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 118: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 119: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 120: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 121: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 123: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 124: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 125: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 127: mul_sat r6.xy, r6.xzxx, cb0[16].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[16].yyyy))).xy;
    // 128: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 129: add_sat r6.y, r6.y, -cb0[16].z
    r6.y = (saturate((r6.yyyy)+(-(source[16].zzzz)))).y;
    // 130: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 131: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 132: mul r6.y, r6.y, cb0[16].w
    r6.y = ((r6.yyyy)*(source[16].wwww)).y;
    // 133: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 134: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 135: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 136: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 137: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 138: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 139: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 140: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 142: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 143: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 144: mad r6.y, cb0[18].z, r6.y, -r4.z
    r6.y = ((source[18].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 145: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 146: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 147: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 148: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 149: mad r6.yzw, -cb0[18].yyyy, r8.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[18].yyyy))*(r8.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 150: mad r6.yzw, r5.xxyz, r6.yyzw, r10.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r10.xxyz)).yzw;
    // 151: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 152: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 153: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 154: mad r7.xyz, r8.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r8.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 155: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 156: mad r7.xyz, r8.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r8.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 157: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 158: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 159: mad r7.xyz, cb0[14].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[14].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 160: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 161: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 162: mad r7.xyz, cb0[14].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[14].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 163: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mad r11.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 166: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 167: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 168: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 169: add r12.xyz, -r11.xyzx, r0.zzzz
    r12.xyz = ((-(r11.xyzx))+(r0.zzzz)).xyz;
    // 170: mad r11.xyz, cb0[14].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[14].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 171: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 172: add r12.xyz, -r11.xyzx, r0.zzzz
    r12.xyz = ((-(r11.xyzx))+(r0.zzzz)).xyz;
    // 173: mad r11.xyz, cb0[14].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[14].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 174: mul r12.xyz, r7.xyzx, r11.xyzx
    r12.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 175: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 177: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 178: add r1.yzw, -cb0[8].xxyz, cb0[9].xxyz
    r1.yzw = ((-(source[8].xxyz))+(source[9].xxyz)).yzw;
    // 179: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[8].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[8].xyzx)).xyz;
    // 180: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 181: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 182: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 183: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r2.xyz, cb0[14].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[14].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 186: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 187: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 188: mad r2.xyz, cb0[14].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[14].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 189: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 191: mul r14.xyz, r14.xyzx, cb0[16].xxxx
    r14.xyz = ((r14.xyzx)*(source[16].xxxx)).xyz;
    // 192: add r0.z, r8.y, r8.x
    r0.z = ((r8.yyyy)+(r8.xxxx)).z;
    // 193: add r0.z, r8.z, r0.z
    r0.z = ((r8.zzzz)+(r0.zzzz)).z;
    // 194: add_sat r0.z, r8.w, r0.z
    r0.z = (saturate((r8.wwww)+(r0.zzzz))).z;
    // 195: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 196: max r8.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r8.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 197: log r8.xyz, r8.xyzx
    r8.xyz = (log2(r8.xyzx)).xyz;
    // 198: mul r8.xyz, r8.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r8.xyz = ((r8.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 199: exp r8.xyz, r8.xyzx
    r8.xyz = (exp2(r8.xyzx)).xyz;
    // 200: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 201: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 202: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 203: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 204: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 205: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 206: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 207: div r1.w, cb0[17].y, r1.w
    r1.w = ((source[17].yyyy)/(r1.wwww)).w;
    // 208: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 209: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 210: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 211: mul r1.w, r1.w, cb0[17].z
    r1.w = ((r1.wwww)*(source[17].zzzz)).w;
    // 212: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 213: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 214: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 215: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 216: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 217: add r8.xyz, -r1.xyzx, r1.wwww
    r8.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 218: mad r1.xyz, cb0[14].yyyy, r8.xyzx, r1.xyzx
    r1.xyz = ((source[14].yyyy)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 219: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 220: add r8.xyz, -r1.xyzx, r1.wwww
    r8.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 221: mad r1.xyz, cb0[14].zzzz, r8.xyzx, r1.xyzx
    r1.xyz = ((source[14].zzzz)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 222: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 223: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 224: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 225: mul r4.z, r4.z, cb0[17].w
    r4.z = ((r4.zzzz)*(source[17].wwww)).z;
    // 226: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 227: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 228: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 230: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 231: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 232: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 233: mul r8.z, r6.x, l(0.125000)
    r8.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 234: mov r8.xw, l(0,0,0,0)
    r8.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 235: mul r8.y, cb0[2].y, cb0[10].y
    r8.y = ((source[2].yyyy)*(source[10].yyyy)).y;
    // 236: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 237: mul r10.x, r6.x, l(0.125000)
    r10.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 238: mov r10.y, v4.y
    r10.y = (v4.yyyy).y;
    // 239: add r8.xy, r8.xyxx, r10.xyxx
    r8.xy = ((r8.xyxx)+(r10.xyxx)).xy;
    // 240: add r8.xy, r8.xyxx, r8.zwzz
    r8.xy = ((r8.xyxx)+(r8.zwzz)).xy;
    // 241: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t5.xyzw, s6, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 242: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 243: mul r1.w, r4.z, r8.w
    r1.w = ((r4.zzzz)*(r8.wwww)).w;
    // 244: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 245: mad r1.xyz, r1.wwww, r8.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 246: mul r1.w, cb0[11].y, cb0[17].w
    r1.w = ((source[11].yyyy)*(source[17].wwww)).w;
    // 247: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 248: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 249: mul r8.y, r1.w, l(0.020000)
    r8.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 250: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 251: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 252: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 253: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 254: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 255: mov r8.x, l(0)
    r8.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 256: mad r3.xy, r3.zzzz, r3.xyxx, r8.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r8.xyxx)).xy;
    // 257: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 258: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 259: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 260: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 261: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 262: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 263: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 264: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 265: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 266: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 267: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 268: mul r8.xyz, r3.xyzx, cb0[11].zzzz
    r8.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 269: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 270: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 271: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 272: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 273: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 274: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 275: mad r5.xyz, r7.xyzx, r11.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r11.xyzx)+(-(r2.xyzx))).xyz;
    // 276: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 277: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 278: mul r0.y, r2.w, cb0[19].x
    r0.y = ((r2.wwww)*(source[19].xxxx)).y;
    // 279: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 280: add r0.w, -cb0[19].y, l(2.000000)
    r0.w = ((-(source[19].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 281: mad r0.w, r4.x, r0.w, cb0[19].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[19].yyyy)).w;
    // 282: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 283: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 284: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 285: mul r0.xyz, r0.xyzx, cb0[19].zzzz
    r0.xyz = ((r0.xyzx)*(source[19].zzzz)).xyz;
    // 286: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 287: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 288: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 289: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 290: mul r0.xyz, r0.xyzx, cb0[19].wwww
    r0.xyz = ((r0.xyzx)*(source[19].wwww)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 293: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 294: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 295: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 296: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 297: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 298: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 299: ret
    return output;
}

// source.character.static-map-native-1100.v1 / source program 86b03a7da39f07478f200df2b8333c5e
#else // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
    case 901u: return SourceCharacterLight901(input);
    case 902u: return SourceCharacterLight902(input);
    case 903u: return SourceCharacterLight903(input);
    case 904u: return SourceCharacterLight904(input);
    case 905u: return SourceCharacterLight905(input);
    case 906u: return SourceCharacterLight906(input);
    case 907u: return SourceCharacterLight907(input);
    case 908u: return SourceCharacterLight908(input);
    case 909u: return SourceCharacterLight909(input);
    case 910u: return SourceCharacterLight910(input);
    case 911u: return SourceCharacterLight911(input);
    case 912u: return SourceCharacterLight912(input);
    case 913u: return SourceCharacterLight913(input);
    case 914u: return SourceCharacterLight914(input);
    case 915u: return SourceCharacterLight915(input);
#endif // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
