SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight235(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[16].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[16].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[16].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
    // Original engine primitive opacity/environment rows.
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r4.xy, v4.xyxx, cb0[18].xxxx
    r4.xy = ((v4.xyxx)*(source[18].xxxx)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xyxx, t2.yzwx, s6, l(0.000000)
    r1.w = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 24: add r1.w, r1.w, -cb0[18].y
    r1.w = ((r1.wwww)+(-(source[18].yyyy))).w;
    // 25: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 26: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 27: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 28: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 29: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 30: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
    // 31: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 32: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 33: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 34: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 35: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 36: else
    } else {
    // 37: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 38: endif
    }
    // 39: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 42: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 43: mul r1.w, r6.x, cb0[16].w
    r1.w = ((r6.xxxx)*(source[16].wwww)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 46: add_sat r1.w, r1.w, cb0[17].x
    r1.w = (saturate((r1.wwww)+(source[17].xxxx))).w;
    // 47: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 49: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 50: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 51: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 53: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 54: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 55: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 56: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 57: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 58: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 59: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 60: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 61: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 62: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 63: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 64: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 65: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 66: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 69: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 70: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 71: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 72: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 73: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 74: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 75: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 76: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 77: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 78: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 79: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 80: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 81: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 82: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 83: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 84: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 85: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 86: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 87: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 88: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 90: mad r6.xyw, cb0[15].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 91: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 93: mad r6.xyw, cb0[15].zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 94: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 95: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 96: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 97: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 98: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 100: mad r3.xyz, cb0[15].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 101: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[15].zzzz, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].zzzz)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 104: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 105: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 107: mad r3.xyz, cb0[15].yyyy, r3.xyzx, r10.xyzx
    r3.xyz = ((source[15].yyyy)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 108: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 110: mad r3.xyz, cb0[15].zzzz, r6.xywx, r3.xyzx
    r3.xyz = ((source[15].zzzz)*(r6.xywx)+(r3.xyzx)).xyz;
    // 111: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 112: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 113: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul r4.w, r4.w, cb0[14].z
    r4.w = ((r4.wwww)*(source[14].zzzz)).w;
    // 115: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 116: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 117: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 119: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 120: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 121: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 122: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 123: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 124: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 125: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 126: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 127: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 128: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 129: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 131: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 132: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 133: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 134: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 135: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 136: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 137: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 138: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 139: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 140: mov_sat r1.w, cb0[17].w
    r1.w = (saturate(source[17].wwww)).w;
    // 141: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 142: add r3.w, -cb0[19].y, cb0[19].x
    r3.w = ((-(source[19].yyyy))+(source[19].xxxx)).w;
    // 143: mad r3.w, r9.x, r3.w, cb0[19].y
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[19].yyyy)).w;
    // 144: add r4.w, -r3.w, cb0[19].w
    r4.w = ((-(r3.wwww))+(source[19].wwww)).w;
    // 145: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 146: add r4.w, -r3.w, cb0[20].y
    r4.w = ((-(r3.wwww))+(source[20].yyyy)).w;
    // 147: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 148: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 149: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 150: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 152: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 153: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 155: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 156: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 157: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 158: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 159: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 160: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 161: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 163: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 164: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 167: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 168: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 170: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 171: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 172: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 173: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 174: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 176: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 177: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 178: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 179: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 180: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 181: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 182: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 183: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 184: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 185: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 186: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 187: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 188: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 190: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 191: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 192: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 193: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 194: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 196: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 197: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 198: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 199: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 201: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 202: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 203: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 204: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 205: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 206: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 207: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 208: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 209: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 210: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 211: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 212: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 213: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 214: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 215: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 216: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 217: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 218: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 219: mul r0.x, r0.x, cb0[20].z
    r0.x = ((r0.xxxx)*(source[20].zzzz)).x;
    // 220: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 221: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 222: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 224: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 225: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 226: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 227: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 228: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 229: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 230: ret
    return output;
}

