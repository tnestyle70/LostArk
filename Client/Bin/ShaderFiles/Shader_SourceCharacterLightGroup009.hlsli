SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight9(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[15].z=(g_SourceCharacterTime.xxxx).x;
    source[21]=float4(input.lightColor,1.0);
    source[22].x=1.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[14].xxxx
    r3.xy = ((r2.xyxx)*(source[14].xxxx)).xy;
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
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r4.xyzw, r3.xyzw, cb0[12].xyzw
    r4.xyzw = ((r3.xyzw)*(source[12].xyzw)).xyzw;
    // 23: add r4.xy, r4.ywyy, r4.xzxx
    r4.xy = ((r4.ywyy)+(r4.xzxx)).xy;
    // 24: add r1.w, r4.y, r4.x
    r1.w = ((r4.yyyy)+(r4.xxxx)).w;
    // 25: add r3.xy, r3.ywyy, r3.xzxx
    r3.xy = ((r3.ywyy)+(r3.xzxx)).xy;
    // 26: add r2.w, r3.y, r3.x
    r2.w = ((r3.yyyy)+(r3.xxxx)).w;
    // 27: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 28: mad_sat r1.w, r2.w, r1.w, l(1.000000)
    r1.w = (saturate((r2.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 31: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 32: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 33: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 34: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[22].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[22].xxxx)) * 0xffffffffu)).w;
    // 35: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 36: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 37: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 38: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t6.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 39: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 40: else
    } else {
    // 41: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 42: endif
    }
    // 43: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: lt r7.xyz, |r6.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r7.xyz = (asfloat((uint4)((abs(r6.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 46: log r6.xyz, |r6.xzyx|
    r6.xyz = (log2(abs(r6.xzyx))).xyz;
    // 47: mul r1.w, r6.x, cb0[17].y
    r1.w = ((r6.xxxx)*(source[17].yyyy)).w;
    // 48: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 49: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 50: add_sat r1.w, r1.w, cb0[17].z
    r1.w = (saturate((r1.wwww)+(source[17].zzzz))).w;
    // 51: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r8.xyz, r2.wwww, cb0[10].xyzx
    r8.xyz = ((r2.wwww)*(source[10].xyzx)).xyz;
    // 53: mul r2.w, r6.y, cb0[14].y
    r2.w = ((r6.yyyy)*(source[14].yyyy)).w;
    // 54: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 55: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: movc r2.w, r7.y, l(0), r2.w
    r2.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 57: mul r6.xyw, cb0[3].xyxz, cb0[3].wwww
    r6.xyw = ((source[3].xyxz)*(source[3].wwww)).xyw;
    // 58: max r7.xyw, r6.xyxw, l(0.002170, 0.002170, 0.000000, 0.002170)
    r7.xyw = (max(r6.xyxw,float4(0.002170,0.002170,0.000000,0.002170))).xyw;
    // 59: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 60: max r6.xyw, r6.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r6.xyw = (max(r6.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 61: min r6.xyw, r6.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r6.xyw = (min(r6.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 62: add r6.xyw, -r7.xyxw, r6.xyxw
    r6.xyw = ((-(r7.xyxw))+(r6.xyxw)).xyw;
    // 63: mad r6.xyw, r2.wwww, r6.xyxw, r7.xyxw
    r6.xyw = ((r2.wwww)*(r6.xyxw)+(r7.xyxw)).xyw;
    // 64: mul r7.xyw, cb0[4].xyxz, cb0[4].wwww
    r7.xyw = ((source[4].xyxz)*(source[4].wwww)).xyw;
    // 65: max r9.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r9.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 66: min r9.xyz, r9.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r9.xyz = (min(r9.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 67: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 68: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 69: add r7.xyw, -r9.xyxz, r7.xyxw
    r7.xyw = ((-(r9.xyxz))+(r7.xyxw)).xyw;
    // 70: mad r7.xyw, r2.wwww, r7.xyxw, r9.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r9.xyxz)).xyw;
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 72: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 73: mad r6.xyw, r9.xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 74: mul r7.xyw, cb0[5].xyxz, cb0[5].wwww
    r7.xyw = ((source[5].xyxz)*(source[5].wwww)).xyw;
    // 75: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 76: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 77: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 78: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 79: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 80: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 81: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 82: mad r6.xyw, r9.yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 83: mul r7.xyw, cb0[6].xyxz, cb0[6].wwww
    r7.xyw = ((source[6].xyxz)*(source[6].wwww)).xyw;
    // 84: max r10.xyz, r7.xywx, l(0.002170, 0.002170, 0.002170, 0.000000)
    r10.xyz = (max(r7.xywx,float4(0.002170,0.002170,0.002170,0.000000))).xyz;
    // 85: min r10.xyz, r10.xyzx, l(100.000000, 100.000000, 100.000000, 0.000000)
    r10.xyz = (min(r10.xyzx,float4(100.000000,100.000000,100.000000,0.000000))).xyz;
    // 86: max r7.xyw, r7.xyxw, l(0.010000, 0.010000, 0.000000, 0.010000)
    r7.xyw = (max(r7.xyxw,float4(0.010000,0.010000,0.000000,0.010000))).xyw;
    // 87: min r7.xyw, r7.xyxw, l(100.000000, 100.000000, 0.000000, 100.000000)
    r7.xyw = (min(r7.xyxw,float4(100.000000,100.000000,0.000000,100.000000))).xyw;
    // 88: add r7.xyw, -r10.xyxz, r7.xyxw
    r7.xyw = ((-(r10.xyxz))+(r7.xyxw)).xyw;
    // 89: mad r7.xyw, r2.wwww, r7.xyxw, r10.xyxz
    r7.xyw = ((r2.wwww)*(r7.xyxw)+(r10.xyxz)).xyw;
    // 90: add r7.xyw, -r6.xyxw, r7.xyxw
    r7.xyw = ((-(r6.xyxw))+(r7.xyxw)).xyw;
    // 91: mad r6.xyw, r9.zzzz, r7.xyxw, r6.xyxw
    r6.xyw = ((r9.zzzz)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 92: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 94: mad r6.xyw, cb0[16].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 95: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 97: mad r6.xyw, cb0[16].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[16].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 98: mad r7.xyw, cb0[8].wwww, cb0[8].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[8].wwww)*(source[8].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 99: mad r10.xyz, cb0[9].wwww, cb0[9].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[9].wwww)*(source[9].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 100: mul r7.xyw, r7.xyxw, r10.xyxz
    r7.xyw = ((r7.xyxw)*(r10.xyxz)).xyw;
    // 101: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 102: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 104: mad r3.xyz, cb0[16].xxxx, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].xxxx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 105: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: add r10.xyz, -r3.xyzx, r3.wwww
    r10.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 107: mad r3.xyz, cb0[16].yyyy, r10.xyzx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 108: mul r10.xyz, r3.xyzx, r6.xywx
    r10.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 109: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 111: mad r3.xyz, cb0[16].xxxx, r3.xyzx, r10.xyzx
    r3.xyz = ((source[16].xxxx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 112: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 113: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 114: mad r3.xyz, cb0[16].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[16].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 115: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 116: mul r3.w, cb0[7].z, l(1.500000)
    r3.w = ((source[7].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 117: add r4.w, -cb0[7].w, l(1.000000)
    r4.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r4.w, r4.w, cb0[15].z
    r4.w = ((r4.wwww)*(source[15].zzzz)).w;
    // 119: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 120: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 121: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 123: mad r3.w, r3.w, l(0.500000), cb0[7].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).w;
    // 124: frc r4.w, cb0[7].x
    r4.w = (frac(source[7].xxxx)).w;
    // 125: add r5.w, -r4.w, cb0[7].x
    r5.w = ((-(r4.wwww))+(source[7].xxxx)).w;
    // 126: mul r10.z, r5.w, l(0.125000)
    r10.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 127: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 128: mul r10.y, cb0[7].y, cb0[11].y
    r10.y = ((source[7].yyyy)*(source[11].yyyy)).y;
    // 129: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 130: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 131: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 132: add r6.xy, r6.xyxx, r10.xyxx
    r6.xy = ((r6.xyxx)+(r10.xyxx)).xy;
    // 133: add r6.xy, r6.xyxx, r10.zwzz
    r6.xy = ((r6.xyxx)+(r10.zwzz)).xy;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 135: mul r6.xyw, r3.wwww, r10.xyxz
    r6.xyw = ((r3.wwww)*(r10.xyxz)).xyw;
    // 136: mul r3.w, r4.w, r10.w
    r3.w = ((r4.wwww)*(r10.wwww)).w;
    // 137: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 138: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 139: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 140: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 141: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 142: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 143: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 144: mov_sat r1.w, cb0[18].y
    r1.w = (saturate(source[18].yyyy)).w;
    // 145: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 146: add r3.w, -cb0[19].y, cb0[19].x
    r3.w = ((-(source[19].yyyy))+(source[19].xxxx)).w;
    // 147: mad r3.w, r9.x, r3.w, cb0[19].y
    r3.w = ((r9.xxxx)*(r3.wwww)+(source[19].yyyy)).w;
    // 148: add r4.w, -r3.w, cb0[19].w
    r4.w = ((-(r3.wwww))+(source[19].wwww)).w;
    // 149: mad r3.w, r9.y, r4.w, r3.w
    r3.w = ((r9.yyyy)*(r4.wwww)+(r3.wwww)).w;
    // 150: add r4.w, -r3.w, cb0[20].y
    r4.w = ((-(r3.wwww))+(source[20].yyyy)).w;
    // 151: mad r3.w, r9.z, r4.w, r3.w
    r3.w = ((r9.zzzz)*(r4.wwww)+(r3.wwww)).w;
    // 152: mul r3.w, r6.z, r3.w
    r3.w = ((r6.zzzz)*(r3.wwww)).w;
    // 153: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 154: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 156: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 157: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 159: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 160: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 161: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 162: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 163: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 164: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 165: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 167: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 168: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 171: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 172: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 174: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 175: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 176: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 177: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 178: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 180: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 181: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 182: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 183: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 184: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 185: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 186: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 187: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 188: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 189: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 190: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 191: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 192: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 194: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 195: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 196: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 197: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 198: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 199: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 200: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 201: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 202: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 203: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 204: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 205: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 206: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 207: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 208: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 209: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 210: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 211: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 212: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 213: mul_sat r6.xyz, cb0[13].xyzx, cb0[13].wwww
    r6.xyz = (saturate((source[13].xyzx)*(source[13].wwww))).xyz;
    // 214: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 215: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 216: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 217: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 218: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 219: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 220: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 221: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 222: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 223: mul r0.x, r0.x, cb0[20].z
    r0.x = ((r0.xxxx)*(source[20].zzzz)).x;
    // 224: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 225: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 226: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 228: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 229: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 230: mul o0.xyz, r0.xyzx, cb0[21].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)).xyz;
    // 231: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 232: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 233: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 234: ret
    return output;
}

// source.character.classic-parts-lower.v1 / source program 3ffaa7d2addaef40b0ca55252de0fba3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight10(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.wxyz, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
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
    // 37: mul r10.xy, r9.xyxx, cb0[14].xxxx
    r10.xy = ((r9.xyxx)*(source[14].xxxx)).xy;
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
    // 44: mad r9.xyz, cb0[14].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[14].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 65: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 66: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 67: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 68: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 69: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 71: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 72: mul r1.x, r1.x, cb0[15].y
    r1.x = ((r1.xxxx)*(source[15].yyyy)).x;
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
    // 88: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 89: rcp r0.x, cb0[15].z
    r0.x = (1.0/(source[15].zzzz)).x;
    // 90: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 91: mul r9.xyz, r4.xyzx, cb0[15].zzzz
    r9.xyz = ((r4.xyzx)*(source[15].zzzz)).xyz;
    // 92: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 93: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 94: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 95: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 96: mad r4.xyz, r9.xyzx, cb0[15].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[15].zzzz)+(r4.xyzx)).xyz;
    // 97: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 98: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 99: add r0.x, cb0[15].z, l(1.000000)
    r0.x = ((source[15].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 120: mul r11.xyz, r9.xyzx, cb0[18].yyyy
    r11.xyz = ((r9.xyzx)*(source[18].yyyy)).xyz;
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
    // 132: mul_sat r6.xy, r6.xzxx, cb0[16].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[16].yyyy))).xy;
    // 133: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 134: add_sat r6.y, r6.y, -cb0[16].z
    r6.y = (saturate((r6.yyyy)+(-(source[16].zzzz)))).y;
    // 135: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 136: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 137: mul r6.y, r6.y, cb0[16].w
    r6.y = ((r6.yyyy)*(source[16].wwww)).y;
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
    // 148: mul r0.z, r0.z, cb0[18].w
    r0.z = ((r0.zzzz)*(source[18].wwww)).z;
    // 149: mad r6.y, cb0[18].z, r6.y, -r4.z
    r6.y = ((source[18].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 150: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 151: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 152: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 153: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 154: mad r6.yzw, -cb0[18].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[18].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 155: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 156: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 157: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 158: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 159: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 160: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 161: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 162: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 163: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 164: mad r7.xyz, cb0[14].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[14].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 165: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 166: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 167: mad r7.xyz, cb0[14].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[14].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 171: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 172: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 173: add r12.xyz, -r8.yzwy, r0.zzzz
    r12.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 174: mad r8.xyz, cb0[14].yyyy, r12.xyzx, r8.yzwy
    r8.xyz = ((source[14].yyyy)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 175: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 176: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 177: mad r8.xyz, cb0[14].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[14].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 178: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 179: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 180: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 181: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 182: add r1.yzw, -cb0[8].xxyz, cb0[9].xxyz
    r1.yzw = ((-(source[8].xxyz))+(source[9].xxyz)).yzw;
    // 183: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[8].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[8].xyzx)).xyz;
    // 184: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 185: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 186: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 187: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 189: mad r2.xyz, cb0[14].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[14].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 190: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 191: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 192: mad r2.xyz, cb0[14].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[14].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 193: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 194: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 195: mul r14.xyz, r14.xyzx, cb0[16].xxxx
    r14.xyz = ((r14.xyzx)*(source[16].xxxx)).xyz;
    // 196: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 197: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 198: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 199: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 200: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 201: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 202: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 203: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 204: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 206: mul r0.z, r0.z, cb0[17].x
    r0.z = ((r0.zzzz)*(source[17].xxxx)).z;
    // 207: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 208: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 211: div r1.w, cb0[17].y, r1.w
    r1.w = ((source[17].yyyy)/(r1.wwww)).w;
    // 212: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 213: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 214: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: mul r1.w, r1.w, cb0[17].z
    r1.w = ((r1.wwww)*(source[17].zzzz)).w;
    // 216: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 217: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 218: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 219: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 220: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 222: mad r1.xyz, cb0[14].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[14].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 223: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 225: mad r1.xyz, cb0[14].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[14].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 226: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 227: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 228: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r4.z, r4.z, cb0[17].w
    r4.z = ((r4.zzzz)*(source[17].wwww)).z;
    // 230: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 231: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 232: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 234: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 235: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 236: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 237: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: mul r9.y, cb0[2].y, cb0[10].y
    r9.y = ((source[2].yyyy)*(source[10].yyyy)).y;
    // 240: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 241: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 243: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 244: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t5.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 247: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 248: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 249: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.w, cb0[11].y, cb0[17].w
    r1.w = ((source[11].yyyy)*(source[17].wwww)).w;
    // 251: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 253: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 255: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 256: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 257: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 258: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 259: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 260: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 261: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 262: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 263: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 264: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 265: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 266: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 272: mul r9.xyz, r3.xyzx, cb0[11].zzzz
    r9.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 273: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 275: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 276: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 278: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 279: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 280: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 281: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 282: mul r0.y, r2.w, cb0[19].x
    r0.y = ((r2.wwww)*(source[19].xxxx)).y;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 284: add r0.w, -cb0[19].y, l(2.000000)
    r0.w = ((-(source[19].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 285: mad r0.w, r4.x, r0.w, cb0[19].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[19].yyyy)).w;
    // 286: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 287: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 288: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 289: mul r0.xyz, r0.xyzx, cb0[19].zzzz
    r0.xyz = ((r0.xyzx)*(source[19].zzzz)).xyz;
    // 290: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 293: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 294: mul r0.xyz, r0.xyzx, cb0[19].wwww
    r0.xyz = ((r0.xyzx)*(source[19].wwww)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 297: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 298: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 299: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 300: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 301: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 302: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 303: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 304: ret
    return output;
}

// source.character.classic-parts-upper.v1 / source program 4693c9c05b3e574ca63f499025c539d3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight11(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[26]=float4(input.lightColor,1.0);
    source[27].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[27].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[27].xxxx)) * 0xffffffffu)).w;
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
    // 42: mul r10.xy, r9.xyxx, cb0[16].xxxx
    r10.xy = ((r9.xyxx)*(source[16].xxxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mad r9.xy, cb0[16].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[16].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 45: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 46: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 47: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r12.xyz, cb0[18].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[18].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 69: add r1.x, -cb0[17].y, cb0[17].x
    r1.x = ((-(source[17].yyyy))+(source[17].xxxx)).x;
    // 70: mad r1.x, r11.w, r1.x, cb0[17].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[17].yyyy)).x;
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
    // 78: add r1.y, -r1.x, cb0[18].y
    r1.y = ((-(r1.xxxx))+(source[18].yyyy)).y;
    // 79: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 80: mul r1.x, r1.x, cb0[18].z
    r1.x = ((r1.xxxx)*(source[18].zzzz)).x;
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
    // 97: rcp r0.x, cb0[18].w
    r0.x = (1.0/(source[18].wwww)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r12.xyz, r4.xyzx, cb0[18].wwww
    r12.xyz = ((r4.xyzx)*(source[18].wwww)).xyz;
    // 100: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r12.xyzx, cb0[18].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[18].wwww)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[18].w, l(1.000000)
    r0.x = ((source[18].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 128: mul r13.xyz, r12.xyzx, cb0[22].wwww
    r13.xyz = ((r12.xyzx)*(source[22].wwww)).xyz;
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
    // 142: dp3 r6.w, cb0[15].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[15].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 143: add r7.xyz, r6.wwww, -cb0[15].xyzx
    r7.xyz = ((r6.wwww)+(-(source[15].xyzx))).xyz;
    // 144: mad r7.xyz, r5.wwww, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[15].xyzx)).xyz;
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
    // 150: add_sat r4.z, r11.w, cb0[23].x
    r4.z = (saturate((r11.wwww)+(source[23].xxxx))).z;
    // 151: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 153: mul_sat r6.xy, r6.xzxx, cb0[19].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[19].zzzz))).xy;
    // 154: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 155: add_sat r6.y, r6.y, -cb0[19].w
    r6.y = (saturate((r6.yyyy)+(-(source[19].wwww)))).y;
    // 156: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 157: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 158: mul r6.y, r6.y, cb0[20].x
    r6.y = ((r6.yyyy)*(source[20].xxxx)).y;
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
    // 169: mul r8.x, r7.w, cb0[23].y
    r8.x = ((r7.wwww)*(source[23].yyyy)).x;
    // 170: mad r0.z, -r7.w, cb0[23].y, r0.z
    r0.z = ((-(r7.wwww))*(source[23].yyyy)+(r0.zzzz)).z;
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
    // 177: mad r6.yzw, -cb0[22].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[22].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 178: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 179: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 180: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 181: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 182: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 183: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 184: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 185: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 186: mad r7.xyz, cb0[17].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[17].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 187: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 189: mad r7.xyz, cb0[17].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[17].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 190: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 193: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 194: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[8].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[8].xyzx)).xyz;
    // 195: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 196: dp3 r0.z, r8.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 197: add r12.xyz, -r8.yzwy, r0.zzzz
    r12.xyz = ((-(r8.yzwy))+(r0.zzzz)).xyz;
    // 198: mad r8.xyz, cb0[17].zzzz, r12.xyzx, r8.yzwy
    r8.xyz = ((source[17].zzzz)*(r12.xyzx)+(r8.yzwy)).xyz;
    // 199: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 200: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 201: mad r8.xyz, cb0[17].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[17].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 202: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 203: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 204: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 206: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 207: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 208: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 209: mul r1.xyz, r1.xyzx, cb0[19].xxxx
    r1.xyz = ((r1.xyzx)*(source[19].xxxx)).xyz;
    // 210: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 211: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 213: mad r14.xyz, cb0[17].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[17].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 214: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 215: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 216: mad r14.xyz, cb0[17].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[17].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 217: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 218: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 219: mul r15.xyz, r15.xyzx, cb0[19].yyyy
    r15.xyz = ((r15.xyzx)*(source[19].yyyy)).xyz;
    // 220: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 221: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 222: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 223: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 224: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 225: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 226: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 227: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 228: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 229: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 230: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 231: add r1.w, -cb0[20].z, cb0[20].y
    r1.w = ((-(source[20].zzzz))+(source[20].yyyy)).w;
    // 232: mad r1.w, r11.w, r1.w, cb0[20].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[20].zzzz)).w;
    // 233: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 234: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 235: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 236: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 238: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 239: div r1.w, cb0[20].w, r1.w
    r1.w = ((source[20].wwww)/(r1.wwww)).w;
    // 240: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 241: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 242: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.w, r1.w, cb0[21].x
    r1.w = ((r1.wwww)*(source[21].xxxx)).w;
    // 244: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 245: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 246: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 247: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 248: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 249: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 250: mad r1.xyz, cb0[17].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[17].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 251: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 252: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 253: mad r1.xyz, cb0[17].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[17].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 254: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 255: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 256: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 257: mul r4.z, r4.z, cb0[21].y
    r4.z = ((r4.zzzz)*(source[21].yyyy)).z;
    // 258: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 259: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 260: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 261: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 262: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 263: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 264: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 265: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 266: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 267: mul r10.y, cb0[2].y, cb0[11].y
    r10.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 268: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 269: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 270: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 271: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 272: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t5.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 274: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 275: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 276: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 277: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 278: mul r1.w, cb0[12].y, cb0[21].y
    r1.w = ((source[12].yyyy)*(source[21].yyyy)).w;
    // 279: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 280: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 281: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 282: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 283: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 284: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 285: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 286: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 287: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 288: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 289: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 290: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 291: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 292: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 293: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t5.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 294: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 295: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 296: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 297: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 298: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 299: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 300: mul r10.xyz, r3.xyzx, cb0[12].zzzz
    r10.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 301: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 302: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 303: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 304: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 305: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 306: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 307: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 308: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 309: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 310: mul r0.z, r2.w, cb0[23].z
    r0.z = ((r2.wwww)*(source[23].zzzz)).z;
    // 311: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 312: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 313: min r0.w, r0.w, cb0[23].z
    r0.w = (min(r0.wwww,source[23].zzzz)).w;
    // 314: add r1.w, -cb0[24].y, cb0[24].x
    r1.w = ((-(source[24].yyyy))+(source[24].xxxx)).w;
    // 315: mad r1.w, cb0[23].w, r1.w, cb0[24].y
    r1.w = ((source[23].wwww)*(r1.wwww)+(source[24].yyyy)).w;
    // 316: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 317: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 318: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 319: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 320: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 321: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 322: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 323: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 324: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t6.xyzw, s7, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 325: add r0.y, cb0[24].w, -cb0[25].x
    r0.y = ((source[24].wwww)+(-(source[25].xxxx))).y;
    // 326: mad r0.y, cb0[24].z, r0.y, cb0[25].x
    r0.y = ((source[24].zzzz)*(r0.yyyy)+(source[25].xxxx)).y;
    // 327: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 328: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t6.xwyz, s7, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 329: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 330: add r0.w, -cb0[25].y, l(2.000000)
    r0.w = ((-(source[25].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 331: mad r0.w, r4.x, r0.w, cb0[25].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[25].yyyy)).w;
    // 332: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 333: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 334: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 335: mul r0.xyz, r0.xyzx, cb0[25].zzzz
    r0.xyz = ((r0.xyzx)*(source[25].zzzz)).xyz;
    // 336: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 337: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 338: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 339: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 340: mul r0.xyz, r0.xyzx, cb0[25].wwww
    r0.xyz = ((r0.xyzx)*(source[25].wwww)).xyz;
    // 341: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 342: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 343: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 344: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 345: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 346: mul o0.xyz, r0.xyzx, cb0[26].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)).xyz;
    // 347: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 348: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 349: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 350: ret
    return output;
}

// source.character.classic-head-legacy.v1 / source program 8d176013b3896b4b887ccd106afc94e6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight12(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[8]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[9]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].x=(g_SourceCharacterTime.xxxx).x;
    source[15].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[15].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[15].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[16].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: add r0.xyzw, v6.yzxy, cb0[0].yzxy
    r0.xyzw = ((v6.yzxy)+(source[0].yzxy)).xyzw;
    // 2: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).x;
    // 3: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 4: mul r1.xyz, v6.yyyy, cb1[1].xywx
    r1.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 5: mad r1.xyz, cb1[0].xywx, v6.xxxx, r1.xyzx
    r1.xyz = ((projection[0].xywx)*(v6.xxxx)+(r1.xyzx)).xyz;
    // 6: mad r1.xyz, cb1[2].xywx, v6.zzzz, r1.xyzx
    r1.xyz = ((projection[2].xywx)*(v6.zzzz)+(r1.xyzx)).xyz;
    // 7: mad r1.xyz, cb1[3].xywx, v6.wwww, r1.xyzx
    r1.xyz = ((projection[3].xywx)*(v6.wwww)+(r1.xyzx)).xyz;
    // 8: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 9: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s0
    r1.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 16: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 17: mul r2.xyz, r1.wwww, v5.xyzx
    r2.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 18: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 19: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 20: mul r3.xyz, r1.wwww, v3.zxyz
    r3.xyz = ((r1.wwww)*(v3.zxyz)).xyz;
    // 21: mov_sat r2.w, -r3.x
    r2.w = (saturate(-(r3.xxxx))).w;
    // 22: add r3.w, -|r2.z|, l(1.000000)
    r3.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: dp2 r4.w, r4.xyxx, r4.xyxx
    r4.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 26: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: max r4.w, r4.w, l(0.000000)
    r4.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 29: add r4.z, r4.w, l(0.000010)
    r4.z = ((r4.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 31: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 32: div r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)/(r4.wwww)).xyz;
    // 33: dp3 r4.w, r4.xyzx, r2.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 34: add r5.x, -|r4.w|, l(1.000000)
    r5.x = ((-(abs(r4.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mul r5.y, r3.w, r5.x
    r5.y = ((r3.wwww)*(r5.xxxx)).y;
    // 36: lt r5.z, |r5.y|, l(0.000001)
    r5.z = (asfloat((uint4)((abs(r5.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 37: log r5.y, |r5.y|
    r5.y = (log2(abs(r5.yyyy))).y;
    // 38: mul r5.y, r5.y, l(10.000000)
    r5.y = ((r5.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000))).y;
    // 39: exp r5.y, r5.y
    r5.y = (exp2(r5.yyyy)).y;
    // 40: mul r2.w, r2.w, r5.y
    r2.w = ((r2.wwww)*(r5.yyyy)).w;
    // 41: mul r2.w, r2.w, l(0.050000)
    r2.w = ((r2.wwww)*(float4(0.050000,0.050000,0.050000,0.050000))).w;
    // 42: mul r6.xyz, r2.wwww, cb0[2].xyzx
    r6.xyz = ((r2.wwww)*(source[2].xyzx)).xyz;
    // 43: movc r5.yzw, r5.zzzz, l(0,0,0,0), r6.xxyz
    r5.yzw = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxyz)).yzw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v2.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: mul r7.xyz, r6.xyzx, cb0[3].xyzx
    r7.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 46: div r8.xy, l(1024.000000, 1024.000000, 0.000000, 0.000000), cb0[4].xyxx
    r8.xy = ((float4(1024.000000,1024.000000,0.000000,0.000000))/(source[4].xyxx)).xy;
    // 47: mad r8.zw, -cb0[4].xxxy, l(0.000000, 0.000000, 0.500000, 0.500000), cb0[4].zzzw
    r8.zw = ((-(source[4].xxxy))*(float4(0.000000,0.000000,0.500000,0.500000))+(source[4].zzzw)).zw;
    // 48: div r8.zw, r8.zzzw, cb0[4].xxxy
    r8.zw = ((r8.zzzw)/(source[4].xxxy)).zw;
    // 49: mad r8.xy, v2.xyxx, r8.xyxx, -r8.zwzz
    r8.xy = ((v2.xyxx)*(r8.xyxx)+(-(r8.zwzz))).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r8.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r8.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mad r8.xyz, -r6.xyzx, cb0[3].xyzx, r8.xyzx
    r8.xyz = ((-(r6.xyzx))*(source[3].xyzx)+(r8.xyzx)).xyz;
    // 52: mad r7.xyz, r8.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r8.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v2.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 54: add r6.xyz, r6.xyzx, -r7.xyzx
    r6.xyz = ((r6.xyzx)+(-(r7.xyzx))).xyz;
    // 55: mad r6.xyz, r8.yyyy, r6.xyzx, r7.xyzx
    r6.xyz = ((r8.yyyy)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 56: mul r2.w, cb0[5].z, l(1.500000)
    r2.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 57: add r6.w, -cb0[5].w, l(1.000000)
    r6.w = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul r6.w, r6.w, cb0[15].x
    r6.w = ((r6.wwww)*(source[15].xxxx)).w;
    // 59: mul r6.w, r6.w, l(6.283185)
    r6.w = ((r6.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 60: sincos r6.w, null, r6.w
    r6.w = (sin(r6.wwww)).w;
    // 61: add r6.w, r6.w, l(1.000000)
    r6.w = ((r6.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: mul r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)*(r6.wwww)).w;
    // 63: mad r2.w, r2.w, l(0.500000), cb0[5].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 64: frc r6.w, cb0[5].x
    r6.w = (frac(source[5].xxxx)).w;
    // 65: add r7.x, -r6.w, cb0[5].x
    r7.x = ((-(r6.wwww))+(source[5].xxxx)).x;
    // 66: mul r7.z, r7.x, l(0.125000)
    r7.z = ((r7.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 67: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 68: mul r7.y, cb0[5].y, cb0[6].y
    r7.y = ((source[5].yyyy)*(source[6].yyyy)).y;
    // 69: frc r8.y, v2.x
    r8.y = (frac(v2.xxxx)).y;
    // 70: mul r9.x, r8.y, l(0.125000)
    r9.x = ((r8.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 71: mov r9.y, v2.y
    r9.y = (v2.yyyy).y;
    // 72: add r7.xy, r7.xyxx, r9.xyxx
    r7.xy = ((r7.xyxx)+(r9.xyxx)).xy;
    // 73: add r7.xy, r7.xyxx, r7.zwzz
    r7.xy = ((r7.xyxx)+(r7.zwzz)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t4.xyzw, s5, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 75: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 76: mul r2.w, r6.w, r7.w
    r2.w = ((r6.wwww)*(r7.wwww)).w;
    // 77: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 78: mad r6.xyz, r2.wwww, r7.xyzx, r6.xyzx
    r6.xyz = ((r2.wwww)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 79: mul r2.w, cb0[7].y, cb0[15].x
    r2.w = ((source[7].yyyy)*(source[15].xxxx)).w;
    // 80: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 81: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 82: mul r7.y, r2.w, l(0.020000)
    r7.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 83: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 84: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 85: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 86: mad r0.xy, cb0[7].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[7].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 87: mul r0.z, cb0[7].x, l(0.001000)
    r0.z = ((source[7].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 88: mov r7.x, l(0)
    r7.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 89: mad r0.xy, r0.zzzz, r0.xyxx, r7.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r7.xyxx)).xy;
    // 90: dp2 r0.z, cb0[8].xyxx, r0.xyxx
    r0.z = (dot((source[8].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 91: dp2 r0.y, cb0[9].xyxx, r0.xyxx
    r0.y = (dot((source[9].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 92: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 93: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 95: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 96: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r6.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r6.xyzx))).xyz;
    // 97: mad r0.xyz, r0.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 98: add r0.w, r2.w, l(1.000000)
    r0.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 100: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 101: mul r7.xyz, r0.xyzx, cb0[7].zzzz
    r7.xyz = ((r0.xyzx)*(source[7].zzzz)).xyz;
    // 102: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 104: mad r0.xyz, cb0[7].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[7].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 105: mad r0.xyz, r0.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 106: dp3 r0.w, r4.zxyz, r3.xyzx
    r0.w = (dot((r4.zxyz).xyz,(r3.xyzx).xyz).xxxx).w;
    // 107: max r2.w, r0.w, l(0.000000)
    r2.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 108: min r3.y, r2.w, l(1.000000)
    r3.y = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 109: mad r3.z, -r3.w, r5.x, l(1.000000)
    r3.z = ((-(r3.wwww))*(r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 110: mul r3.z, r3.z, r3.y
    r3.z = ((r3.zzzz)*(r3.yyyy)).z;
    // 111: add r3.w, -cb0[16].w, cb0[16].z
    r3.w = ((-(source[16].wwww))+(source[16].zzzz)).w;
    // 112: mad r3.z, r3.z, r3.w, cb0[16].w
    r3.z = ((r3.zzzz)*(r3.wwww)+(source[16].wwww)).z;
    // 113: mul_sat r3.y, r3.y, r3.z
    r3.y = (saturate((r3.yyyy)*(r3.zzzz))).y;
    // 114: mul r3.yzw, r1.xxyz, r3.yyyy
    r3.yzw = ((r1.xxyz)*(r3.yyyy)).yzw;
    // 115: mul r6.xyz, cb0[10].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r6.xyz = ((source[10].xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 116: mad r7.xyz, -cb0[10].xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[10].xyzx))*(float4(0.500000,0.500000,0.500000,0.000000))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 117: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 119: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 120: mul r6.xyz, r6.xyzx, r0.wwww
    r6.xyz = ((r6.xyzx)*(r0.wwww)).xyz;
    // 121: mad r3.yzw, r3.yyzw, r7.xxyz, r6.xxyz
    r3.yzw = ((r3.yyzw)*(r7.xxyz)+(r6.xxyz)).yzw;
    // 122: mov_sat r3.x, r3.x
    r3.x = (saturate(r3.xxxx)).x;
    // 123: mul r0.w, r3.x, r3.x
    r0.w = ((r3.xxxx)*(r3.xxxx)).w;
    // 124: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: add r6.xyz, -cb0[11].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[11].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 127: mad r6.xyz, r0.wwww, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)+(source[11].xyzx)).xyz;
    // 128: mad r6.xyz, r3.yzwy, l(0.500000, 0.500000, 0.500000, 0.000000), r6.xyzx
    r6.xyz = ((r3.yzwy)*(float4(0.500000,0.500000,0.500000,0.000000))+(r6.xyzx)).xyz;
    // 129: mul_sat r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = (saturate((r0.xyzx)*(r6.xyzx))).xyz;
    // 130: mul r3.xyz, r3.yzwy, cb0[12].xyzx
    r3.xyz = ((r3.yzwy)*(source[12].xyzx)).xyz;
    // 131: add r0.w, r8.z, -cb0[17].w
    r0.w = ((r8.zzzz)+(-(source[17].wwww))).w;
    // 132: mul_sat r0.w, r0.w, l(2.500000)
    r0.w = (saturate((r0.wwww)*(float4(2.500000,2.500000,2.500000,2.500000)))).w;
    // 133: add r3.w, -cb0[18].y, cb0[18].x
    r3.w = ((-(source[18].yyyy))+(source[18].xxxx)).w;
    // 134: mad r0.w, r0.w, r3.w, cb0[18].y
    r0.w = ((r0.wwww)*(r3.wwww)+(source[18].yyyy)).w;
    // 135: mul r6.xyz, r0.wwww, cb0[14].xyzx
    r6.xyz = ((r0.wwww)*(source[14].xyzx)).xyz;
    // 136: mul r6.xyz, r6.xyzx, r8.xxxx
    r6.xyz = ((r6.xyzx)*(r8.xxxx)).xyz;
    // 137: mad r2.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 138: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 139: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 140: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 141: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 142: add r2.x, -cb0[18].w, l(5.250000)
    r2.x = ((-(source[18].wwww))+(float4(5.250000,5.250000,5.250000,5.250000))).x;
    // 143: mad r2.x, cb0[18].z, r2.x, cb0[18].w
    r2.x = ((source[18].zzzz)*(r2.xxxx)+(source[18].wwww)).x;
    // 144: mul r2.x, r2.x, r8.w
    r2.x = ((r2.xxxx)*(r8.wwww)).x;
    // 145: mul r2.xy, r2.xxxx, cb0[19].xzxx
    r2.xy = ((r2.xxxx)*(source[19].xzxx)).xy;
    // 146: lt r2.z, |r1.w|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 147: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 148: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 149: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 150: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 152: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 153: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 155: mul r0.w, r0.w, cb0[19].y
    r0.w = ((r0.wwww)*(source[19].yyyy)).w;
    // 156: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 157: mul r1.w, r4.w, r4.w
    r1.w = ((r4.wwww)*(r4.wwww)).w;
    // 158: lt r2.x, r1.w, l(0.000001)
    r2.x = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 159: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 160: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 161: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 162: mul r1.w, r1.w, cb0[19].w
    r1.w = ((r1.wwww)*(source[19].wwww)).w;
    // 163: add r2.y, r1.x, l(0.200000)
    r2.y = ((r1.xxxx)+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 164: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 166: movc r1.w, r2.x, l(0), r1.w
    r1.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 167: mad r1.xyz, r0.wwww, r1.xyzx, r1.wwww
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r1.wwww)).xyz;
    // 168: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 169: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 170: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 171: mul r1.xyz, r1.xyzx, cb0[13].xyzx
    r1.xyz = ((r1.xyzx)*(source[13].xyzx)).xyz;
    // 172: mad r0.xyz, r0.xyzx, r3.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 173: add r0.xyz, r0.xyzx, r5.yzwy
    r0.xyz = ((r0.xyzx)+(r5.yzwy)).xyz;
    // 174: mul r1.xyz, r2.wwww, cb2[3].xyzx
    r1.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 175: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 176: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 177: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 178: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 179: ret
    return output;
}

// source.character.classic-armor-emissive.v1 / source program 449cc07427498e4dbd51170031c9615d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight13(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].x=(g_SourceCharacterTime.xxxx).x;
    source[19].y=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[19].z=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[19].w=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[20].x=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23]=float4(input.lightColor,1.0);
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[24].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[24].xxxx)) * 0xffffffffu)).w;
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
    r8.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[14].xyzw
    r9.xyzw = ((r8.xyzw)*(source[14].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 45: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 52: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
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
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[16].z
    r0.x = (1.0/(source[16].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[16].zzzz
    r9.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[16].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[16].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[16].z, l(1.000000)
    r0.x = ((source[16].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 128: mul r11.xyz, r9.xyzx, cb0[20].zzzz
    r11.xyz = ((r9.xyzx)*(source[20].zzzz)).xyz;
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
    // 140: mul_sat r6.xy, r6.xzxx, cb0[17].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[17].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[17].z
    r6.y = (saturate((r6.yyyy)+(-(source[17].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[17].w
    r6.y = ((r6.yyyy)*(source[17].wwww)).y;
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
    // 156: mul r0.z, r0.z, cb0[21].x
    r0.z = ((r0.zzzz)*(source[21].xxxx)).z;
    // 157: mad r6.y, cb0[20].w, r6.y, -r4.z
    r6.y = ((source[20].wwww)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[20].zzzz, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].zzzz))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 166: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 167: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 169: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 170: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r7.xyz, cb0[15].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 173: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 174: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 175: mad r7.xyz, cb0[15].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 176: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 180: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 181: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 182: mad r8.xyz, cb0[15].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 183: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 186: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 187: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 189: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 190: add r1.yzw, -cb0[8].xxyz, cb0[9].xxyz
    r1.yzw = ((-(source[8].xxyz))+(source[9].xxyz)).yzw;
    // 191: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[8].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[8].xyzx)).xyz;
    // 192: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 193: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 194: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 195: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r2.xyz, cb0[15].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 198: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r2.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 201: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 202: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 203: mul r14.xyz, r14.xyzx, cb0[17].xxxx
    r14.xyz = ((r14.xyzx)*(source[17].xxxx)).xyz;
    // 204: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 205: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 206: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 207: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 208: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 209: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 210: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 211: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 212: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 213: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 214: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 215: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 216: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 219: div r1.w, cb0[18].y, r1.w
    r1.w = ((source[18].yyyy)/(r1.wwww)).w;
    // 220: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 221: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 222: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 224: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 225: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 226: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 227: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 228: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 230: mad r1.xyz, cb0[15].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 231: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 233: mad r1.xyz, cb0[15].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 234: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 235: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 236: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r4.z, r4.z, cb0[19].x
    r4.z = ((r4.zzzz)*(source[19].xxxx)).z;
    // 238: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 239: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 240: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 241: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 242: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 243: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 244: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 245: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 247: mul r9.y, cb0[2].y, cb0[10].y
    r9.y = ((source[2].yyyy)*(source[10].yyyy)).y;
    // 248: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 249: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 251: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 252: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 253: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 254: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 255: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 256: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 257: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 258: mul r1.w, cb0[11].y, cb0[19].x
    r1.w = ((source[11].yyyy)*(source[19].xxxx)).w;
    // 259: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 260: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 261: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 262: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 263: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 264: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 265: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 266: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 267: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 268: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 269: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 270: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 271: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 272: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 274: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 275: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 276: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 278: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 279: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 280: mul r9.xyz, r3.xyzx, cb0[11].zzzz
    r9.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 281: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 283: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 284: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 285: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 286: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 287: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 288: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 289: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 290: mul r0.y, r2.w, cb0[21].y
    r0.y = ((r2.wwww)*(source[21].yyyy)).y;
    // 291: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 292: add r0.w, -cb0[21].z, l(2.000000)
    r0.w = ((-(source[21].zzzz))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 293: mad r0.w, r4.x, r0.w, cb0[21].z
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[21].zzzz)).w;
    // 294: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 295: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 296: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 297: mul r0.xyz, r0.xyzx, cb0[21].wwww
    r0.xyz = ((r0.xyzx)*(source[21].wwww)).xyz;
    // 298: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 301: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 302: mul r0.xyz, r0.xyzx, cb0[22].xxxx
    r0.xyz = ((r0.xyzx)*(source[22].xxxx)).xyz;
    // 303: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 304: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 305: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 306: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 307: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 308: mul o0.xyz, r0.xyzx, cb0[23].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)).xyz;
    // 309: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 310: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 311: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 312: ret
    return output;
}

// source.character.classic-weapon-emissive.v1 / source program 9daabe1f7b5c26458ec0d7d849944c73
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight14(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[12]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[13]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[19].z=(g_SourceCharacterTime.xxxx).x;
    source[22]=float4(input.lightColor,1.0);
    source[23].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[23].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[23].xxxx)) * 0xffffffffu)).w;
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
    r8.xyzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[14].xyzw
    r9.xyzw = ((r8.xyzw)*(source[14].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 45: mul r10.xy, r9.xyxx, cb0[15].xxxx
    r10.xy = ((r9.xyxx)*(source[15].xxxx)).xy;
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
    // 52: mad r9.xyz, cb0[15].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[15].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 74: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 75: mul r0.z, r0.z, cb0[16].x
    r0.z = ((r0.zzzz)*(source[16].xxxx)).z;
    // 76: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 77: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 79: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 80: mul r1.x, r1.x, cb0[16].y
    r1.x = ((r1.xxxx)*(source[16].yyyy)).x;
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
    // 96: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 97: rcp r0.x, cb0[16].z
    r0.x = (1.0/(source[16].zzzz)).x;
    // 98: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 99: mul r9.xyz, r4.xyzx, cb0[16].zzzz
    r9.xyz = ((r4.xyzx)*(source[16].zzzz)).xyz;
    // 100: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 101: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 102: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 103: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 104: mad r4.xyz, r9.xyzx, cb0[16].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[16].zzzz)+(r4.xyzx)).xyz;
    // 105: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 106: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 107: add r0.x, cb0[16].z, l(1.000000)
    r0.x = ((source[16].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
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
    // 128: mul r11.xyz, r9.xyzx, cb0[20].yyyy
    r11.xyz = ((r9.xyzx)*(source[20].yyyy)).xyz;
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
    // 140: mul_sat r6.xy, r6.xzxx, cb0[17].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[17].yyyy))).xy;
    // 141: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 142: add_sat r6.y, r6.y, -cb0[17].z
    r6.y = (saturate((r6.yyyy)+(-(source[17].zzzz)))).y;
    // 143: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 144: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 145: mul r6.y, r6.y, cb0[17].w
    r6.y = ((r6.yyyy)*(source[17].wwww)).y;
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
    // 156: mul r0.z, r0.z, cb0[20].w
    r0.z = ((r0.zzzz)*(source[20].wwww)).z;
    // 157: mad r6.y, cb0[20].z, r6.y, -r4.z
    r6.y = ((source[20].zzzz)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 158: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 159: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 160: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 161: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 162: mad r6.yzw, -cb0[20].yyyy, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[20].yyyy))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 163: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 164: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 165: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 166: mad r11.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r11.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 167: mad r7.xyz, r9.xxxx, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.xxxx)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 169: mad r7.xyz, r9.yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((r9.yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 170: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 171: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 172: mad r7.xyz, cb0[15].yyyy, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].yyyy)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 173: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 174: add r11.xyz, -r7.xyzx, r0.zzzz
    r11.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 175: mad r7.xyz, cb0[15].zzzz, r11.xyzx, r7.xyzx
    r7.xyz = ((source[15].zzzz)*(r11.xyzx)+(r7.xyzx)).xyz;
    // 176: mad r11.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 180: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 181: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 182: mad r8.xyz, cb0[15].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 183: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 184: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 185: mad r8.xyz, cb0[15].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[15].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 186: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 187: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 188: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 189: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 190: add r1.yzw, -cb0[8].xxyz, cb0[9].xxyz
    r1.yzw = ((-(source[8].xxyz))+(source[9].xxyz)).yzw;
    // 191: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[8].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[8].xyzx)).xyz;
    // 192: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 193: mul r1.xyz, r1.xyzx, cb0[16].wwww
    r1.xyz = ((r1.xyzx)*(source[16].wwww)).xyz;
    // 194: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 195: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r2.xyz, cb0[15].yyyy, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].yyyy)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 198: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 199: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 200: mad r2.xyz, cb0[15].zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((source[15].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 201: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 202: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 203: mul r14.xyz, r14.xyzx, cb0[17].xxxx
    r14.xyz = ((r14.xyzx)*(source[17].xxxx)).xyz;
    // 204: add r0.z, r9.y, r9.x
    r0.z = ((r9.yyyy)+(r9.xxxx)).z;
    // 205: add r0.z, r9.z, r0.z
    r0.z = ((r9.zzzz)+(r0.zzzz)).z;
    // 206: add_sat r0.z, r9.w, r0.z
    r0.z = (saturate((r9.wwww)+(r0.zzzz))).z;
    // 207: mad r2.xyz, r0.zzzz, r14.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 208: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 209: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 210: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 211: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 212: dp3 r0.z, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 213: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 214: mul r0.z, r0.z, cb0[18].x
    r0.z = ((r0.zzzz)*(source[18].xxxx)).z;
    // 215: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 216: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 217: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 218: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 219: div r1.w, cb0[18].y, r1.w
    r1.w = ((source[18].yyyy)/(r1.wwww)).w;
    // 220: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 221: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 222: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 224: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 225: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 226: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 227: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 228: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 230: mad r1.xyz, cb0[15].yyyy, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].yyyy)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 231: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 232: add r9.xyz, -r1.xyzx, r1.wwww
    r9.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 233: mad r1.xyz, cb0[15].zzzz, r9.xyzx, r1.xyzx
    r1.xyz = ((source[15].zzzz)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 234: mul r1.xyz, r11.xyzx, r1.xyzx
    r1.xyz = ((r11.xyzx)*(r1.xyzx)).xyz;
    // 235: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 236: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r4.z, r4.z, cb0[19].z
    r4.z = ((r4.zzzz)*(source[19].zzzz)).z;
    // 238: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 239: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 240: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 241: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 242: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 243: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 244: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 245: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 246: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 247: mul r9.y, cb0[2].y, cb0[10].y
    r9.y = ((source[2].yyyy)*(source[10].yyyy)).y;
    // 248: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 249: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 250: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 251: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 252: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 253: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 254: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 255: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 256: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 257: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 258: mul r1.w, cb0[11].y, cb0[19].z
    r1.w = ((source[11].yyyy)*(source[19].zzzz)).w;
    // 259: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 260: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 261: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 262: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 263: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 264: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 265: mad r3.xy, cb0[11].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[11].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 266: mul r3.z, cb0[11].x, l(0.001000)
    r3.z = ((source[11].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 267: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 268: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 269: dp2 r3.z, cb0[12].xyxx, r3.xyxx
    r3.z = (dot((source[12].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 270: dp2 r3.y, cb0[13].xyxx, r3.xyxx
    r3.y = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 271: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 272: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 274: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 275: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 276: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 278: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 279: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 280: mul r9.xyz, r3.xyzx, cb0[11].zzzz
    r9.xyz = ((r3.xyzx)*(source[11].zzzz)).xyz;
    // 281: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 282: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 283: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 284: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 285: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 286: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 287: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 288: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 289: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 290: mul r0.y, r2.w, cb0[21].x
    r0.y = ((r2.wwww)*(source[21].xxxx)).y;
    // 291: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 292: add r0.w, -cb0[21].y, l(2.000000)
    r0.w = ((-(source[21].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 293: mad r0.w, r4.x, r0.w, cb0[21].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[21].yyyy)).w;
    // 294: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 295: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 296: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 297: mul r0.xyz, r0.xyzx, cb0[21].zzzz
    r0.xyz = ((r0.xyzx)*(source[21].zzzz)).xyz;
    // 298: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 299: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 300: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 301: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 302: mul r0.xyz, r0.xyzx, cb0[21].wwww
    r0.xyz = ((r0.xyzx)*(source[21].wwww)).xyz;
    // 303: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 304: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 305: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 306: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 307: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 308: mul o0.xyz, r0.xyzx, cb0[22].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)).xyz;
    // 309: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 310: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 311: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 312: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight15(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[14]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[15]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[24].w=(g_SourceCharacterTime.xxxx).x;
    source[29]=float4(input.lightColor,1.0);
    source[30].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[30].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[30].xxxx)) * 0xffffffffu)).w;
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
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t3.xyzw, s7, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xyzw, r8.xyzw, cb0[16].xyzw
    r9.xyzw = ((r8.xyzw)*(source[16].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 44: mad r9.xyzw, r9.xyxy, l(2.000000, 2.000000, 2.000000, 2.000000), l(-1.000000, -1.000000, -1.000000, -1.000000)
    r9.xyzw = ((r9.xyxy)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).xyzw;
    // 45: dp2 r1.w, r9.zwzz, r9.zwzz
    r1.w = (dot((r9.zwzz).xy,(r9.zwzz).xy).xxxx).w;
    // 46: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r10.xy, r9.xyxx, cb0[18].xxxx
    r10.xy = ((r9.xyxx)*(source[18].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[18].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[18].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[20].wwww, r10.xyzx, r9.xyzx
    r12.xyz = ((source[20].wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 57: dp3 r1.w, r12.xyzx, r12.xyzx
    r1.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 58: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 59: div r12.xyz, r12.xyzx, r1.wwww
    r12.xyz = ((r12.xyzx)/(r1.wwww)).xyz;
    // 60: dp3 r13.x, r1.xyzx, r12.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 61: dp3 r13.y, r2.xyzx, r12.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 62: dp3 r13.z, r0.xyzx, r12.xyzx
    r13.z = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 63: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 64: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 65: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 66: dp3 r0.x, r13.xyzx, r1.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: mul r0.xyz, r13.xyzx, r0.xxxx
    r0.xyz = ((r13.xyzx)*(r0.xxxx)).xyz;
    // 68: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 69: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 70: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 71: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 72: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 73: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 74: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: add r0.z, -cb0[21].y, cb0[21].x
    r0.z = ((-(source[21].yyyy))+(source[21].xxxx)).z;
    // 76: mad r0.z, r11.x, r0.z, cb0[21].y
    r0.z = ((r11.xxxx)*(r0.zzzz)+(source[21].yyyy)).z;
    // 77: add r1.x, -r0.z, cb0[21].z
    r1.x = ((-(r0.zzzz))+(source[21].zzzz)).x;
    // 78: mad r0.z, r11.y, r1.x, r0.z
    r0.z = ((r11.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 79: add r1.x, -r0.z, cb0[21].w
    r1.x = ((-(r0.zzzz))+(source[21].wwww)).x;
    // 80: mad r0.z, r11.z, r1.x, r0.z
    r0.z = ((r11.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 82: add r1.x, -r2.w, l(1.000000)
    r1.x = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: add r1.y, -cb0[19].y, cb0[19].x
    r1.y = ((-(source[19].yyyy))+(source[19].xxxx)).y;
    // 84: mad r1.y, r11.x, r1.y, cb0[19].y
    r1.y = ((r11.xxxx)*(r1.yyyy)+(source[19].yyyy)).y;
    // 85: add r1.w, -r1.y, cb0[19].z
    r1.w = ((-(r1.yyyy))+(source[19].zzzz)).w;
    // 86: mad r1.y, r11.y, r1.w, r1.y
    r1.y = ((r11.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 87: add r1.w, -r1.y, cb0[19].w
    r1.w = ((-(r1.yyyy))+(source[19].wwww)).w;
    // 88: mad r1.y, r11.z, r1.w, r1.y
    r1.y = ((r11.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 89: add r1.w, -r1.y, cb0[20].x
    r1.w = ((-(r1.yyyy))+(source[20].xxxx)).w;
    // 90: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 91: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 92: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 93: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 94: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 95: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 96: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 97: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 98: add r1.w, -r1.y, cb0[22].x
    r1.w = ((-(r1.yyyy))+(source[22].xxxx)).w;
    // 99: mad r1.y, r11.w, r1.w, r1.y
    r1.y = ((r11.wwww)*(r1.wwww)+(r1.yyyy)).y;
    // 100: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 101: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 102: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 103: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 104: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 105: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 106: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 107: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 108: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 109: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 110: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 111: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 112: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 113: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 114: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 115: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 116: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 117: rcp r1.y, cb0[22].y
    r1.y = (1.0/(source[22].yyyy)).y;
    // 118: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 119: mul r12.xyz, r4.xyzx, cb0[22].yyyy
    r12.xyz = ((r4.xyzx)*(source[22].yyyy)).xyz;
    // 120: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 121: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 122: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 123: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 124: mad r4.xyz, r12.xyzx, cb0[22].yyyy, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[22].yyyy)+(r4.xyzx)).xyz;
    // 125: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 126: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 127: add r1.y, cb0[22].y, l(1.000000)
    r1.y = ((source[22].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 129: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 130: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 131: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 132: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 133: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 134: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 135: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 136: mul r2.w, |r1.y|, |r1.y|
    r2.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 137: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 138: mul r1.y, |r1.y|, r2.w
    r1.y = ((abs(r1.yyyy))*(r2.wwww)).y;
    // 139: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 140: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 141: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 142: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 143: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 144: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 145: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 147: mul r12.xyz, r0.xyzx, r1.yyyy
    r12.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 148: mul r13.xyz, r12.xyzx, cb0[25].yyyy
    r13.xyz = ((r12.xyzx)*(source[25].yyyy)).xyz;
    // 149: mul r2.w, r11.w, l(0.500000)
    r2.w = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 150: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 152: mul r4.w, r1.x, r4.w
    r4.w = ((r1.xxxx)*(r4.wwww)).w;
    // 153: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 154: mad r9.xyz, r2.wwww, r10.xyzx, r9.xyzx
    r9.xyz = ((r2.wwww)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 155: dp3 r2.w, r9.xyzx, r9.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: div r9.xyz, r9.xyzx, r2.wwww
    r9.xyz = ((r9.xyzx)/(r2.wwww)).xyz;
    // 158: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 159: max r4.w, r2.w, l(0.000000)
    r4.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 160: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp3 r6.w, cb0[17].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[17].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r7.xyz, r6.wwww, -cb0[17].xyzx
    r7.xyz = ((r6.wwww)+(-(source[17].xyzx))).xyz;
    // 164: mad r7.xyz, r5.wwww, r7.xyzx, cb0[17].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[17].xyzx)).xyz;
    // 165: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 166: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r7.xyz, r1.xxxx, r7.xyzx
    r7.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 168: mad r2.w, r2.w, l(0.500000), -r5.w
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).w;
    // 169: mad r7.xyz, r7.xyzx, r2.wwww, r5.wwww
    r7.xyz = ((r7.xyzx)*(r2.wwww)+(r5.wwww)).xyz;
    // 170: add_sat r2.w, r11.w, cb0[25].z
    r2.w = (saturate((r11.wwww)+(source[25].zzzz))).w;
    // 171: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 173: mul_sat r6.xy, r6.xzxx, cb0[23].xxxx
    r6.xy = (saturate((r6.xzxx)*(source[23].xxxx))).xy;
    // 174: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 175: add_sat r6.y, r6.y, -cb0[23].y
    r6.y = (saturate((r6.yyyy)+(-(source[23].yyyy)))).y;
    // 176: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 177: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 178: mul r6.y, r6.y, cb0[23].z
    r6.y = ((r6.yyyy)*(source[23].zzzz)).y;
    // 179: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 180: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 181: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 182: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 183: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 184: mad r7.w, r0.w, l(2.000000), -r1.y
    r7.w = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).w;
    // 185: mad r6.yzw, r6.yyzw, r7.wwww, r1.yyyy
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r1.yyyy)).yzw;
    // 186: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 187: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 188: mul r7.w, r1.x, r1.x
    r7.w = ((r1.xxxx)*(r1.xxxx)).w;
    // 189: mul r8.w, r7.w, cb0[25].w
    r8.w = ((r7.wwww)*(source[25].wwww)).w;
    // 190: mad r1.x, -r7.w, cb0[25].w, r1.x
    r1.x = ((-(r7.wwww))*(source[25].wwww)+(r1.xxxx)).x;
    // 191: mad r1.x, r11.w, r1.x, r8.w
    r1.x = ((r11.wwww)*(r1.xxxx)+(r8.wwww)).x;
    // 192: mad r6.yzw, r2.wwww, r6.yyzw, -r7.xxyz
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 193: mad r6.yzw, r1.xxxx, r6.yyzw, r7.xxyz
    r6.yzw = ((r1.xxxx)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 194: sqrt r1.x, r5.w
    r1.x = (sqrt(r5.wwww)).x;
    // 195: mul r5.xyz, r5.xyzx, r1.xxxx
    r5.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 196: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 197: mad r6.yzw, -cb0[25].yyyy, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[25].yyyy))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 198: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 199: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 200: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 201: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 202: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 203: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 204: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, -r7.xyzx
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r7.xyzx))).xyz;
    // 205: mad r7.xyz, r11.zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 206: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 207: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 208: mad r7.xyz, cb0[20].yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((source[20].yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 209: dp3 r1.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 210: add r10.xyz, -r7.xyzx, r1.xxxx
    r10.xyz = ((-(r7.xyzx))+(r1.xxxx)).xyz;
    // 211: mad r7.xyz, cb0[20].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[20].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 212: mad r10.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 213: mad r12.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 214: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 215: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 216: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[9].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[9].xyzx)).xyz;
    // 217: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 218: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 219: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 220: mad r8.xyz, cb0[20].yyyy, r12.xyzx, r8.xyzx
    r8.xyz = ((source[20].yyyy)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 221: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 222: add r12.xyz, -r8.xyzx, r1.xxxx
    r12.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 223: mad r8.xyz, cb0[20].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[20].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 224: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 225: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 227: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 228: add r13.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r13.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 229: mad r13.xyz, r0.yyyy, r13.xyzx, cb0[10].xyzx
    r13.xyz = ((r0.yyyy)*(r13.xyzx)+(source[10].xyzx)).xyz;
    // 230: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 231: mul r0.xyz, r0.xyzx, cb0[22].zzzz
    r0.xyz = ((r0.xyzx)*(source[22].zzzz)).xyz;
    // 232: mul r13.xyz, r0.xyzx, r12.xyzx
    r13.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 233: dp3 r1.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 234: add r14.xyz, -r2.xyzx, r1.xxxx
    r14.xyz = ((-(r2.xyzx))+(r1.xxxx)).xyz;
    // 235: mad r2.yzw, cb0[20].yyyy, r14.xxyz, r2.xxyz
    r2.yzw = ((source[20].yyyy)*(r14.xxyz)+(r2.xxyz)).yzw;
    // 236: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 237: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 238: mad r2.yzw, cb0[20].zzzz, r14.xxyz, r2.yyzw
    r2.yzw = ((source[20].zzzz)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 239: dp3 r1.x, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 240: add r14.xyz, -r2.yzwy, r1.xxxx
    r14.xyz = ((-(r2.yzwy))+(r1.xxxx)).xyz;
    // 241: mul r14.xyz, r14.xyzx, cb0[22].wwww
    r14.xyz = ((r14.xyzx)*(source[22].wwww)).xyz;
    // 242: add r1.x, r11.y, r11.x
    r1.x = ((r11.yyyy)+(r11.xxxx)).x;
    // 243: add r1.x, r11.z, r1.x
    r1.x = ((r11.zzzz)+(r1.xxxx)).x;
    // 244: add_sat r1.x, r11.w, r1.x
    r1.x = (saturate((r11.wwww)+(r1.xxxx))).x;
    // 245: mad r2.yzw, r1.xxxx, r14.xxyz, r2.yyzw
    r2.yzw = ((r1.xxxx)*(r14.xxyz)+(r2.yyzw)).yzw;
    // 246: add r11.xyz, -r2.yzwy, r2.xxxx
    r11.xyz = ((-(r2.yzwy))+(r2.xxxx)).xyz;
    // 247: mad r2.xyz, r11.wwww, r11.xyzx, r2.yzwy
    r2.xyz = ((r11.wwww)*(r11.xyzx)+(r2.yzwy)).xyz;
    // 248: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 249: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 250: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 251: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 252: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 253: add r1.z, cb0[23].w, -cb0[24].x
    r1.z = ((source[23].wwww)+(-(source[24].xxxx))).z;
    // 254: mad r1.z, r11.w, r1.z, cb0[24].x
    r1.z = ((r11.wwww)*(r1.zzzz)+(source[24].xxxx)).z;
    // 255: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 256: mul r1.x, r1.x, r1.z
    r1.x = ((r1.xxxx)*(r1.zzzz)).x;
    // 257: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 258: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 259: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 260: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 261: div r1.z, cb0[24].y, r1.z
    r1.z = ((source[24].yyyy)/(r1.zzzz)).z;
    // 262: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 263: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 264: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 265: mul r1.z, r1.z, cb0[24].z
    r1.z = ((r1.zzzz)*(source[24].zzzz)).z;
    // 266: mad r0.xyz, r2.xyzx, r0.xyzx, -r13.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 267: mad r0.xyz, r1.zzzz, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 268: mad r0.xyz, r1.yyyy, r0.xyzx, -r12.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r12.xyzx))).xyz;
    // 269: mad r0.xyz, r1.xxxx, r0.xyzx, r12.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r12.xyzx)).xyz;
    // 270: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 271: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 272: mad r0.xyz, cb0[20].yyyy, r11.xyzx, r0.xyzx
    r0.xyz = ((source[20].yyyy)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 273: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 274: add r11.xyz, -r0.xyzx, r1.zzzz
    r11.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 275: mad r0.xyz, cb0[20].zzzz, r11.xyzx, r0.xyzx
    r0.xyz = ((source[20].zzzz)*(r11.xyzx)+(r0.xyzx)).xyz;
    // 276: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 277: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 278: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 279: mul r2.w, r2.w, cb0[24].w
    r2.w = ((r2.wwww)*(source[24].wwww)).w;
    // 280: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 281: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 282: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 283: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 284: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 285: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 286: add r6.x, -r2.w, cb0[2].x
    r6.x = ((-(r2.wwww))+(source[2].xxxx)).x;
    // 287: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 288: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 289: mul r10.y, cb0[2].y, cb0[12].y
    r10.y = ((source[2].yyyy)*(source[12].yyyy)).y;
    // 290: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 291: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 292: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 293: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 294: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 295: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 296: mul r10.xyz, r1.zzzz, r10.xyzx
    r10.xyz = ((r1.zzzz)*(r10.xyzx)).xyz;
    // 297: mul r1.z, r2.w, r10.w
    r1.z = ((r2.wwww)*(r10.wwww)).z;
    // 298: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 299: mad r0.xyz, r1.zzzz, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 300: mul r1.z, cb0[13].y, cb0[24].w
    r1.z = ((source[13].yyyy)*(source[24].wwww)).z;
    // 301: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 302: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 303: mul r10.y, r1.z, l(0.020000)
    r10.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 304: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 305: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 306: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 307: mad r3.xy, cb0[13].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[13].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 308: mul r2.w, cb0[13].x, l(0.001000)
    r2.w = ((source[13].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 309: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 310: mad r3.xy, r2.wwww, r3.xyxx, r10.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r10.xyxx)).xy;
    // 311: dp2 r2.w, cb0[14].xyxx, r3.xyxx
    r2.w = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 312: dp2 r3.y, cb0[15].xyxx, r3.xyxx
    r3.y = (dot((source[15].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 313: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 314: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 315: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 316: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 317: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 318: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 319: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 320: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 321: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 322: mul r10.xyz, r3.xyzx, cb0[13].zzzz
    r10.xyz = ((r3.xyzx)*(source[13].zzzz)).xyz;
    // 323: dp3 r1.z, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 324: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 325: mad r3.xyz, cb0[13].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[13].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 326: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 327: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 328: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 329: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 330: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 331: dp3 r4.x, r4.xyzx, r9.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 332: mul r4.z, r0.w, cb0[26].x
    r4.z = ((r0.wwww)*(source[26].xxxx)).z;
    // 333: mul r0.w, r11.w, r4.z
    r0.w = ((r11.wwww)*(r4.zzzz)).w;
    // 334: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 335: min r0.w, r0.w, cb0[26].x
    r0.w = (min(r0.wwww,source[26].xxxx)).w;
    // 336: add r1.x, -cb0[26].w, cb0[26].z
    r1.x = ((-(source[26].wwww))+(source[26].zzzz)).x;
    // 337: mad r1.x, cb0[26].y, r1.x, cb0[26].w
    r1.x = ((source[26].yyyy)*(r1.xxxx)+(source[26].wwww)).x;
    // 338: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 339: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 340: mad r1.x, r11.w, r1.x, l(1.000000)
    r1.x = ((r11.wwww)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 341: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 342: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 343: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 344: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 345: movc r4.y, r1.z, l(0), r0.w
    r4.y = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 346: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 347: add r0.w, -cb0[27].z, cb0[27].y
    r0.w = ((-(source[27].zzzz))+(source[27].yyyy)).w;
    // 348: mad r0.w, cb0[27].x, r0.w, cb0[27].z
    r0.w = ((source[27].xxxx)*(r0.wwww)+(source[27].zzzz)).w;
    // 349: mul r0.w, r0.w, r11.w
    r0.w = ((r0.wwww)*(r11.wwww)).w;
    // 350: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xzxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 351: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 352: add r0.w, -cb0[27].w, l(2.000000)
    r0.w = ((-(source[27].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 353: mad r0.w, r1.y, r0.w, cb0[27].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[27].wwww)).w;
    // 354: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 355: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 356: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 357: mul r1.xyz, r1.xyzx, cb0[28].xxxx
    r1.xyz = ((r1.xyzx)*(source[28].xxxx)).xyz;
    // 358: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 359: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 360: mul r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)*(r5.wwww)).xyz;
    // 361: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 362: mul r1.xyz, r1.xyzx, cb0[28].yyyy
    r1.xyz = ((r1.xyzx)*(source[28].yyyy)).xyz;
    // 363: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 364: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 365: mad r0.xyz, r6.yzwy, r0.xyzx, r1.xyzx
    r0.xyz = ((r6.yzwy)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 366: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 367: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 368: mul o0.xyz, r0.xyzx, cb0[29].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[29].xyzx)).xyz;
    // 369: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 370: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 371: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 372: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight16(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[21].y=(g_SourceCharacterTime.xxxx).x;
    source[21].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[21].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[22].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[22].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[25]=float4(input.lightColor,1.0);
    source[26].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[26].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[26].xxxx)) * 0xffffffffu)).w;
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
    // 31: mul r9.xyzw, r8.xyzw, cb0[15].xyzw
    r9.xyzw = ((r8.xyzw)*(source[15].xyzw)).xyzw;
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
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 45: mul r10.xy, r9.xyxx, cb0[16].xxxx
    r10.xy = ((r9.xyxx)*(source[16].xxxx)).xy;
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
    // 52: mad r9.xyz, cb0[16].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[16].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
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
    // 71: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 72: add r0.z, -cb0[17].y, cb0[17].x
    r0.z = ((-(source[17].yyyy))+(source[17].xxxx)).z;
    // 73: mad r0.z, r2.x, r0.z, cb0[17].y
    r0.z = ((r2.xxxx)*(r0.zzzz)+(source[17].yyyy)).z;
    // 74: add r1.x, -r0.z, cb0[17].z
    r1.x = ((-(r0.zzzz))+(source[17].zzzz)).x;
    // 75: mad r0.z, r2.y, r1.x, r0.z
    r0.z = ((r2.yyyy)*(r1.xxxx)+(r0.zzzz)).z;
    // 76: add r1.x, -r0.z, cb0[17].w
    r1.x = ((-(r0.zzzz))+(source[17].wwww)).x;
    // 77: mad r0.z, r2.z, r1.x, r0.z
    r0.z = ((r2.zzzz)*(r1.xxxx)+(r0.zzzz)).z;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 79: add r1.x, -r9.w, l(1.000000)
    r1.x = ((-(r9.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: add r1.y, -cb0[18].y, cb0[18].x
    r1.y = ((-(source[18].yyyy))+(source[18].xxxx)).y;
    // 81: mad r1.y, r2.x, r1.y, cb0[18].y
    r1.y = ((r2.xxxx)*(r1.yyyy)+(source[18].yyyy)).y;
    // 82: add r1.w, -r1.y, cb0[18].z
    r1.w = ((-(r1.yyyy))+(source[18].zzzz)).w;
    // 83: mad r1.y, r2.y, r1.w, r1.y
    r1.y = ((r2.yyyy)*(r1.wwww)+(r1.yyyy)).y;
    // 84: add r1.w, -r1.y, cb0[18].w
    r1.w = ((-(r1.yyyy))+(source[18].wwww)).w;
    // 85: mad r1.y, r2.z, r1.w, r1.y
    r1.y = ((r2.zzzz)*(r1.wwww)+(r1.yyyy)).y;
    // 86: lt r1.w, |r1.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 87: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 88: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 89: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 90: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 91: movc r1.x, r1.w, l(0), r1.x
    r1.x = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 92: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 93: mul r0.z, r0.z, r1.y
    r0.z = ((r0.zzzz)*(r1.yyyy)).z;
    // 94: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 95: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 96: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 97: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 98: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 99: dp2 r4.x, r4.xyxx, r4.xyxx
    r4.x = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 100: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 101: max r1.y, r1.y, r4.x
    r1.y = (max(r1.yyyy,r4.xxxx)).y;
    // 102: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 103: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 104: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 105: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 106: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 107: add r0.z, r0.z, |r1.y|
    r0.z = ((r0.zzzz)+(abs(r1.yyyy))).z;
    // 108: round_ni r0.z, r0.z
    r0.z = (floor(r0.zzzz)).z;
    // 109: sample_l_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, r0.z
    r0.xyz = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r0.zzzz).x)).xyzw).xyz;
    // 110: rcp r1.y, cb0[19].x
    r1.y = (1.0/(source[19].xxxx)).y;
    // 111: log r4.xyz, r0.xyzx
    r4.xyz = (log2(r0.xyzx)).xyz;
    // 112: mul r11.xyz, r4.xyzx, cb0[19].xxxx
    r11.xyz = ((r4.xyzx)*(source[19].xxxx)).xyz;
    // 113: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 114: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 115: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 116: mul r4.xyz, r1.yyyy, r4.xyzx
    r4.xyz = ((r1.yyyy)*(r4.xyzx)).xyz;
    // 117: mad r4.xyz, r11.xyzx, cb0[19].xxxx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(source[19].xxxx)+(r4.xyzx)).xyz;
    // 118: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 119: mul r0.xyz, r0.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 120: add r1.y, cb0[19].x, l(1.000000)
    r1.y = ((source[19].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 121: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 122: mad r4.xyz, v5.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 123: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 124: sqrt r1.y, r0.w
    r1.y = (sqrt(r0.wwww)).y;
    // 125: div r4.xyz, r4.xyzx, r1.yyyy
    r4.xyz = ((r4.xyzx)/(r1.yyyy)).xyz;
    // 126: dp3 r1.y, r4.xyzx, r6.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 127: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 128: lt r1.w, |r1.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 129: mul r4.w, |r1.y|, |r1.y|
    r4.w = ((abs(r1.yyyy))*(abs(r1.yyyy))).w;
    // 130: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 131: mul r1.y, |r1.y|, r4.w
    r1.y = ((abs(r1.yyyy))*(r4.wwww)).y;
    // 132: movc r1.y, r1.w, l(0), r1.y
    r1.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 133: add r1.w, r1.y, l(-0.027778)
    r1.w = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).w;
    // 134: mad r1.y, r1.y, r1.w, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.wwww)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 135: div r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)/(r0.wwww)).w;
    // 136: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 137: min r1.yw, r0.wwww, l(0.000000, 1.000000, 0.000000, 3.000000)
    r1.yw = (min(r0.wwww,float4(0.000000,1.000000,0.000000,3.000000))).yw;
    // 138: add r0.w, -r1.y, l(1.000000)
    r0.w = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 140: mul r11.xyz, r0.xyzx, r1.yyyy
    r11.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 141: mul r12.xyz, r11.xyzx, cb0[22].wwww
    r12.xyz = ((r11.xyzx)*(source[22].wwww)).xyz;
    // 142: dp3 r4.w, r10.xyzx, r10.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 143: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 144: div r10.xyz, r10.xyzx, r4.wwww
    r10.xyz = ((r10.xyzx)/(r4.wwww)).xyz;
    // 145: dp3 r4.w, r10.xyzx, r7.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 146: max r5.w, r4.w, l(0.000000)
    r5.w = (max(r4.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 147: min r6.w, r5.w, l(1.000000)
    r6.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r4.w, r4.w, l(0.500000), -r6.w
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r6.wwww))).w;
    // 150: mad r4.w, r1.x, r4.w, r6.w
    r4.w = ((r1.xxxx)*(r4.wwww)+(r6.wwww)).w;
    // 151: mad r0.w, -r1.x, r0.w, l(1.000000)
    r0.w = ((-(r1.xxxx))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 153: mul_sat r6.xy, r6.xzxx, cb0[19].wwww
    r6.xy = (saturate((r6.xzxx)*(source[19].wwww))).xy;
    // 154: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 155: add_sat r6.y, r6.y, -cb0[20].x
    r6.y = (saturate((r6.yyyy)+(-(source[20].xxxx)))).y;
    // 156: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 157: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 158: mul r6.y, r6.y, cb0[20].y
    r6.y = ((r6.yyyy)*(source[20].yyyy)).y;
    // 159: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 160: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 161: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 162: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 163: mul r6.y, r4.w, r6.y
    r6.y = ((r4.wwww)*(r6.yyyy)).y;
    // 164: mad r6.z, r0.w, l(2.000000), -r1.y
    r6.z = ((r0.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r1.yyyy))).z;
    // 165: mad r6.y, r6.y, r6.z, r1.y
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r1.yyyy)).y;
    // 166: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul_sat r6.y, r4.w, r6.y
    r6.y = (saturate((r4.wwww)*(r6.yyyy))).y;
    // 168: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 169: mul r1.x, r1.x, cb0[23].y
    r1.x = ((r1.xxxx)*(source[23].yyyy)).x;
    // 170: mad r6.y, cb0[23].x, r6.y, -r4.w
    r6.y = ((source[23].xxxx)*(r6.yyyy)+(-(r4.wwww))).y;
    // 171: mad r1.x, r1.x, r6.y, r4.w
    r1.x = ((r1.xxxx)*(r6.yyyy)+(r4.wwww)).x;
    // 172: sqrt r4.w, r6.w
    r4.w = (sqrt(r6.wwww)).w;
    // 173: mul r5.xyz, r5.xyzx, r4.wwww
    r5.xyz = ((r5.xyzx)*(r4.wwww)).xyz;
    // 174: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 175: mad r7.xyz, -cb0[22].wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[22].wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mad r7.xyz, r5.xyzx, r7.xyzx, r12.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 177: mul r11.xyz, cb0[3].xyzx, cb0[3].wwww
    r11.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 178: mad r12.xyz, cb0[4].wwww, cb0[4].xyzx, -r11.xyzx
    r12.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r11.xyzx))).xyz;
    // 179: mad r11.xyz, r2.xxxx, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 180: mad r12.xyz, cb0[5].wwww, cb0[5].xyzx, -r11.xyzx
    r12.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r11.xyzx))).xyz;
    // 181: mad r11.xyz, r2.yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 182: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, -r11.xyzx
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(-(r11.xyzx))).xyz;
    // 183: mad r11.xyz, r2.zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 184: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 186: mad r11.xyz, cb0[16].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 187: dp3 r1.x, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 188: add r12.xyz, -r11.xyzx, r1.xxxx
    r12.xyz = ((-(r11.xyzx))+(r1.xxxx)).xyz;
    // 189: mad r11.xyz, cb0[16].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[16].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 190: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mad r13.xyz, cb0[8].wwww, cb0[8].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[8].wwww)*(source[8].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 193: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 194: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 195: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 196: mad r8.xyz, cb0[16].yyyy, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].yyyy)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 197: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 198: add r13.xyz, -r8.xyzx, r1.xxxx
    r13.xyz = ((-(r8.xyzx))+(r1.xxxx)).xyz;
    // 199: mad r8.xyz, cb0[16].zzzz, r13.xyzx, r8.xyzx
    r8.xyz = ((source[16].zzzz)*(r13.xyzx)+(r8.xyzx)).xyz;
    // 200: mul r13.xyz, r8.xyzx, r11.xyzx
    r13.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 201: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: add r0.y, r1.z, l(1.000000)
    r0.y = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 203: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 204: add r14.xyz, -cb0[9].xyzx, cb0[10].xyzx
    r14.xyz = ((-(source[9].xyzx))+(source[10].xyzx)).xyz;
    // 205: mad r14.xyz, r0.yyyy, r14.xyzx, cb0[9].xyzx
    r14.xyz = ((r0.yyyy)*(r14.xyzx)+(source[9].xyzx)).xyz;
    // 206: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 207: mul r0.xyz, r0.xyzx, cb0[19].yyyy
    r0.xyz = ((r0.xyzx)*(source[19].yyyy)).xyz;
    // 208: mul r14.xyz, r0.xyzx, r13.xyzx
    r14.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 209: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 210: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 211: mad r9.xyz, cb0[16].yyyy, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].yyyy)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 212: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 213: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 214: mad r9.xyz, cb0[16].zzzz, r15.xyzx, r9.xyzx
    r9.xyz = ((source[16].zzzz)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 215: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: add r15.xyz, -r9.xyzx, r1.xxxx
    r15.xyz = ((-(r9.xyzx))+(r1.xxxx)).xyz;
    // 217: mul r15.xyz, r15.xyzx, cb0[19].zzzz
    r15.xyz = ((r15.xyzx)*(source[19].zzzz)).xyz;
    // 218: add r1.x, r2.y, r2.x
    r1.x = ((r2.yyyy)+(r2.xxxx)).x;
    // 219: add r1.x, r2.z, r1.x
    r1.x = ((r2.zzzz)+(r1.xxxx)).x;
    // 220: add_sat r1.x, r2.w, r1.x
    r1.x = (saturate((r2.wwww)+(r1.xxxx))).x;
    // 221: mad r2.xyz, r1.xxxx, r15.xyzx, r9.xyzx
    r2.xyz = ((r1.xxxx)*(r15.xyzx)+(r9.xyzx)).xyz;
    // 222: max r9.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r9.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 223: log r9.xyz, r9.xyzx
    r9.xyz = (log2(r9.xyzx)).xyz;
    // 224: mul r9.xyz, r9.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r9.xyz = ((r9.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 225: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 226: dp3 r1.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 227: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 228: mul r1.x, r1.x, cb0[20].z
    r1.x = ((r1.xxxx)*(source[20].zzzz)).x;
    // 229: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 230: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mad r1.z, -r1.x, r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 232: max r1.z, r1.z, l(0.001000)
    r1.z = (max(r1.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 233: div r1.z, cb0[20].w, r1.z
    r1.z = ((source[20].wwww)/(r1.zzzz)).z;
    // 234: mul r1.z, r1.z, r6.x
    r1.z = ((r1.zzzz)*(r6.xxxx)).z;
    // 235: mul r0.xyz, r0.xyzx, r1.zzzz
    r0.xyz = ((r0.xyzx)*(r1.zzzz)).xyz;
    // 236: add r1.z, -r1.x, l(1.000000)
    r1.z = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: mul r1.z, r1.z, cb0[21].x
    r1.z = ((r1.zzzz)*(source[21].xxxx)).z;
    // 238: mad r0.xyz, r2.xyzx, r0.xyzx, -r14.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r14.xyzx))).xyz;
    // 239: mad r0.xyz, r1.zzzz, r0.xyzx, r14.xyzx
    r0.xyz = ((r1.zzzz)*(r0.xyzx)+(r14.xyzx)).xyz;
    // 240: mad r0.xyz, r1.yyyy, r0.xyzx, -r13.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(-(r13.xyzx))).xyz;
    // 241: mad r0.xyz, r1.xxxx, r0.xyzx, r13.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)+(r13.xyzx)).xyz;
    // 242: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 243: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 244: mad r0.xyz, cb0[16].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 245: dp3 r1.z, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 246: add r6.xyz, -r0.xyzx, r1.zzzz
    r6.xyz = ((-(r0.xyzx))+(r1.zzzz)).xyz;
    // 247: mad r0.xyz, cb0[16].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[16].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 248: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 249: mul r1.z, cb0[2].z, l(1.500000)
    r1.z = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 250: add r2.w, -cb0[2].w, l(1.000000)
    r2.w = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r2.w, r2.w, cb0[21].y
    r2.w = ((r2.wwww)*(source[21].yyyy)).w;
    // 252: mul r2.w, r2.w, l(6.283185)
    r2.w = ((r2.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 253: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 254: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 255: mul r1.z, r1.z, r2.w
    r1.z = ((r1.zzzz)*(r2.wwww)).z;
    // 256: mad r1.z, r1.z, l(0.500000), cb0[2].z
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).z;
    // 257: frc r2.w, cb0[2].x
    r2.w = (frac(source[2].xxxx)).w;
    // 258: add r4.w, -r2.w, cb0[2].x
    r4.w = ((-(r2.wwww))+(source[2].xxxx)).w;
    // 259: mul r9.z, r4.w, l(0.125000)
    r9.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 260: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 261: mul r9.y, cb0[2].y, cb0[11].y
    r9.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 262: frc r4.w, v4.x
    r4.w = (frac(v4.xxxx)).w;
    // 263: mul r6.x, r4.w, l(0.125000)
    r6.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 264: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 265: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 266: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 267: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 268: mul r6.xyz, r1.zzzz, r9.xyzx
    r6.xyz = ((r1.zzzz)*(r9.xyzx)).xyz;
    // 269: mul r1.z, r2.w, r9.w
    r1.z = ((r2.wwww)*(r9.wwww)).z;
    // 270: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 271: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 272: mul r1.z, cb0[12].y, cb0[21].y
    r1.z = ((source[12].yyyy)*(source[21].yyyy)).z;
    // 273: mul r1.z, r1.z, l(0.628319)
    r1.z = ((r1.zzzz)*(float4(0.628319,0.628319,0.628319,0.628319))).z;
    // 274: sincos r1.z, null, r1.z
    r1.z = (sin(r1.zzzz)).z;
    // 275: mul r6.y, r1.z, l(0.020000)
    r6.y = ((r1.zzzz)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 276: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 277: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 278: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 279: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 280: mul r2.w, cb0[12].x, l(0.001000)
    r2.w = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 281: mov r6.x, l(0)
    r6.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 282: mad r3.xy, r2.wwww, r3.xyxx, r6.xyxx
    r3.xy = ((r2.wwww)*(r3.xyxx)+(r6.xyxx)).xy;
    // 283: dp2 r2.w, cb0[13].xyxx, r3.xyxx
    r2.w = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 284: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 285: frc r2.w, r2.w
    r2.w = (frac(r2.wwww)).w;
    // 286: mul r3.x, r2.w, l(0.125000)
    r3.x = ((r2.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 287: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 288: mul r2.w, r3.w, l(0.900000)
    r2.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 289: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r0.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r0.xyzx))).xyz;
    // 290: mad r3.xyz, r2.wwww, r3.xyzx, r0.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 291: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 292: mul r1.z, r1.z, l(0.500000)
    r1.z = ((r1.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 293: mul_sat r3.xyz, r3.xyzx, r1.zzzz
    r3.xyz = (saturate((r3.xyzx)*(r1.zzzz))).xyz;
    // 294: mul r6.xyz, r3.xyzx, cb0[12].zzzz
    r6.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 295: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 296: mul r1.z, r1.z, l(3.000000)
    r1.z = ((r1.zzzz)*(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 297: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r0.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 298: mad r0.xyz, r1.zzzz, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 299: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 300: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 301: mad r5.xyz, r11.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r11.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 302: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 303: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 304: mul r4.y, r0.w, cb0[23].z
    r4.y = ((r0.wwww)*(source[23].zzzz)).y;
    // 305: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t7.xyzw, s8, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 306: add r0.w, -cb0[23].w, l(2.000000)
    r0.w = ((-(source[23].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 307: mad r0.w, r1.y, r0.w, cb0[23].w
    r0.w = ((r1.yyyy)*(r0.wwww)+(source[23].wwww)).w;
    // 308: mul r1.xyz, r4.xyzx, r0.wwww
    r1.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 309: max r1.xyz, |r1.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r1.xyz = (max(abs(r1.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 310: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 311: mul r1.xyz, r1.xyzx, cb0[24].xxxx
    r1.xyz = ((r1.xyzx)*(source[24].xxxx)).xyz;
    // 312: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 313: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 314: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 315: min r1.xyz, r1.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 316: mul r1.xyz, r1.xyzx, cb0[24].yyyy
    r1.xyz = ((r1.xyzx)*(source[24].yyyy)).xyz;
    // 317: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 318: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 319: mad r0.xyz, r7.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 320: mul r1.xyz, r5.wwww, cb2[3].xyzx
    r1.xyz = ((r5.wwww)*(passValues[3].xyzx)).xyz;
    // 321: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 322: mul o0.xyz, r0.xyzx, cb0[25].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)).xyz;
    // 323: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 324: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 325: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 326: ret
    return output;
}

