#ifndef SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight108(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11].z=(g_SourceCharacterTime.xxxx).x;
    source[13].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[13].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=float4(input.lightColor,1.0);
    source[17].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
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
    // 9: mul r3.xy, r2.xyxx, cb0[10].xxxx
    r3.xy = ((r2.xyxx)*(source[10].xxxx)).xy;
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
    // 22: mul r4.xy, v4.xyxx, cb0[14].wwww
    r4.xy = ((v4.xyxx)*(source[14].wwww)).xy;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r4.xyxx, t2.yzwx, s5, l(0.000000)
    r1.w = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 24: add r1.w, r1.w, -cb0[15].x
    r1.w = ((r1.wwww)+(-(source[15].xxxx))).w;
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
    // 30: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[17].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[17].xxxx)) * 0xffffffffu)).w;
    // 31: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 32: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 33: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 34: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s0
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
    // 43: mul r1.w, r6.x, cb0[13].z
    r1.w = ((r6.xxxx)*(source[13].zzzz)).w;
    // 44: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 45: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 46: add_sat r1.w, r1.w, cb0[13].w
    r1.w = (saturate((r1.wwww)+(source[13].wwww))).w;
    // 47: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r8.xyz, r2.wwww, cb0[7].xyzx
    r8.xyz = ((r2.wwww)*(source[7].xyzx)).xyz;
    // 49: mul r2.w, r6.y, cb0[10].y
    r2.w = ((r6.yyyy)*(source[10].yyyy)).w;
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
    // 60: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 61: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 62: mad r6.xyw, cb0[12].xxxx, r7.xyxw, r6.xyxw
    r6.xyw = ((source[12].xxxx)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 63: dp3 r3.w, r6.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: add r7.xyw, -r6.xyxw, r3.wwww
    r7.xyw = ((-(r6.xyxw))+(r3.wwww)).xyw;
    // 65: mad r6.xyw, cb0[12].yyyy, r7.xyxw, r6.xyxw
    r6.xyw = ((source[12].yyyy)*(r7.xyxw)+(r6.xyxw)).xyw;
    // 66: mad r7.xyw, cb0[5].wwww, cb0[5].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r7.xyw = ((source[5].wwww)*(source[5].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 67: mad r9.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mul r7.xyw, r7.xyxw, r9.xyxz
    r7.xyw = ((r7.xyxw)*(r9.xyxz)).xyw;
    // 69: mul r6.xyw, r6.xyxw, r7.xyxw
    r6.xyw = ((r6.xyxw)*(r7.xyxw)).xyw;
    // 70: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 72: mad r3.xyz, cb0[12].xxxx, r9.xyzx, r3.xyzx
    r3.xyz = ((source[12].xxxx)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 73: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r9.xyz, -r3.xyzx, r3.wwww
    r9.xyz = ((-(r3.xyzx))+(r3.wwww)).xyz;
    // 75: mad r3.xyz, cb0[12].yyyy, r9.xyzx, r3.xyzx
    r3.xyz = ((source[12].yyyy)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 76: mul r9.xyz, r3.xyzx, r6.xywx
    r9.xyz = ((r3.xyzx)*(r6.xywx)).xyz;
    // 77: dp3 r3.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 78: mad r3.xyz, -r6.xywx, r3.xyzx, r3.wwww
    r3.xyz = ((-(r6.xywx))*(r3.xyzx)+(r3.wwww)).xyz;
    // 79: mad r3.xyz, cb0[12].xxxx, r3.xyzx, r9.xyzx
    r3.xyz = ((source[12].xxxx)*(r3.xyzx)+(r9.xyzx)).xyz;
    // 80: dp3 r3.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r6.xyw, -r3.xyxz, r3.wwww
    r6.xyw = ((-(r3.xyxz))+(r3.wwww)).xyw;
    // 82: mad r3.xyz, cb0[12].yyyy, r6.xywx, r3.xyzx
    r3.xyz = ((source[12].yyyy)*(r6.xywx)+(r3.xyzx)).xyz;
    // 83: mul r3.xyz, r7.xywx, r3.xyzx
    r3.xyz = ((r7.xywx)*(r3.xyzx)).xyz;
    // 84: mul r3.w, cb0[4].z, l(1.500000)
    r3.w = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 85: add r4.w, -cb0[4].w, l(1.000000)
    r4.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 86: mul r4.w, r4.w, cb0[11].z
    r4.w = ((r4.wwww)*(source[11].zzzz)).w;
    // 87: mul r4.w, r4.w, l(6.283185)
    r4.w = ((r4.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 88: sincos r4.w, null, r4.w
    r4.w = (sin(r4.wwww)).w;
    // 89: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 91: mad r3.w, r3.w, l(0.500000), cb0[4].z
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 92: frc r4.w, cb0[4].x
    r4.w = (frac(source[4].xxxx)).w;
    // 93: add r5.w, -r4.w, cb0[4].x
    r5.w = ((-(r4.wwww))+(source[4].xxxx)).w;
    // 94: mul r9.z, r5.w, l(0.125000)
    r9.z = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 95: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 96: mul r9.y, cb0[4].y, cb0[8].y
    r9.y = ((source[4].yyyy)*(source[8].yyyy)).y;
    // 97: frc r5.w, v4.x
    r5.w = (frac(v4.xxxx)).w;
    // 98: mul r6.x, r5.w, l(0.125000)
    r6.x = ((r5.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 99: mov r6.y, v4.y
    r6.y = (v4.yyyy).y;
    // 100: add r6.xy, r6.xyxx, r9.xyxx
    r6.xy = ((r6.xyxx)+(r9.xyxx)).xy;
    // 101: add r6.xy, r6.xyxx, r9.zwzz
    r6.xy = ((r6.xyxx)+(r9.zwzz)).xy;
    // 102: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.xyxx, t4.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 103: mul r6.xyw, r3.wwww, r9.xyxz
    r6.xyw = ((r3.wwww)*(r9.xyxz)).xyw;
    // 104: mul r3.w, r4.w, r9.w
    r3.w = ((r4.wwww)*(r9.wwww)).w;
    // 105: mad r6.xyw, r6.xyxw, l(2.000000, 2.000000, 0.000000, 2.000000), -r3.xyxz
    r6.xyw = ((r6.xyxw)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r3.xyxz))).xyw;
    // 106: mad r3.xyz, r3.wwww, r6.xywx, r3.xyzx
    r3.xyz = ((r3.wwww)*(r6.xywx)+(r3.xyzx)).xyz;
    // 107: mul r6.xyw, r3.xyxz, r8.xyxz
    r6.xyw = ((r3.xyxz)*(r8.xyxz)).xyw;
    // 108: mad r3.xyz, -r8.xyzx, r3.xyzx, r3.xyzx
    r3.xyz = ((-(r8.xyzx))*(r3.xyzx)+(r3.xyzx)).xyz;
    // 109: mad r3.xyz, r1.wwww, r3.xyzx, r6.xywx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xywx)).xyz;
    // 110: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 111: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 112: mov_sat r1.w, cb0[14].z
    r1.w = (saturate(source[14].zzzz)).w;
    // 113: mul_sat r2.w, r2.w, cb2[3].w
    r2.w = (saturate((r2.wwww)*(passValues[3].wwww))).w;
    // 114: mul r3.w, r6.z, cb0[15].y
    r3.w = ((r6.zzzz)*(source[15].yyyy)).w;
    // 115: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 116: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: movc r3.w, r7.z, l(0), r3.w
    r3.w = ((asuint(r7.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 118: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 119: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 121: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 122: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 123: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 124: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 125: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 126: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 127: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 129: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 130: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 133: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 134: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mad r5.xyz, -r3.xyzx, r2.wwww, r3.xyzx
    r5.xyz = ((-(r3.xyzx))*(r2.wwww)+(r3.xyzx)).xyz;
    // 136: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 137: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 138: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 139: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 140: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 142: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 143: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 144: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 145: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 146: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 147: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 148: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 149: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 150: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 151: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 152: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r3.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r3.xxyz)).yzw;
    // 153: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 154: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 156: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 157: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 158: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 159: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 160: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 162: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 163: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 164: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 165: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 167: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 168: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 169: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 170: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 171: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 172: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 173: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 174: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 175: mul_sat r6.xyz, cb0[9].xyzx, cb0[9].wwww
    r6.xyz = (saturate((source[9].xyzx)*(source[9].wwww))).xyz;
    // 176: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 177: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 178: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 179: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 180: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 181: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 182: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 183: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 184: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 185: mul r0.x, r0.x, cb0[15].z
    r0.x = ((r0.xxxx)*(source[15].zzzz)).x;
    // 186: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 187: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 188: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 190: mad r0.xyz, r5.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 191: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 192: mul o0.xyz, r0.xyzx, cb0[16].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)).xyz;
    // 193: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 194: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 195: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 196: ret
    return output;
}

// source.character.monster-fd6df5a0ab9a.v1 / source program 6d53b407f618b947baf08379872cb1a7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight109(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[11]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[12]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[15].w=(g_SourceCharacterTime.xxxx).x;
    source[16].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[16].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[19]=float4(input.lightColor,1.0);
    source[20].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: add r0.xyzw, v6.yzxy, cb0[0].yzxy
    r0.xyzw = ((v6.yzxy)+(source[0].yzxy)).xyzw;
    // 2: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[20].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[20].xxxx)) * 0xffffffffu)).x;
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
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s0
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
    // 20: mul r3.xyz, r1.wwww, v3.xyzx
    r3.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v2.xyxx, t1.wxyz, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 22: mov_sat r4.x, r4.x
    r4.x = (saturate(r4.xxxx)).x;
    // 23: add r2.w, r4.x, l(-0.333300)
    r2.w = ((r4.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 24: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 25: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v2.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 27: add r6.xyzw, -cb0[4].xyzw, cb0[5].xyzw
    r6.xyzw = ((-(source[4].xyzw))+(source[5].xyzw)).xyzw;
    // 28: mad r6.xyzw, r5.xxxx, r6.xyzw, cb0[4].xyzw
    r6.xyzw = ((r5.xxxx)*(r6.xyzw)+(source[4].xyzw)).xyzw;
    // 29: add r7.xyzw, -r6.xyzw, cb0[6].xyzw
    r7.xyzw = ((-(r6.xyzw))+(source[6].xyzw)).xyzw;
    // 30: mad r6.xyzw, r5.yyyy, r7.xyzw, r6.xyzw
    r6.xyzw = ((r5.yyyy)*(r7.xyzw)+(r6.xyzw)).xyzw;
    // 31: add r7.xyzw, -r6.xyzw, cb0[7].xyzw
    r7.xyzw = ((-(r6.xyzw))+(source[7].xyzw)).xyzw;
    // 32: mad r5.xyzw, r5.zzzz, r7.xyzw, r6.xyzw
    r5.xyzw = ((r5.zzzz)*(r7.xyzw)+(r6.xyzw)).xyzw;
    // 33: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 34: mul r5.xyz, r5.xyzx, cb0[16].wwww
    r5.xyz = ((r5.xyzx)*(source[16].wwww)).xyz;
    // 35: mul r6.xyz, r4.yzwy, r5.xyzx
    r6.xyz = ((r4.yzwy)*(r5.xyzx)).xyz;
    // 36: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: mad r4.xyz, -r4.yzwy, r5.xyzx, r2.wwww
    r4.xyz = ((-(r4.yzwy))*(r5.xyzx)+(r2.wwww)).xyz;
    // 38: mad r5.xyz, cb0[14].xxxx, r4.xyzx, r6.xyzx
    r5.xyz = ((source[14].xxxx)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 39: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 41: mad r5.xyz, cb0[14].yyyy, r7.xyzx, r5.xyzx
    r5.xyz = ((source[14].yyyy)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 42: mad r7.xyz, cb0[2].wwww, cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[2].wwww)*(source[2].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: mad r8.xyz, cb0[3].wwww, cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[3].wwww)*(source[3].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 44: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 45: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 46: mul r2.w, cb0[8].z, l(1.500000)
    r2.w = ((source[8].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 47: add r3.w, -cb0[8].w, l(1.000000)
    r3.w = ((-(source[8].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r3.w, r3.w, cb0[15].w
    r3.w = ((r3.wwww)*(source[15].wwww)).w;
    // 49: mul r3.w, r3.w, l(6.283185)
    r3.w = ((r3.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 50: sincos r3.w, null, r3.w
    r3.w = (sin(r3.wwww)).w;
    // 51: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 53: mad r2.w, r2.w, l(0.500000), cb0[8].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).w;
    // 54: frc r3.w, cb0[8].x
    r3.w = (frac(source[8].xxxx)).w;
    // 55: add r4.w, -r3.w, cb0[8].x
    r4.w = ((-(r3.wwww))+(source[8].xxxx)).w;
    // 56: mul r7.z, r4.w, l(0.125000)
    r7.z = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 57: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 58: mul r7.y, cb0[8].y, cb0[9].y
    r7.y = ((source[8].yyyy)*(source[9].yyyy)).y;
    // 59: frc r4.w, v2.x
    r4.w = (frac(v2.xxxx)).w;
    // 60: mul r8.x, r4.w, l(0.125000)
    r8.x = ((r4.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 61: mov r8.y, v2.y
    r8.y = (v2.yyyy).y;
    // 62: add r7.xy, r7.xyxx, r8.xyxx
    r7.xy = ((r7.xyxx)+(r8.xyxx)).xy;
    // 63: add r7.xy, r7.xyxx, r7.zwzz
    r7.xy = ((r7.xyxx)+(r7.zwzz)).xy;
    // 64: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 65: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 66: mul r2.w, r3.w, r7.w
    r2.w = ((r3.wwww)*(r7.wwww)).w;
    // 67: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 68: mad r5.xyz, r2.wwww, r7.xyzx, r5.xyzx
    r5.xyz = ((r2.wwww)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 69: mul r2.w, cb0[10].y, cb0[15].w
    r2.w = ((source[10].yyyy)*(source[15].wwww)).w;
    // 70: mul r2.w, r2.w, l(0.628319)
    r2.w = ((r2.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 71: sincos r2.w, null, r2.w
    r2.w = (sin(r2.wwww)).w;
    // 72: mul r7.y, r2.w, l(0.020000)
    r7.y = ((r2.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 73: add r0.xyzw, r0.xyzw, -cb0[1].yzxy
    r0.xyzw = ((r0.xyzw)+(-(source[1].yzxy))).xyzw;
    // 74: add r0.xy, -r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 75: add r0.xy, -r0.zwzz, r0.xyxx
    r0.xy = ((-(r0.zwzz))+(r0.xyxx)).xy;
    // 76: mad r0.xy, cb0[10].wwww, r0.xyxx, r0.zwzz
    r0.xy = ((source[10].wwww)*(r0.xyxx)+(r0.zwzz)).xy;
    // 77: mul r0.z, cb0[10].x, l(0.001000)
    r0.z = ((source[10].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 78: mov r7.x, l(0)
    r7.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 79: mad r0.xy, r0.zzzz, r0.xyxx, r7.xyxx
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(r7.xyxx)).xy;
    // 80: dp2 r0.z, cb0[11].xyxx, r0.xyxx
    r0.z = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 81: dp2 r0.y, cb0[12].xyxx, r0.xyxx
    r0.y = (dot((source[12].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 82: frc r0.z, r0.z
    r0.z = (frac(r0.zzzz)).z;
    // 83: mul r0.x, r0.z, l(0.125000)
    r0.x = ((r0.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t3.xyzw, s4, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 85: mul r0.w, r0.w, l(0.900000)
    r0.w = ((r0.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 86: mad r0.xyz, r0.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r5.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 88: add r0.w, r2.w, l(1.000000)
    r0.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 90: mul_sat r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = (saturate((r0.xyzx)*(r0.wwww))).xyz;
    // 91: mul r7.xyz, r0.xyzx, cb0[10].zzzz
    r7.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 92: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 94: mad r0.xyz, cb0[10].zzzz, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[10].zzzz)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 95: mad r0.xyz, r0.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 97: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 98: dp2 r0.w, r5.xyxx, r5.xyxx
    r0.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 99: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 101: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 102: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 103: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 104: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 105: div r7.xyz, r5.xyzx, r0.wwww
    r7.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 106: dp3_sat r0.w, r7.xyzx, r3.xyzx
    r0.w = (saturate(dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx)).w;
    // 107: mul r8.xyz, r1.xyzx, r0.wwww
    r8.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 108: mad r4.xyz, cb0[17].yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((source[17].yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 109: mul r4.xyz, r4.xyzx, cb0[17].zzzz
    r4.xyz = ((r4.xyzx)*(source[17].zzzz)).xyz;
    // 110: mad r6.xyz, v3.xyzx, r1.wwww, r2.xyzx
    r6.xyz = ((v3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 111: dp3 r0.w, r6.xyzx, r6.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 112: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 113: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 114: dp3 r1.w, r6.xyzx, r7.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 115: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 116: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 117: mul r1.w, r1.w, cb0[17].w
    r1.w = ((r1.wwww)*(source[17].wwww)).w;
    // 118: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 119: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 121: mad r0.w, r0.w, r0.w, l(0.100000)
    r0.w = ((r0.wwww)*(r0.wwww)+(float4(0.100000,0.100000,0.100000,0.100000))).w;
    // 122: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: div r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)/(r0.wwww)).w;
    // 124: mul r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)*(r0.wwww)).xyz;
    // 125: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 126: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 127: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 128: mad r0.xyz, r0.xyzx, r8.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 129: mul r1.xyz, cb0[13].xyzx, cb0[18].xxxx
    r1.xyz = ((source[13].xyzx)*(source[18].xxxx)).xyz;
    // 130: add r0.w, -|r2.z|, l(1.000000)
    r0.w = ((-(abs(r2.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 132: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 134: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 135: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 136: mul r0.w, r0.w, cb0[18].y
    r0.w = ((r0.wwww)*(source[18].yyyy)).w;
    // 137: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 138: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 139: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 140: mov_sat r0.w, -r3.z
    r0.w = (saturate(-(r3.zzzz))).w;
    // 141: mad r2.xyz, r0.wwww, r1.xyzx, -r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)+(-(r1.xyzx))).xyz;
    // 142: mad r1.xyz, cb0[13].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[13].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 143: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 144: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 145: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 146: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 147: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 148: mul o0.xyz, r0.xyzx, cb0[19].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)).xyz;
    // 149: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 150: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 151: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 152: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-01-head-st-mi-fx-dead.v1 / source program 9ea28633266d1d4c93e87f351c3bac06
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight110(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13].x=(g_SourceCharacterTime.xxxx).x;
    source[13].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[13].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[14].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[14].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[14].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[18]=float4(input.lightColor,1.0);
    source[19].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
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
    // 30: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: lt r6.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 33: log r5.xyz, |r5.xzyx|
    r5.xyz = (log2(abs(r5.xzyx))).xyz;
    // 34: mul r1.w, r5.x, cb0[14].w
    r1.w = ((r5.xxxx)*(source[14].wwww)).w;
    // 35: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 36: movc r1.w, r6.x, l(0), r1.w
    r1.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 37: add_sat r1.w, r1.w, cb0[15].x
    r1.w = (saturate((r1.wwww)+(source[15].xxxx))).w;
    // 38: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r7.xyz, r2.wwww, cb0[8].xyzx
    r7.xyz = ((r2.wwww)*(source[8].xyzx)).xyz;
    // 40: add r2.w, r1.w, l(-1.000000)
    r2.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 41: mad r2.w, cb0[15].z, r2.w, l(1.000000)
    r2.w = ((source[15].zzzz)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r8.xyz, cb0[4].xyzx, cb0[4].wwww
    r8.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
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
    // 53: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 54: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 55: mad r5.xyw, cb0[12].yyyy, r6.xyxw, r5.xyxw
    r5.xyw = ((source[12].yyyy)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 56: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 58: mad r5.xyw, cb0[12].zzzz, r6.xyxw, r5.xyxw
    r5.xyw = ((source[12].zzzz)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 59: mad r6.xyw, cb0[6].wwww, cb0[6].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[6].wwww)*(source[6].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 60: mad r8.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 62: mul r5.xyw, r5.xyxw, r6.xyxw
    r5.xyw = ((r5.xyxw)*(r6.xyxw)).xyw;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 66: mad r8.xyz, cb0[12].yyyy, r9.xyzx, r8.xyzx
    r8.xyz = ((source[12].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 67: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 69: mad r8.xyz, cb0[12].zzzz, r9.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 70: mul r9.xyz, r5.xywx, r8.xyzx
    r9.xyz = ((r5.xywx)*(r8.xyzx)).xyz;
    // 71: dp3 r4.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: mad r5.xyw, -r5.xyxw, r8.xyxz, r4.wwww
    r5.xyw = ((-(r5.xyxw))*(r8.xyxz)+(r4.wwww)).xyw;
    // 73: mad r5.xyw, cb0[12].yyyy, r5.xyxw, r9.xyxz
    r5.xyw = ((source[12].yyyy)*(r5.xyxw)+(r9.xyxz)).xyw;
    // 74: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: add r8.xyz, -r5.xywx, r4.wwww
    r8.xyz = ((-(r5.xywx))+(r4.wwww)).xyz;
    // 76: mad r5.xyw, cb0[12].zzzz, r8.xyxz, r5.xyxw
    r5.xyw = ((source[12].zzzz)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 77: mul r5.xyw, r6.xyxw, r5.xyxw
    r5.xyw = ((r6.xyxw)*(r5.xyxw)).xyw;
    // 78: mul r4.w, cb0[5].z, l(1.500000)
    r4.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 79: add r6.x, -cb0[5].w, l(1.000000)
    r6.x = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mul r6.x, r6.x, cb0[13].x
    r6.x = ((r6.xxxx)*(source[13].xxxx)).x;
    // 81: mul r6.x, r6.x, l(6.283185)
    r6.x = ((r6.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 82: sincos r6.x, null, r6.x
    r6.x = (sin(r6.xxxx)).x;
    // 83: add r6.x, r6.x, l(1.000000)
    r6.x = ((r6.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 85: mad r4.w, r4.w, l(0.500000), cb0[5].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 86: frc r6.x, cb0[5].x
    r6.x = (frac(source[5].xxxx)).x;
    // 87: add r6.y, -r6.x, cb0[5].x
    r6.y = ((-(r6.xxxx))+(source[5].xxxx)).y;
    // 88: mul r9.z, r6.y, l(0.125000)
    r9.z = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 89: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 90: mul r9.y, cb0[5].y, cb0[9].y
    r9.y = ((source[5].yyyy)*(source[9].yyyy)).y;
    // 91: frc r6.y, v4.x
    r6.y = (frac(v4.xxxx)).y;
    // 92: mul r8.x, r6.y, l(0.125000)
    r8.x = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 93: mov r8.y, v4.y
    r8.y = (v4.yyyy).y;
    // 94: add r6.yw, r8.xxxy, r9.xxxy
    r6.yw = ((r8.xxxy)+(r9.xxxy)).yw;
    // 95: add r6.yw, r6.yyyw, r9.zzzw
    r6.yw = ((r6.yyyw)+(r9.zzzw)).yw;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.ywyy, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 97: mul r8.xyz, r4.wwww, r9.xyzx
    r8.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 98: mul r4.w, r6.x, r9.w
    r4.w = ((r6.xxxx)*(r9.wwww)).w;
    // 99: mad r6.xyw, r8.xyxz, l(2.000000, 2.000000, 0.000000, 2.000000), -r5.xyxw
    r6.xyw = ((r8.xyxz)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r5.xyxw))).xyw;
    // 100: mad r5.xyw, r4.wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((r4.wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 101: mul r6.xyw, r2.wwww, r5.xyxw
    r6.xyw = ((r2.wwww)*(r5.xyxw)).xyw;
    // 102: mul r6.xyw, r6.xyxw, r7.xyxz
    r6.xyw = ((r6.xyxw)*(r7.xyxz)).xyw;
    // 103: mad r5.xyw, r2.wwww, r5.xyxw, -r6.xyxw
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(-(r6.xyxw))).xyw;
    // 104: mad r5.xyw, r1.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 105: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 106: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 107: mov_sat r1.w, cb0[15].w
    r1.w = (saturate(source[15].wwww)).w;
    // 108: mul_sat r2.w, r3.w, cb2[3].w
    r2.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 109: mul r3.w, r5.z, cb0[16].w
    r3.w = ((r5.zzzz)*(source[16].wwww)).w;
    // 110: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 111: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: movc r3.w, r6.z, l(0), r3.w
    r3.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 113: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 114: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 116: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 117: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 118: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 119: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 120: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 121: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 122: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 124: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 125: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 128: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 129: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mad r5.xyz, -r4.xyzx, r2.wwww, r4.xyzx
    r5.xyz = ((-(r4.xyzx))*(r2.wwww)+(r4.xyzx)).xyz;
    // 131: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 132: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 133: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 134: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 135: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 137: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 138: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 139: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 141: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 142: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 143: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 144: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 145: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 146: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r4.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r4.xxyz)).yzw;
    // 148: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 149: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 151: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 152: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 153: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 155: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 157: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 158: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 159: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 160: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 162: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 163: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 164: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 165: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 166: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 167: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 168: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 169: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 170: mul_sat r6.xyz, cb0[10].xyzx, cb0[10].wwww
    r6.xyz = (saturate((source[10].xyzx)*(source[10].wwww))).xyz;
    // 171: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 172: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 173: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 174: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 175: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 176: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 177: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 178: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 179: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 180: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 181: mul r0.xyz, r4.xyzx, r0.xxxx
    r0.xyz = ((r4.xyzx)*(r0.xxxx)).xyz;
    // 182: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 183: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 185: mad r0.xyz, r5.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 186: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 187: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 188: mul r0.xy, v4.xyxx, cb0[16].xxxx
    r0.xy = ((v4.xyxx)*(source[16].xxxx)).xy;
    // 189: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 190: add r0.x, r0.x, -cb0[16].y
    r0.x = ((r0.xxxx)+(-(source[16].yyyy))).x;
    // 191: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 192: mul_sat r0.x, r0.x, r8.w
    r0.x = (saturate((r0.xxxx)*(r8.wwww))).x;
    // 193: mul_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)*(source[16].zzzz))).x;
    // 194: mul o0.w, r0.x, cb0[2].x
    output.targets[0].w = ((r0.xxxx)*(source[2].xxxx)).w;
    // 195: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 196: ret
    return output;
}

// source.character.guardianknight-sk-ddk-drr-01-neck-st-mi-fx-dead.v1 / source program 9ea28633266d1d4c93e87f351c3bac06
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight111(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13].x=(g_SourceCharacterTime.xxxx).x;
    source[13].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[13].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[14].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[14].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[14].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[18]=float4(input.lightColor,1.0);
    source[19].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
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
    // 30: add r4.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: lt r6.xyz, |r5.xzyx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (asfloat((uint4)((abs(r5.xzyx))<(float4(0.000001,0.000001,0.000001,0.000000))) * 0xffffffffu)).xyz;
    // 33: log r5.xyz, |r5.xzyx|
    r5.xyz = (log2(abs(r5.xzyx))).xyz;
    // 34: mul r1.w, r5.x, cb0[14].w
    r1.w = ((r5.xxxx)*(source[14].wwww)).w;
    // 35: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 36: movc r1.w, r6.x, l(0), r1.w
    r1.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 37: add_sat r1.w, r1.w, cb0[15].x
    r1.w = (saturate((r1.wwww)+(source[15].xxxx))).w;
    // 38: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul r7.xyz, r2.wwww, cb0[8].xyzx
    r7.xyz = ((r2.wwww)*(source[8].xyzx)).xyz;
    // 40: add r2.w, r1.w, l(-1.000000)
    r2.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 41: mad r2.w, cb0[15].z, r2.w, l(1.000000)
    r2.w = ((source[15].zzzz)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mul r8.xyz, cb0[4].xyzx, cb0[4].wwww
    r8.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
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
    // 53: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 54: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 55: mad r5.xyw, cb0[12].yyyy, r6.xyxw, r5.xyxw
    r5.xyw = ((source[12].yyyy)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 56: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r6.xyw, -r5.xyxw, r4.wwww
    r6.xyw = ((-(r5.xyxw))+(r4.wwww)).xyw;
    // 58: mad r5.xyw, cb0[12].zzzz, r6.xyxw, r5.xyxw
    r5.xyw = ((source[12].zzzz)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 59: mad r6.xyw, cb0[6].wwww, cb0[6].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r6.xyw = ((source[6].wwww)*(source[6].xyxz)+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 60: mad r8.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r6.xyw, r6.xyxw, r8.xyxz
    r6.xyw = ((r6.xyxw)*(r8.xyxz)).xyw;
    // 62: mul r5.xyw, r5.xyxw, r6.xyxw
    r5.xyw = ((r5.xyxw)*(r6.xyxw)).xyw;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 64: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 66: mad r8.xyz, cb0[12].yyyy, r9.xyzx, r8.xyzx
    r8.xyz = ((source[12].yyyy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 67: dp3 r4.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: add r9.xyz, -r8.xyzx, r4.wwww
    r9.xyz = ((-(r8.xyzx))+(r4.wwww)).xyz;
    // 69: mad r8.xyz, cb0[12].zzzz, r9.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 70: mul r9.xyz, r5.xywx, r8.xyzx
    r9.xyz = ((r5.xywx)*(r8.xyzx)).xyz;
    // 71: dp3 r4.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 72: mad r5.xyw, -r5.xyxw, r8.xyxz, r4.wwww
    r5.xyw = ((-(r5.xyxw))*(r8.xyxz)+(r4.wwww)).xyw;
    // 73: mad r5.xyw, cb0[12].yyyy, r5.xyxw, r9.xyxz
    r5.xyw = ((source[12].yyyy)*(r5.xyxw)+(r9.xyxz)).xyw;
    // 74: dp3 r4.w, r5.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r5.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: add r8.xyz, -r5.xywx, r4.wwww
    r8.xyz = ((-(r5.xywx))+(r4.wwww)).xyz;
    // 76: mad r5.xyw, cb0[12].zzzz, r8.xyxz, r5.xyxw
    r5.xyw = ((source[12].zzzz)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 77: mul r5.xyw, r6.xyxw, r5.xyxw
    r5.xyw = ((r6.xyxw)*(r5.xyxw)).xyw;
    // 78: mul r4.w, cb0[5].z, l(1.500000)
    r4.w = ((source[5].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 79: add r6.x, -cb0[5].w, l(1.000000)
    r6.x = ((-(source[5].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 80: mul r6.x, r6.x, cb0[13].x
    r6.x = ((r6.xxxx)*(source[13].xxxx)).x;
    // 81: mul r6.x, r6.x, l(6.283185)
    r6.x = ((r6.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 82: sincos r6.x, null, r6.x
    r6.x = (sin(r6.xxxx)).x;
    // 83: add r6.x, r6.x, l(1.000000)
    r6.x = ((r6.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 84: mul r4.w, r4.w, r6.x
    r4.w = ((r4.wwww)*(r6.xxxx)).w;
    // 85: mad r4.w, r4.w, l(0.500000), cb0[5].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].zzzz)).w;
    // 86: frc r6.x, cb0[5].x
    r6.x = (frac(source[5].xxxx)).x;
    // 87: add r6.y, -r6.x, cb0[5].x
    r6.y = ((-(r6.xxxx))+(source[5].xxxx)).y;
    // 88: mul r9.z, r6.y, l(0.125000)
    r9.z = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 89: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 90: mul r9.y, cb0[5].y, cb0[9].y
    r9.y = ((source[5].yyyy)*(source[9].yyyy)).y;
    // 91: frc r6.y, v4.x
    r6.y = (frac(v4.xxxx)).y;
    // 92: mul r8.x, r6.y, l(0.125000)
    r8.x = ((r6.yyyy)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 93: mov r8.y, v4.y
    r8.y = (v4.yyyy).y;
    // 94: add r6.yw, r8.xxxy, r9.xxxy
    r6.yw = ((r8.xxxy)+(r9.xxxy)).yw;
    // 95: add r6.yw, r6.yyyw, r9.zzzw
    r6.yw = ((r6.yyyw)+(r9.zzzw)).yw;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r6.ywyy, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 97: mul r8.xyz, r4.wwww, r9.xyzx
    r8.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 98: mul r4.w, r6.x, r9.w
    r4.w = ((r6.xxxx)*(r9.wwww)).w;
    // 99: mad r6.xyw, r8.xyxz, l(2.000000, 2.000000, 0.000000, 2.000000), -r5.xyxw
    r6.xyw = ((r8.xyxz)*(float4(2.000000,2.000000,0.000000,2.000000))+(-(r5.xyxw))).xyw;
    // 100: mad r5.xyw, r4.wwww, r6.xyxw, r5.xyxw
    r5.xyw = ((r4.wwww)*(r6.xyxw)+(r5.xyxw)).xyw;
    // 101: mul r6.xyw, r2.wwww, r5.xyxw
    r6.xyw = ((r2.wwww)*(r5.xyxw)).xyw;
    // 102: mul r6.xyw, r6.xyxw, r7.xyxz
    r6.xyw = ((r6.xyxw)*(r7.xyxz)).xyw;
    // 103: mad r5.xyw, r2.wwww, r5.xyxw, -r6.xyxw
    r5.xyw = ((r2.wwww)*(r5.xyxw)+(-(r6.xyxw))).xyw;
    // 104: mad r5.xyw, r1.wwww, r5.xyxw, r6.xyxw
    r5.xyw = ((r1.wwww)*(r5.xyxw)+(r6.xyxw)).xyw;
    // 105: mul r4.xyz, r4.xyzx, r5.xywx
    r4.xyz = ((r4.xyzx)*(r5.xywx)).xyz;
    // 106: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 107: mov_sat r1.w, cb0[15].w
    r1.w = (saturate(source[15].wwww)).w;
    // 108: mul_sat r2.w, r3.w, cb2[3].w
    r2.w = (saturate((r3.wwww)*(passValues[3].wwww))).w;
    // 109: mul r3.w, r5.z, cb0[16].w
    r3.w = ((r5.zzzz)*(source[16].wwww)).w;
    // 110: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 111: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: movc r3.w, r6.z, l(0), r3.w
    r3.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 113: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 114: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: mad r5.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 116: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 117: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 118: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 119: dp3_sat r4.w, r2.xyzx, r5.xyzx
    r4.w = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 120: dp3 r5.w, r2.xyzx, r0.xyzx
    r5.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 121: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 122: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: dp3_sat r6.x, r2.xyzx, r1.xyzx
    r6.x = (saturate(dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 124: dp3_sat r5.x, r0.xyzx, r5.xyzx
    r5.x = (saturate(dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 125: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add r5.x, r5.x, l(1.000000)
    r5.x = ((r5.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 128: add r0.w, -r0.w, r5.x
    r0.w = ((-(r0.wwww))+(r5.xxxx)).w;
    // 129: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mad r5.xyz, -r4.xyzx, r2.wwww, r4.xyzx
    r5.xyz = ((-(r4.xyzx))*(r2.wwww)+(r4.xyzx)).xyz;
    // 131: mul r5.xyz, r5.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 132: mul r6.y, r3.w, r3.w
    r6.y = ((r3.wwww)*(r3.wwww)).y;
    // 133: mul r6.z, r6.y, r6.y
    r6.z = ((r6.yyyy)*(r6.yyyy)).z;
    // 134: mad r6.w, r4.w, r6.z, -r4.w
    r6.w = ((r4.wwww)*(r6.zzzz)+(-(r4.wwww))).w;
    // 135: mad r4.w, r6.w, r4.w, l(1.000000)
    r4.w = ((r6.wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 137: mul r4.w, r4.w, l(3.141593)
    r4.w = ((r4.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 138: div r4.w, r6.z, r4.w
    r4.w = ((r6.zzzz)/(r4.wwww)).w;
    // 139: mad r6.z, -r3.w, r3.w, l(1.000000)
    r6.z = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: mad r6.w, r5.w, r6.z, r6.y
    r6.w = ((r5.wwww)*(r6.zzzz)+(r6.yyyy)).w;
    // 141: mad r6.y, r6.x, r6.z, r6.y
    r6.y = ((r6.xxxx)*(r6.zzzz)+(r6.yyyy)).y;
    // 142: mul r5.w, r5.w, r6.y
    r5.w = ((r5.wwww)*(r6.yyyy)).w;
    // 143: mad r5.w, r6.x, r6.w, r5.w
    r5.w = ((r6.xxxx)*(r6.wwww)+(r5.wwww)).w;
    // 144: rcp r5.w, r5.w
    r5.w = (1.0/(r5.wwww)).w;
    // 145: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 146: mul r5.w, r1.w, l(0.080000)
    r5.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mad r6.yzw, -r1.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r4.xxyz
    r6.yzw = ((-(r1.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r4.xxyz)).yzw;
    // 148: mad r6.yzw, r2.wwww, r6.yyzw, r5.wwww
    r6.yzw = ((r2.wwww)*(r6.yyzw)+(r5.wwww)).yzw;
    // 149: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 151: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 152: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 153: mul_sat r1.w, r6.z, l(50.000000)
    r1.w = (saturate((r6.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 155: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: max r7.xyz, r6.yzwy, r3.wwww
    r7.xyz = (max(r6.yzwy,r3.wwww)).xyz;
    // 157: add r7.xyz, -r6.yzwy, r7.xyzx
    r7.xyz = ((-(r6.yzwy))+(r7.xyzx)).xyz;
    // 158: mad r6.yzw, -r0.wwww, r6.yyzw, r6.yyzw
    r6.yzw = ((-(r0.wwww))*(r6.yyzw)+(r6.yyzw)).yzw;
    // 159: mad r6.yzw, r1.wwww, r7.xxyz, r6.yyzw
    r6.yzw = ((r1.wwww)*(r7.xxyz)+(r6.yyzw)).yzw;
    // 160: dp3 r0.w, r6.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 161: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 162: mul r1.w, r4.w, l(0.500000)
    r1.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 163: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 164: min r0.w, r0.w, r1.w
    r0.w = (min(r0.wwww,r1.wwww)).w;
    // 165: mul r7.xyz, r6.yzwy, r0.wwww
    r7.xyz = ((r6.yzwy)*(r0.wwww)).xyz;
    // 166: add r6.yzw, -r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(r6.yyzw))+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 167: mad r5.xyz, r5.xyzx, r6.yzwy, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r6.yzwy)+(r7.xyzx)).xyz;
    // 168: mul r5.xyz, r6.xxxx, r5.xyzx
    r5.xyz = ((r6.xxxx)*(r5.xyzx)).xyz;
    // 169: mul r5.xyz, r5.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 170: mul_sat r6.xyz, cb0[10].xyzx, cb0[10].wwww
    r6.xyz = (saturate((source[10].xyzx)*(source[10].wwww))).xyz;
    // 171: mad r1.xyz, r2.xyzx, cb0[1].xxxx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(source[1].xxxx)+(r1.xyzx)).xyz;
    // 172: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 173: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 174: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 175: dp3_sat r0.x, r0.xyzx, -r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(-(r1.xyzx)).xyz).xxxx)).x;
    // 176: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 177: mul r0.x, r0.x, cb0[1].y
    r0.x = ((r0.xxxx)*(source[1].yyyy)).x;
    // 178: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 179: mad_sat r0.x, r0.x, cb0[1].w, cb0[1].z
    r0.x = (saturate((r0.xxxx)*(source[1].wwww)+(source[1].zzzz))).x;
    // 180: mul r0.x, r0.x, cb0[17].x
    r0.x = ((r0.xxxx)*(source[17].xxxx)).x;
    // 181: mul r0.xyz, r4.xyzx, r0.xxxx
    r0.xyz = ((r4.xyzx)*(r0.xxxx)).xyz;
    // 182: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 183: add r0.w, -r2.w, l(1.000000)
    r0.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 185: mad r0.xyz, r5.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 186: mul r0.xyz, r0.xyzx, l(0.450000, 0.450000, 0.450000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.450000,0.450000,0.450000,0.000000))).xyz;
    // 187: mul o0.xyz, r0.xyzx, cb0[18].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[18].xyzx)).xyz;
    // 188: mul r0.xy, v4.xyxx, cb0[16].xxxx
    r0.xy = ((v4.xyxx)*(source[16].xxxx)).xy;
    // 189: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s5, l(0.000000)
    r0.x = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 190: add r0.x, r0.x, -cb0[16].y
    r0.x = ((r0.xxxx)+(-(source[16].yyyy))).x;
    // 191: round_pi_sat r0.x, r0.x
    r0.x = (saturate(ceil(r0.xxxx))).x;
    // 192: mul_sat r0.x, r0.x, r8.w
    r0.x = (saturate((r0.xxxx)*(r8.wwww))).x;
    // 193: mul_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)*(source[16].zzzz))).x;
    // 194: mul o0.w, r0.x, cb0[2].x
    output.targets[0].w = ((r0.xxxx)*(source[2].xxxx)).w;
    // 195: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 196: ret
    return output;
}

// source.character.classic-armor-skin-masked.v1 / source program c5df3fee8e45da48bd2c274cccd1ca3a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight112(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[13]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[14]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[22].y=(g_SourceCharacterTime.xxxx).x;
    source[22].z=((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))).x;
    source[22].w=(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[23].x=((float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))))).x;
    source[23].y=(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))).x;
    source[27]=float4(input.lightColor,1.0);
    source[28].x=1.0;
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
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[28].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[28].xxxx)) * 0xffffffffu)).w;
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
    // 50: mul r10.xy, r9.xyxx, cb0[17].xxxx
    r10.xy = ((r9.xyxx)*(source[17].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mad r9.xy, cb0[17].wwww, r9.zwzz, -r10.xyxx
    r9.xy = ((source[17].wwww)*(r9.zwzz)+(-(r10.xyxx))).xy;
    // 53: mov r9.z, l(0)
    r9.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 54: mad r9.xyz, r11.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r11.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 55: add r10.xyz, -r9.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r9.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r12.xyz, cb0[19].xxxx, r10.xyzx, r9.xyzx
    r12.xyz = ((source[19].xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
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
    // 75: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 76: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 77: add r1.x, -cb0[18].y, cb0[18].x
    r1.x = ((-(source[18].yyyy))+(source[18].xxxx)).x;
    // 78: mad r1.x, r11.w, r1.x, cb0[18].y
    r1.x = ((r11.wwww)*(r1.xxxx)+(source[18].yyyy)).x;
    // 79: lt r1.y, |r0.z|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 80: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 81: mul r0.z, r0.z, r1.x
    r0.z = ((r0.zzzz)*(r1.xxxx)).z;
    // 82: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 83: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 84: movc r0.z, r1.y, l(0), r0.z
    r0.z = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 85: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 86: add r1.y, -r1.x, cb0[19].y
    r1.y = ((-(r1.xxxx))+(source[19].yyyy)).y;
    // 87: mad r1.x, r11.w, r1.y, r1.x
    r1.x = ((r11.wwww)*(r1.yyyy)+(r1.xxxx)).x;
    // 88: mul r1.x, r1.x, cb0[19].z
    r1.x = ((r1.xxxx)*(source[19].zzzz)).x;
    // 89: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 90: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 91: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 92: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 93: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 94: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 95: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 96: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 97: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 98: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 99: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 100: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 101: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 102: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 103: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 104: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t5.xywz, s5, r1.x
    r1.xyw = ((g_SourceCharacterTexture4.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 105: rcp r0.x, cb0[19].w
    r0.x = (1.0/(source[19].wwww)).x;
    // 106: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 107: mul r12.xyz, r4.xyzx, cb0[19].wwww
    r12.xyz = ((r4.xyzx)*(source[19].wwww)).xyz;
    // 108: exp r12.xyz, r12.xyzx
    r12.xyz = (exp2(r12.xyzx)).xyz;
    // 109: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 110: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 111: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 112: mad r4.xyz, r12.xyzx, cb0[19].wwww, r4.xyzx
    r4.xyz = ((r12.xyzx)*(source[19].wwww)+(r4.xyzx)).xyz;
    // 113: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 114: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 115: add r0.x, cb0[19].w, l(1.000000)
    r0.x = ((source[19].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 116: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 117: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 118: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 119: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 120: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 121: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 122: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 123: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 124: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 125: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 126: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 127: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 128: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 129: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 130: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 131: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 132: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 133: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 135: mul r12.xyz, r1.xywx, r4.xxxx
    r12.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 136: mul r13.xyz, r12.xyzx, cb0[23].wwww
    r13.xyz = ((r12.xyzx)*(source[23].wwww)).xyz;
    // 137: mul r4.z, r11.w, l(0.500000)
    r4.z = ((r11.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 138: add r4.w, -|r7.z|, l(1.000000)
    r4.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 140: mul r4.w, r0.z, r4.w
    r4.w = ((r0.zzzz)*(r4.wwww)).w;
    // 141: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 142: mad r9.xyz, r4.zzzz, r10.xyzx, r9.xyzx
    r9.xyz = ((r4.zzzz)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 143: dp3 r4.z, r9.xyzx, r9.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 144: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 145: div r9.xyz, r9.xyzx, r4.zzzz
    r9.xyz = ((r9.xyzx)/(r4.zzzz)).xyz;
    // 146: dp3 r4.z, r9.xyzx, r7.xyzx
    r4.z = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 147: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 148: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: dp3 r6.w, cb0[16].xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((source[16].xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 151: add r7.xyz, r6.wwww, -cb0[16].xyzx
    r7.xyz = ((r6.wwww)+(-(source[16].xyzx))).xyz;
    // 152: mad r7.xyz, r5.wwww, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r5.wwww)*(r7.xyzx)+(source[16].xyzx)).xyz;
    // 153: add r7.xyz, r7.xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 154: mad r7.xyz, r11.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 156: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 157: mad r7.xyz, r7.xyzx, r4.zzzz, r5.wwww
    r7.xyz = ((r7.xyzx)*(r4.zzzz)+(r5.wwww)).xyz;
    // 158: add_sat r4.z, r11.w, cb0[24].x
    r4.z = (saturate((r11.wwww)+(source[24].xxxx))).z;
    // 159: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: dp3 r6.x, r9.xyzx, r6.xyzx
    r6.x = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 161: mul_sat r6.xy, r6.xzxx, cb0[20].zzzz
    r6.xy = (saturate((r6.xzxx)*(source[20].zzzz))).xy;
    // 162: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 163: add_sat r6.y, r6.y, -cb0[20].w
    r6.y = (saturate((r6.yyyy)+(-(source[20].wwww)))).y;
    // 164: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 165: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 166: mul r6.y, r6.y, cb0[21].x
    r6.y = ((r6.yyyy)*(source[21].xxxx)).y;
    // 167: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 168: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 169: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 170: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: mul r6.yzw, r7.xxyz, r6.yyyy
    r6.yzw = ((r7.xxyz)*(r6.yyyy)).yzw;
    // 172: mad r7.w, r2.w, l(2.000000), -r4.x
    r7.w = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).w;
    // 173: mad r6.yzw, r6.yyzw, r7.wwww, r4.xxxx
    r6.yzw = ((r6.yyzw)*(r7.wwww)+(r4.xxxx)).yzw;
    // 174: add r6.yzw, r6.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((r6.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 175: mul_sat r6.yzw, r7.xxyz, r6.yyzw
    r6.yzw = (saturate((r7.xxyz)*(r6.yyzw))).yzw;
    // 176: mul r7.w, r0.z, r0.z
    r7.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 177: mul r8.w, r7.w, cb0[24].y
    r8.w = ((r7.wwww)*(source[24].yyyy)).w;
    // 178: mad r0.z, -r7.w, cb0[24].y, r0.z
    r0.z = ((-(r7.wwww))*(source[24].yyyy)+(r0.zzzz)).z;
    // 179: mad r0.z, r11.w, r0.z, r8.w
    r0.z = ((r11.wwww)*(r0.zzzz)+(r8.wwww)).z;
    // 180: mad r6.yzw, r4.zzzz, r6.yyzw, -r7.xxyz
    r6.yzw = ((r4.zzzz)*(r6.yyzw)+(-(r7.xxyz))).yzw;
    // 181: mad r6.yzw, r0.zzzz, r6.yyzw, r7.xxyz
    r6.yzw = ((r0.zzzz)*(r6.yyzw)+(r7.xxyz)).yzw;
    // 182: sqrt r0.z, r5.w
    r0.z = (sqrt(r5.wwww)).z;
    // 183: mul r5.xyz, r5.xyzx, r0.zzzz
    r5.xyz = ((r5.xyzx)*(r0.zzzz)).xyz;
    // 184: mul r5.xyz, r5.xyzx, r6.yzwy
    r5.xyz = ((r5.xyzx)*(r6.yzwy)).xyz;
    // 185: mad r6.yzw, -cb0[23].wwww, r12.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[23].wwww))*(r12.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 186: mad r6.yzw, r5.xxyz, r6.yyzw, r13.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r13.xxyz)).yzw;
    // 187: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 188: mad r10.xyz, cb0[4].wwww, cb0[4].xyzx, -r7.xyzx
    r10.xyz = ((source[4].wwww)*(source[4].xyzx)+(-(r7.xyzx))).xyz;
    // 189: mad r7.xyz, r11.xxxx, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.xxxx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 190: mad r10.xyz, cb0[5].wwww, cb0[5].xyzx, -r7.xyzx
    r10.xyz = ((source[5].wwww)*(source[5].xyzx)+(-(r7.xyzx))).xyz;
    // 191: mad r7.xyz, r11.yyyy, r10.xyzx, r7.xyzx
    r7.xyz = ((r11.yyyy)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 192: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 194: mad r7.xyz, cb0[18].zzzz, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].zzzz)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 195: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 196: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 197: mad r7.xyz, cb0[18].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[18].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 198: mad r10.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 199: mad r12.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 200: mul r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r12.xyzx)).xyz;
    // 201: mul r12.xyz, r7.xyzx, r10.xyzx
    r12.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 202: mad r7.xyz, -r7.xyzx, r10.xyzx, cb0[8].xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.xyzx)+(source[8].xyzx)).xyz;
    // 203: mad r7.xyz, r11.wwww, r7.xyzx, r12.xyzx
    r7.xyz = ((r11.wwww)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 204: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 206: mad r8.xyz, cb0[18].zzzz, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].zzzz)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 207: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 208: add r12.xyz, -r8.xyzx, r0.zzzz
    r12.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 209: mad r8.xyz, cb0[18].wwww, r12.xyzx, r8.xyzx
    r8.xyz = ((source[18].wwww)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 210: mul r12.xyz, r7.xyzx, r8.xyzx
    r12.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 211: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 212: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 214: add r1.yzw, -cb0[9].xxyz, cb0[10].xxyz
    r1.yzw = ((-(source[9].xxyz))+(source[10].xxyz)).yzw;
    // 215: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[9].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[9].xyzx)).xyz;
    // 216: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 217: mul r1.xyz, r1.xyzx, cb0[20].xxxx
    r1.xyz = ((r1.xyzx)*(source[20].xxxx)).xyz;
    // 218: mul r13.xyz, r1.xyzx, r12.xyzx
    r13.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 219: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 220: add r14.xyz, -r2.xyzx, r0.zzzz
    r14.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 221: mad r14.xyz, cb0[18].zzzz, r14.xyzx, r2.xyzx
    r14.xyz = ((source[18].zzzz)*(r14.xyzx)+(r2.xyzx)).xyz;
    // 222: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 223: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 224: mad r14.xyz, cb0[18].wwww, r15.xyzx, r14.xyzx
    r14.xyz = ((source[18].wwww)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 225: dp3 r0.z, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: add r15.xyz, -r14.xyzx, r0.zzzz
    r15.xyz = ((-(r14.xyzx))+(r0.zzzz)).xyz;
    // 227: mul r15.xyz, r15.xyzx, cb0[20].yyyy
    r15.xyz = ((r15.xyzx)*(source[20].yyyy)).xyz;
    // 228: add r0.z, r11.y, r11.x
    r0.z = ((r11.yyyy)+(r11.xxxx)).z;
    // 229: add r0.z, r11.z, r0.z
    r0.z = ((r11.zzzz)+(r0.zzzz)).z;
    // 230: add_sat r0.z, r11.w, r0.z
    r0.z = (saturate((r11.wwww)+(r0.zzzz))).z;
    // 231: mad r11.xyz, r0.zzzz, r15.xyzx, r14.xyzx
    r11.xyz = ((r0.zzzz)*(r15.xyzx)+(r14.xyzx)).xyz;
    // 232: add r2.xyz, r2.xxxx, -r11.xyzx
    r2.xyz = ((r2.xxxx)+(-(r11.xyzx))).xyz;
    // 233: mad r2.xyz, r11.wwww, r2.xyzx, r11.xyzx
    r2.xyz = ((r11.wwww)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 234: max r11.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r11.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 235: log r11.xyz, r11.xyzx
    r11.xyz = (log2(r11.xyzx)).xyz;
    // 236: mul r11.xyz, r11.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r11.xyz = ((r11.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 237: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 238: dp3 r0.z, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 239: add r1.w, -cb0[21].z, cb0[21].y
    r1.w = ((-(source[21].zzzz))+(source[21].yyyy)).w;
    // 240: mad r1.w, r11.w, r1.w, cb0[21].z
    r1.w = ((r11.wwww)*(r1.wwww)+(source[21].zzzz)).w;
    // 241: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 242: mul r0.z, r0.z, r1.w
    r0.z = ((r0.zzzz)*(r1.wwww)).z;
    // 243: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 244: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 245: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 247: div r1.w, cb0[21].w, r1.w
    r1.w = ((source[21].wwww)/(r1.wwww)).w;
    // 248: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 249: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 250: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 251: mul r1.w, r1.w, cb0[22].x
    r1.w = ((r1.wwww)*(source[22].xxxx)).w;
    // 252: mad r1.xyz, r2.xyzx, r1.xyzx, -r13.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r13.xyzx))).xyz;
    // 253: mad r1.xyz, r1.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 254: mad r1.xyz, r4.xxxx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 255: mad r1.xyz, r0.zzzz, r1.xyzx, r12.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 256: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 257: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 258: mad r1.xyz, cb0[18].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 259: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 260: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 261: mad r1.xyz, cb0[18].wwww, r11.xyzx, r1.xyzx
    r1.xyz = ((source[18].wwww)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 262: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 263: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 264: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 265: mul r4.z, r4.z, cb0[22].y
    r4.z = ((r4.zzzz)*(source[22].yyyy)).z;
    // 266: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 267: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 268: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 269: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 270: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 271: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 272: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 273: mul r10.z, r6.x, l(0.125000)
    r10.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 274: mov r10.xw, l(0,0,0,0)
    r10.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 275: mul r10.y, cb0[2].y, cb0[11].y
    r10.y = ((source[2].yyyy)*(source[11].yyyy)).y;
    // 276: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 277: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 278: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 279: add r10.xy, r10.xyxx, r11.xyxx
    r10.xy = ((r10.xyxx)+(r11.xyxx)).xy;
    // 280: add r10.xy, r10.xyxx, r10.zwzz
    r10.xy = ((r10.xyxx)+(r10.zwzz)).xy;
    // 281: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t6.xyzw, s6, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 282: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 283: mul r1.w, r4.z, r10.w
    r1.w = ((r4.zzzz)*(r10.wwww)).w;
    // 284: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 285: mad r1.xyz, r1.wwww, r10.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r10.xyzx)+(r1.xyzx)).xyz;
    // 286: mul r1.w, cb0[12].y, cb0[22].y
    r1.w = ((source[12].yyyy)*(source[22].yyyy)).w;
    // 287: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 288: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 289: mul r10.y, r1.w, l(0.020000)
    r10.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 290: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 291: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 292: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 293: mad r3.xy, cb0[12].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[12].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 294: mul r3.z, cb0[12].x, l(0.001000)
    r3.z = ((source[12].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 295: mov r10.x, l(0)
    r10.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 296: mad r3.xy, r3.zzzz, r3.xyxx, r10.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r10.xyxx)).xy;
    // 297: dp2 r3.z, cb0[13].xyxx, r3.xyxx
    r3.z = (dot((source[13].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 298: dp2 r3.y, cb0[14].xyxx, r3.xyxx
    r3.y = (dot((source[14].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 299: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 300: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 301: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 302: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 303: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 304: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 305: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 306: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 307: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 308: mul r10.xyz, r3.xyzx, cb0[12].zzzz
    r10.xyz = ((r3.xyzx)*(source[12].zzzz)).xyz;
    // 309: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 310: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 311: mad r3.xyz, cb0[12].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[12].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 312: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 313: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 314: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 315: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 316: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 317: dp3 r0.x, r0.xywx, r9.xyzx
    r0.x = (dot((r0.xywx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 318: mul r0.z, r2.w, cb0[24].z
    r0.z = ((r2.wwww)*(source[24].zzzz)).z;
    // 319: mul r0.w, r11.w, r0.z
    r0.w = ((r11.wwww)*(r0.zzzz)).w;
    // 320: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 321: min r0.w, r0.w, cb0[24].z
    r0.w = (min(r0.wwww,source[24].zzzz)).w;
    // 322: add r1.w, -cb0[25].y, cb0[25].x
    r1.w = ((-(source[25].yyyy))+(source[25].xxxx)).w;
    // 323: mad r1.w, cb0[24].w, r1.w, cb0[25].y
    r1.w = ((source[24].wwww)*(r1.wwww)+(source[25].yyyy)).w;
    // 324: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 325: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 326: mad r1.w, r11.w, r1.w, l(1.000000)
    r1.w = ((r11.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 327: lt r2.w, |r0.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 328: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 329: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 330: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 331: movc r0.y, r2.w, l(0), r0.w
    r0.y = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 332: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 333: add r0.y, cb0[25].w, -cb0[26].x
    r0.y = ((source[25].wwww)+(-(source[26].xxxx))).y;
    // 334: mad r0.y, cb0[25].z, r0.y, cb0[26].x
    r0.y = ((source[25].zzzz)*(r0.yyyy)+(source[26].xxxx)).y;
    // 335: mul r0.y, r0.y, r11.w
    r0.y = ((r0.yyyy)*(r11.wwww)).y;
    // 336: sample_b_indexable(texture2d)(float,float,float,float) r0.xzw, r0.xzxx, t7.xwyz, s8, l(0.000000)
    r0.xzw = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterLookupSampler, (r0.xzxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xwyz).xzw;
    // 337: mad r0.xyz, r0.yyyy, r5.xyzx, r0.xzwx
    r0.xyz = ((r0.yyyy)*(r5.xyzx)+(r0.xzwx)).xyz;
    // 338: add r0.w, -cb0[26].y, l(2.000000)
    r0.w = ((-(source[26].yyyy))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 339: mad r0.w, r4.x, r0.w, cb0[26].y
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[26].yyyy)).w;
    // 340: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 341: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 342: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 343: mul r0.xyz, r0.xyzx, cb0[26].zzzz
    r0.xyz = ((r0.xyzx)*(source[26].zzzz)).xyz;
    // 344: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 345: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 346: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 347: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 348: mul r0.xyz, r0.xyzx, cb0[26].wwww
    r0.xyz = ((r0.xyzx)*(source[26].wwww)).xyz;
    // 349: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 350: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 351: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 352: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 353: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 354: mul o0.xyz, r0.xyzx, cb0[27].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)).xyz;
    // 355: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 356: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 357: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 358: ret
    return output;
}


// source.character.equipment-native-160.v1 / source program 5262375c2e4ee34d8b51f43c41173a05
#else // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
    case 108u: return SourceCharacterLight108(input);
    case 109u: return SourceCharacterLight109(input);
    case 110u: return SourceCharacterLight110(input);
    case 111u: return SourceCharacterLight111(input);
    case 112u: return SourceCharacterLight112(input);
#endif // SOURCE_CHARACTER_LIGHT_DISPATCH_CASES