// source.character.selection-native-236.v1 / source program 9e3d5653fdf4f648b9dde6eee0f9a145
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight236(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[16].z=(g_SourceCharacterTime.xxxx).x;
    source[17].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
    // Original engine primitive opacity/environment rows.
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r4.xy, v4.xyxx, cb0[18].zzzz
    r4.xy = ((v4.xyxx)*(source[18].zzzz)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xyxx, t2.yzwx, s6, l(0.000000)
    r1.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 24: add r1.w, r1.w, -cb0[18].w
    r1.w = ((r1.wwww)+(-(source[18].wwww))).w;
    // 25: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 26: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 27: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 28: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 29: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 30: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
    // 31: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 32: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 33: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 34: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 35: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 36: else
    } else {
    // 37: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 38: endif
    }
    // 39: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 42: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 43: mul r1.w, r6.x, cb0[17].y
    r1.w = ((r6.xxxx)*(source[17].yyyy)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 46: add_sat r1.w, r1.w, cb0[17].z
    r1.w = (saturate((r1.wwww)+(source[17].zzzz))).w;
    // 47: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 49: mul r2.w, r6.y, cb0[13].y
    r2.w = ((r6.yyyy)*(source[13].yyyy)).w;
    // 50: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 51: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 53: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 54: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 55: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 56: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 57: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 58: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 59: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 60: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 61: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 62: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 63: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 64: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 65: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 66: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 69: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 70: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 71: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 72: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 73: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 74: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 75: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 76: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 77: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 78: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 79: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 80: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 81: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 82: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 83: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 84: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 85: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 86: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 87: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 88: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 90: mad r6.xyw, cb0[14].wwww, r7.xyxw, r6.xyxw
    r6.xyw = ((source[14].wwww)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 91: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 93: mad r6.xyw, cb0[15].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[15].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 94: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 95: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 96: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 97: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 98: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 100: mad r3.xyz, cb0[14].wwww, r10.xyzx, r3.xyzx
    r3.xyz = ((source[14].wwww)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 101: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 102: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 103: mad r3.xyz, cb0[15].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[15].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 104: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 105: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 107: mad r3.xyz, cb0[14].wwww, r3.xyzx, r10.xyzx
    r3.xyz = ((source[14].wwww)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 108: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 110: mad r3.xyz, cb0[15].xxxx, r6.xywx, r3.xyzx
    r3.xyz = ((source[15].xxxx)*(r6.xywx)+(r3.xyzx)).xyz;
    // 111: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 112: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 113: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul r4.w, r4.w, cb0[16].z
    r4.w = ((r4.wwww)*(source[16].zzzz)).w;
    // 115: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 116: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 117: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 119: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 120: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 121: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 122: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 123: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 124: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 125: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 126: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 127: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 128: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 129: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 130: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 131: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 132: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 133: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 134: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 135: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 136: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 137: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 138: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 139: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 140: mov_sat r1.w, cb0[18].y
    r1.w = (saturate(source[18].yyyy)).w;
    // 141: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 142: add r3.w, -cb0[19].w, cb0[19].z
    r3.w = ((-(source[19].wwww))+(source[19].zzzz)).w;
    // 143: mad r3.w, r9.x, r3.w, cb0[19].w
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[19].wwww)).w;
    // 144: add r4.w, -r3.w, cb0[20].y
    r4.w = ((-(r3.wwww))+(source[20].yyyy)).w;
    // 145: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 146: add r4.w, -r3.w, cb0[20].w
    r4.w = ((-(r3.wwww))+(source[20].wwww)).w;
    // 147: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 148: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 149: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 150: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 152: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 153: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 155: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 156: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 157: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 158: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 159: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 160: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 161: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 163: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 164: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 167: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 168: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 170: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 171: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 172: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 173: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 174: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 176: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 177: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 178: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 179: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 180: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 181: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 182: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 183: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 184: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 185: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 186: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 187: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 188: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 190: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 191: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 192: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 193: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 194: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 195: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 196: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 197: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 198: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 199: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 200: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 201: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 202: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 203: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 204: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 205: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 206: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 207: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 208: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 209: mul_sat r6.xyz, cb0[12].xyzx, cb0[12].wwww
    r6.xyz = (saturate((source[12].xyzx)*(source[12].wwww))).xyz;
    // 210: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 211: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 212: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 213: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 214: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 215: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 216: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 217: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 218: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 219: mul r0.x, r0.x, cb0[21].x
    r0.x = ((r0.xxxx)*(source[21].xxxx)).x;
    // 220: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 221: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 222: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 224: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 225: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 226: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 227: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 228: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 229: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 230: ret
    return output;
}

