SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight214(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[13]=float4(input.lightColor,1.0);
    source[14].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: mul r5.xy, v4.xyxx, cb0[3].xyxx
    r5.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t0.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 18: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r7.xy, r5.xyxx, cb0[7].xxxx
    r7.xy = ((r5.xyxx)*(source[7].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r7.xy, r7.xyxx, cb0[7].yyyy
    r7.xy = ((r7.xyxx)*(source[7].yyyy)).xy;
    // 28: mad r5.zw, cb0[6].wwww, r5.zzzw, r7.xxxy
    r5.zw = ((source[6].wwww)*(r5.zzzw)+(r7.xxxy)).zw;
    // 29: mul r6.xy, r5.zwzz, v2.wwww
    r6.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 33: max r1.w, cb0[7].z, l(0.000000)
    r1.w = (max(source[7].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 35: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 37: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 38: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 39: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 40: mul r0.xy, cb0[0].xyxx, cb0[7].wwww
    r0.xy = ((source[0].xyxx)*(source[7].wwww)).xy;
    // 41: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 42: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 43: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 44: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 45: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r0.y, r6.z, r6.z
    r0.y = ((r6.zzzz)*(r6.zzzz)).y;
    // 49: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 50: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 51: mul r1.xy, v4.xyxx, cb0[8].wwww
    r1.xy = ((v4.xyxx)*(source[8].wwww)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.z, r8.w, r8.w
    r0.z = ((r8.wwww)*(r8.wwww)).z;
    // 54: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 55: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 56: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 57: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 58: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 59: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 60: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 61: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 62: add r1.xyz, -r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r6.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 63: mad r1.xyz, r0.yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 64: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 65: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 66: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 67: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 68: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 69: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 70: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 71: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 72: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 73: else
    } else {
    // 74: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 75: endif
    }
    // 76: add r6.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: dp3 r0.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 78: add r9.xyz, -r7.xyzx, r0.yyyy
    r9.xyz = ((-(r7.xyzx))+(r0.yyyy)).xyz;
    // 79: mad r7.xyz, cb0[9].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[9].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 80: mul r9.xyz, cb0[4].xyzx, cb0[9].zzzz
    r9.xyz = ((source[4].xyzx)*(source[9].zzzz)).xyz;
    // 81: mul r10.xyz, r7.xyzx, r9.xyzx
    r10.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 82: mul r11.xyz, cb0[5].xyzx, cb0[9].wwww
    r11.xyz = ((source[5].xyzx)*(source[9].wwww)).xyz;
    // 83: mul r12.xyz, r8.xyzx, r11.xyzx
    r12.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 84: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 85: mad r8.xyz, -r11.xyzx, r8.xyzx, r0.yyyy
    r8.xyz = ((-(r11.xyzx))*(r8.xyzx)+(r0.yyyy)).xyz;
    // 86: mad r8.xyz, cb0[10].yyyy, r8.xyzx, r12.xyzx
    r8.xyz = ((source[10].yyyy)*(r8.xyzx)+(r12.xyzx)).xyz;
    // 87: mad r7.xyz, -r7.xyzx, r9.xyzx, r8.xyzx
    r7.xyz = ((-(r7.xyzx))*(r9.xyzx)+(r8.xyzx)).xyz;
    // 88: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 89: mul r7.xyz, r0.xyzx, cb0[10].zzzz
    r7.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t4.yzxw, s5, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 91: mul r1.w, r5.y, cb0[11].x
    r1.w = ((r5.yyyy)*(source[11].xxxx)).w;
    // 92: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 93: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 94: mul r1.w, r1.w, cb0[11].y
    r1.w = ((r1.wwww)*(source[11].yyyy)).w;
    // 95: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 96: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 97: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mad r0.xyz, cb0[10].wwww, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[10].wwww)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 99: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 100: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 101: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 102: mov_sat r2.w, cb0[11].z
    r2.w = (saturate(source[11].zzzz)).w;
    // 103: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 104: mul r3.w, r5.x, cb0[12].x
    r3.w = ((r5.xxxx)*(source[12].xxxx)).w;
    // 105: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 106: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 107: mul r3.w, r3.w, cb0[12].y
    r3.w = ((r3.wwww)*(source[12].yyyy)).w;
    // 108: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 109: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 110: max r3.w, r3.w, cb0[1].x
    r3.w = (max(r3.wwww,source[1].xxxx)).w;
    // 111: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 113: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 114: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 115: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 116: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 117: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 118: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 119: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 121: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 122: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 125: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 128: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 129: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 130: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 131: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 132: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 133: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 134: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 135: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 136: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 138: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 139: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 140: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 141: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 142: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 143: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 144: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 145: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 146: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 148: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 149: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 150: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 151: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 152: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 154: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 155: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 156: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 157: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 158: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 159: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 160: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 161: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 162: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 163: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 165: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 166: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 167: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 168: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 169: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 170: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 171: ret
    return output;
}

// source.character.static-map-native-215.v1 / source program 2517a4f29a5d67489e62e2f08bc0f3de
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight215(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[5]=float4(input.lightColor,1.0);
    source[6].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[6].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[6].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s0
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 13: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 14: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: add r1.w, r2.w, l(-0.333300)
    r1.w = ((r2.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 17: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 18: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 19: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t3.zwxy, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 21: mul r2.w, r3.y, cb0[3].z
    r2.w = ((r3.yyyy)*(source[3].zzzz)).w;
    // 22: add r4.xyz, -r2.xyzx, r1.wwww
    r4.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 23: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 24: mad r4.xyz, cb0[3].wwww, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[3].wwww)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 25: mad r3.yzw, r3.yyyy, r4.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r3.yzw = ((r3.yyyy)*(r4.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 26: mul r2.xyz, r2.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r3.yzwy)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 28: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 29: dp2 r1.w, r3.yzyy, r3.yzyy
    r1.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 30: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 34: mul r4.xy, r3.yzyy, cb0[3].xxxx
    r4.xy = ((r3.yzyy)*(source[3].xxxx)).xy;
    // 35: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: div r3.yzw, r4.xxyz, r1.wwww
    r3.yzw = ((r4.xxyz)/(r1.wwww)).yzw;
    // 38: dp3 r1.w, r3.yzwy, r1.xyzx
    r1.w = (dot((r3.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 42: mul r5.xyz, cb0[2].xyzx, cb0[4].zzzz
    r5.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 43: mul r5.xyz, r3.xxxx, r5.xyzx
    r5.xyz = ((r3.xxxx)*(r5.xyzx)).xyz;
    // 44: mad r6.xyz, r5.xyzx, r0.xyzx, -r4.xyzx
    r6.xyz = ((r5.xyzx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 45: mad r4.xyz, r5.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r5.xyz, r5.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(source[1].xyzx)).xyz;
    // 48: mul r5.xyz, r5.xyzx, cb0[4].xxxx
    r5.xyz = ((r5.xyzx)*(source[4].xxxx)).xyz;
    // 49: mad r1.xyz, v7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 50: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 51: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 52: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 53: dp3 r0.w, r1.xyzx, r3.yzwy
    r0.w = (dot((r1.xyzx).xyz,(r3.yzwy).xyz).xxxx).w;
    // 54: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 55: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 56: mul r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)*(source[4].yyyy)).w;
    // 57: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 58: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 60: mul r1.xyz, r5.xyzx, r0.wwww
    r1.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // 61: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 62: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 63: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 64: mad r0.xyz, r2.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 65: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 66: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 67: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 68: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 69: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 70: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 71: ret
    return output;
}

// source.character.static-map-native-216.v1 / source program b8bc0688de3edf48ac5c62e71bee5c9a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight216(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[14]=float4(input.lightColor,1.0);
    source[15].x=1.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r7.xyzw, v4.xyxy, cb0[7].yyww
    r7.xyzw = ((v4.xyxy)*(source[7].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r7.xy, r7.xyxx, cb0[7].zzzz
    r7.xy = ((r7.xyxx)*(source[7].zzzz)).xy;
    // 27: mad r5.xy, cb0[7].xxxx, r5.xyxx, r7.xyxx
    r5.xy = ((source[7].xxxx)*(r5.xyxx)+(r7.xyxx)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r7.zwzz, t2.xyzw, s3, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 33: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 34: dp2 r1.w, r6.xyxx, r6.xyxx
    r1.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: mul r8.xy, r6.xyxx, cb0[8].xxxx
    r8.xy = ((r6.xyxx)*(source[8].xxxx)).xy;
    // 40: max r1.w, cb0[8].y, l(0.000000)
    r1.w = (max(source[8].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 44: dp3 r6.x, r1.xyzx, r5.xywx
    r6.x = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 45: dp3 r6.y, r2.xyzx, r5.xywx
    r6.y = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 46: dp3 r6.z, r0.xyzx, r5.xywx
    r6.z = (dot((r0.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // 47: mul r0.xy, cb0[1].xyxx, cb0[8].zzzz
    r0.xy = ((source[1].xyxx)*(source[8].zzzz)).xy;
    // 48: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 49: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 50: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 51: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 52: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r0.y, r5.w, r5.w
    r0.y = ((r5.wwww)*(r5.wwww)).y;
    // 56: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 57: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.zwzz, t4.xyzw, s5, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 60: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 61: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 62: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 63: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 64: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 65: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 66: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 67: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 68: add r8.xyz, -r5.xywx, r8.xyzx
    r8.xyz = ((-(r5.xywx))+(r8.xyzx)).xyz;
    // 69: mad r5.xyw, r0.yyyy, r8.xyxz, r5.xyxw
    r5.xyw = ((r0.yyyy)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 70: dp3 r0.y, r5.xywx, r5.xywx
    r0.y = (dot((r5.xywx).xyz,(r5.xywx).xyz).xxxx).y;
    // 71: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 72: mul r5.xyw, r0.yyyy, r5.xyxw
    r5.xyw = ((r0.yyyy)*(r5.xyxw)).xyw;
    // 73: dp3 r0.y, r5.xywx, r3.xyzx
    r0.y = (dot((r5.xywx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 74: mul r8.xyz, r0.yyyy, r5.xywx
    r8.xyz = ((r0.yyyy)*(r5.xywx)).xyz;
    // 75: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 76: ne r0.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].x
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].xxxx)) * 0xffffffffu)).z;
    // 77: if_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) {
    // 78: div r9.xy, v8.xyxx, v8.wwww
    r9.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 79: mad r9.xy, r9.xyxx, cb2[0].xyxx, cb2[0].wzww
    r9.xy = ((r9.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 80: sample_indexable(texture2d)(float,float,float,float) r9.xyz, r9.xyxx, t7.xyzw, s0
    r9.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 81: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 82: else
    } else {
    // 83: mov r9.xyz, l(1.000000,1.000000,1.000000,0)
    r9.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 84: endif
    }
    // 85: add r10.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 86: dp3 r1.x, r1.xyzx, r8.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 87: dp3 r1.y, r2.xyzx, r8.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 88: mul r1.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r1.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 89: mad r1.xy, cb0[9].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[9].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s6, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 91: mul r1.xyz, r1.xyzx, cb0[4].xyzx
    r1.xyz = ((r1.xyzx)*(source[4].xyzx)).xyz;
    // 92: mad r1.xyz, cb0[9].wwww, r1.xyzx, r1.xyzx
    r1.xyz = ((source[9].wwww)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 93: add r1.xyz, r1.xyzx, -cb0[9].wwww
    r1.xyz = ((r1.xyzx)+(-(source[9].wwww))).xyz;
    // 94: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 95: mul r0.z, r5.z, cb0[10].x
    r0.z = ((r5.zzzz)*(source[10].xxxx)).z;
    // 96: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 97: add r8.xyz, -r6.xyzx, r1.wwww
    r8.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 98: mad r6.xyz, cb0[10].zzzz, r8.xyzx, r6.xyzx
    r6.xyz = ((source[10].zzzz)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 99: mul r8.xyz, cb0[5].xyzx, cb0[10].wwww
    r8.xyz = ((source[5].xyzx)*(source[10].wwww)).xyz;
    // 100: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 101: mad r2.xyz, r0.zzzz, r2.xyzx, r6.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 102: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 103: mad r1.xyz, -r0.zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 104: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 106: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 107: mul r2.xyz, cb0[6].xyzx, cb0[11].xxxx
    r2.xyz = ((source[6].xyzx)*(source[11].xxxx)).xyz;
    // 108: mul r6.xyz, r7.xyzx, r2.xyzx
    r6.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 109: dp3 r0.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 110: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.zzzz
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.zzzz)).xyz;
    // 111: mad r2.xyz, cb0[11].zzzz, r2.xyzx, r6.xyzx
    r2.xyz = ((source[11].zzzz)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 112: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 113: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 114: mul r2.xyz, r1.xyzx, cb0[11].wwww
    r2.xyz = ((r1.xyzx)*(source[11].wwww)).xyz;
    // 115: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, v4.xyxx, t6.yxzw, s7, l(0.000000)
    r0.xz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).xz;
    // 116: mul r0.z, r0.z, cb0[12].y
    r0.z = ((r0.zzzz)*(source[12].yyyy)).z;
    // 117: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 118: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 119: mul r0.z, r0.z, cb0[12].z
    r0.z = ((r0.zzzz)*(source[12].zzzz)).z;
    // 120: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 121: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 122: min r1.w, r0.z, l(1.000000)
    r1.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mad r1.xyz, cb0[12].xxxx, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[12].xxxx)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 124: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 125: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 126: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 127: mov_sat r1.w, cb0[12].w
    r1.w = (saturate(source[12].wwww)).w;
    // 128: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 129: mul r0.x, r0.x, cb0[13].y
    r0.x = ((r0.xxxx)*(source[13].yyyy)).x;
    // 130: lt r2.x, |r0.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 131: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 132: mul r0.x, r0.x, cb0[13].z
    r0.x = ((r0.xxxx)*(source[13].zzzz)).x;
    // 133: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 134: movc r0.x, r2.x, l(0), r0.x
    r0.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 135: max r0.x, r0.x, cb0[2].x
    r0.x = (max(r0.xxxx,source[2].xxxx)).x;
    // 136: mad r2.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 137: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 138: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 139: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 140: dp3_sat r2.w, r5.xywx, r2.xyzx
    r2.w = (saturate(dot((r5.xywx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // 141: add r0.y, |r0.y|, l(0.000010)
    r0.y = ((abs(r0.yyyy))+(float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 142: dp3_sat r3.w, r5.xywx, r4.xyzx
    r3.w = (saturate(dot((r5.xywx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 143: dp3_sat r2.x, r3.xyzx, r2.xyzx
    r2.x = (saturate(dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 144: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: min r0.xyw, r0.xyxw, l(1.000000, 1.000000, 0.000000, 1.000000)
    r0.xyw = (min(r0.xyxw,float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 146: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 147: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 148: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: mad r2.xyz, -r1.xyzx, r0.zzzz, r1.xyzx
    r2.xyz = ((-(r1.xyzx))*(r0.zzzz)+(r1.xyzx)).xyz;
    // 150: mul r3.x, r0.x, r0.x
    r3.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 151: mul r3.y, r3.x, r3.x
    r3.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 152: mad r3.z, r2.w, r3.y, -r2.w
    r3.z = ((r2.wwww)*(r3.yyyy)+(-(r2.wwww))).z;
    // 153: mad r2.w, r3.z, r2.w, l(1.000000)
    r2.w = ((r3.zzzz)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 155: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 156: div r2.w, r3.y, r2.w
    r2.w = ((r3.yyyy)/(r2.wwww)).w;
    // 157: mad r3.y, -r0.x, r0.x, l(1.000000)
    r3.y = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 158: mad r3.z, r0.y, r3.y, r3.x
    r3.z = ((r0.yyyy)*(r3.yyyy)+(r3.xxxx)).z;
    // 159: mad r3.x, r3.w, r3.y, r3.x
    r3.x = ((r3.wwww)*(r3.yyyy)+(r3.xxxx)).x;
    // 160: mul r0.y, r0.y, r3.x
    r0.y = ((r0.yyyy)*(r3.xxxx)).y;
    // 161: mad r0.y, r3.w, r3.z, r0.y
    r0.y = ((r3.wwww)*(r3.zzzz)+(r0.yyyy)).y;
    // 162: rcp r0.y, r0.y
    r0.y = (1.0/(r0.yyyy)).y;
    // 163: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 164: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 165: mad r1.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r1.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 166: mad r1.xyz, r0.zzzz, r1.xyzx, r2.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.wwww)).xyz;
    // 167: add r0.xz, -r0.xxwx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = ((-(r0.xxwx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 168: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 169: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 170: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 171: mul_sat r0.w, r1.y, l(50.000000)
    r0.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 172: mul r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)*(r0.wwww)).w;
    // 173: max r3.xyz, r1.xyzx, r0.xxxx
    r3.xyz = (max(r1.xyzx,r0.xxxx)).xyz;
    // 174: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 175: mad r1.xyz, -r0.zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 176: mad r0.xzw, r0.wwww, r3.xxyz, r1.xxyz
    r0.xzw = ((r0.wwww)*(r3.xxyz)+(r1.xxyz)).xzw;
    // 177: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 178: add r1.x, r1.x, l(0.000100)
    r1.x = ((r1.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 179: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 180: div r1.x, l(3.000000), r1.x
    r1.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.xxxx)).x;
    // 181: min r0.y, r0.y, r1.x
    r0.y = (min(r0.yyyy,r1.xxxx)).y;
    // 182: mul r1.xyz, r0.xzwx, r0.yyyy
    r1.xyz = ((r0.xzwx)*(r0.yyyy)).xyz;
    // 183: add r0.xyz, -r0.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xzwx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 185: mul r0.xyz, r3.wwww, r0.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)).xyz;
    // 186: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 187: mul r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r0.xyzx)).xyz;
    // 188: mul o0.xyz, r0.xyzx, cb0[14].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)).xyz;
    // 189: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 190: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 191: ret
    return output;
}

// source.character.static-map-native-217.v1 / source program b4f7c18fd139c94d85db7120afb908d6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight217(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[14]=float4(input.lightColor,1.0);
    source[15].x=1.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r7.xyzw, v4.xyxy, cb0[7].yyww
    r7.xyzw = ((v4.xyxy)*(source[7].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r7.xy, r7.xyxx, cb0[7].zzzz
    r7.xy = ((r7.xyxx)*(source[7].zzzz)).xy;
    // 27: mad r5.xy, cb0[7].xxxx, r5.xyxx, r7.xyxx
    r5.xy = ((source[7].xxxx)*(r5.xyxx)+(r7.xyxx)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r7.zwzz, t2.xyzw, s3, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 33: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 34: dp2 r1.w, r6.xyxx, r6.xyxx
    r1.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: mul r8.xy, r6.xyxx, cb0[8].xxxx
    r8.xy = ((r6.xyxx)*(source[8].xxxx)).xy;
    // 40: max r1.w, cb0[8].y, l(0.000000)
    r1.w = (max(source[8].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 44: dp3 r6.x, r1.xyzx, r5.xywx
    r6.x = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 45: dp3 r6.y, r2.xyzx, r5.xywx
    r6.y = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 46: dp3 r6.z, r0.xyzx, r5.xywx
    r6.z = (dot((r0.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // 47: mul r0.xy, cb0[1].xyxx, cb0[8].zzzz
    r0.xy = ((source[1].xyxx)*(source[8].zzzz)).xy;
    // 48: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 49: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 50: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 51: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 52: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r0.y, r5.w, r5.w
    r0.y = ((r5.wwww)*(r5.wwww)).y;
    // 56: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 57: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.zwzz, t4.xyzw, s5, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 60: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 61: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 62: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 63: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 64: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 65: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 66: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 67: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 68: add r8.xyz, -r5.xywx, r8.xyzx
    r8.xyz = ((-(r5.xywx))+(r8.xyzx)).xyz;
    // 69: mad r5.xyw, r0.yyyy, r8.xyxz, r5.xyxw
    r5.xyw = ((r0.yyyy)*(r8.xyxz)+(r5.xyxw)).xyw;
    // 70: dp3 r0.y, r5.xywx, r5.xywx
    r0.y = (dot((r5.xywx).xyz,(r5.xywx).xyz).xxxx).y;
    // 71: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 72: mul r5.xyw, r0.yyyy, r5.xyxw
    r5.xyw = ((r0.yyyy)*(r5.xyxw)).xyw;
    // 73: dp3 r0.y, r5.xywx, r3.xyzx
    r0.y = (dot((r5.xywx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 74: mul r8.xyz, r0.yyyy, r5.xywx
    r8.xyz = ((r0.yyyy)*(r5.xywx)).xyz;
    // 75: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 76: ne r0.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].x
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].xxxx)) * 0xffffffffu)).z;
    // 77: if_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) {
    // 78: div r9.xy, v8.xyxx, v8.wwww
    r9.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 79: mad r9.xy, r9.xyxx, cb2[0].xyxx, cb2[0].wzww
    r9.xy = ((r9.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 80: sample_indexable(texture2d)(float,float,float,float) r9.xyz, r9.xyxx, t7.xyzw, s0
    r9.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 81: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 82: else
    } else {
    // 83: mov r9.xyz, l(1.000000,1.000000,1.000000,0)
    r9.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 84: endif
    }
    // 85: add r10.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 86: dp3 r1.x, r1.xyzx, r8.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 87: dp3 r1.y, r2.xyzx, r8.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 88: mul r1.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r1.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 89: mad r1.xy, cb0[9].zzzz, r1.xyxx, r1.zwzz
    r1.xy = ((source[9].zzzz)*(r1.xyxx)+(r1.zwzz)).xy;
    // 90: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s6, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 91: mul r1.xyz, r1.xyzx, cb0[4].xyzx
    r1.xyz = ((r1.xyzx)*(source[4].xyzx)).xyz;
    // 92: mad r1.xyz, cb0[9].wwww, r1.xyzx, r1.xyzx
    r1.xyz = ((source[9].wwww)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 93: add r1.xyz, r1.xyzx, -cb0[9].wwww
    r1.xyz = ((r1.xyzx)+(-(source[9].wwww))).xyz;
    // 94: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 95: mul r0.z, r5.z, cb0[10].x
    r0.z = ((r5.zzzz)*(source[10].xxxx)).z;
    // 96: mul r8.xyz, cb0[5].xyzx, cb0[10].yyyy
    r8.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 97: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 98: mad r2.xyz, r0.zzzz, r2.xyzx, r6.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 99: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 100: mad r1.xyz, -r0.zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 101: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 102: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 103: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 104: mul r2.xyz, cb0[6].xyzx, cb0[10].zzzz
    r2.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 105: mul r6.xyz, r7.xyzx, r2.xyzx
    r6.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 106: dp3 r0.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 107: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.zzzz
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.zzzz)).xyz;
    // 108: mad r2.xyz, cb0[11].xxxx, r2.xyzx, r6.xyzx
    r2.xyz = ((source[11].xxxx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 109: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 110: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 111: mul r2.xyz, r1.xyzx, cb0[11].yyyy
    r2.xyz = ((r1.xyzx)*(source[11].yyyy)).xyz;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r0.xz, v4.xyxx, t6.yxzw, s7, l(0.000000)
    r0.xz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).xz;
    // 113: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 114: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 115: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 116: mul r0.z, r0.z, cb0[12].x
    r0.z = ((r0.zzzz)*(source[12].xxxx)).z;
    // 117: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 118: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 119: min r1.w, r0.z, l(1.000000)
    r1.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: mad r1.xyz, cb0[11].zzzz, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[11].zzzz)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 121: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 122: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 123: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 124: mov_sat r1.w, cb0[12].y
    r1.w = (saturate(source[12].yyyy)).w;
    // 125: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 126: mul r0.x, r0.x, cb0[12].w
    r0.x = ((r0.xxxx)*(source[12].wwww)).x;
    // 127: lt r2.x, |r0.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 128: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 129: mul r0.x, r0.x, cb0[13].x
    r0.x = ((r0.xxxx)*(source[13].xxxx)).x;
    // 130: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 131: movc r0.x, r2.x, l(0), r0.x
    r0.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 132: max r0.x, r0.x, cb0[2].x
    r0.x = (max(r0.xxxx,source[2].xxxx)).x;
    // 133: mad r2.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 134: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 135: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 136: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 137: dp3_sat r2.w, r5.xywx, r2.xyzx
    r2.w = (saturate(dot((r5.xywx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // 138: add r0.y, |r0.y|, l(0.000010)
    r0.y = ((abs(r0.yyyy))+(float4(0.000010,0.000010,0.000010,0.000010))).y;
    // 139: dp3_sat r3.w, r5.xywx, r4.xyzx
    r3.w = (saturate(dot((r5.xywx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 140: dp3_sat r2.x, r3.xyzx, r2.xyzx
    r2.x = (saturate(dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 141: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: min r0.xyw, r0.xyxw, l(1.000000, 1.000000, 0.000000, 1.000000)
    r0.xyw = (min(r0.xyxw,float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 143: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 144: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 145: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mad r2.xyz, -r1.xyzx, r0.zzzz, r1.xyzx
    r2.xyz = ((-(r1.xyzx))*(r0.zzzz)+(r1.xyzx)).xyz;
    // 147: mul r3.x, r0.x, r0.x
    r3.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 148: mul r3.y, r3.x, r3.x
    r3.y = ((r3.xxxx)*(r3.xxxx)).y;
    // 149: mad r3.z, r2.w, r3.y, -r2.w
    r3.z = ((r2.wwww)*(r3.yyyy)+(-(r2.wwww))).z;
    // 150: mad r2.w, r3.z, r2.w, l(1.000000)
    r2.w = ((r3.zzzz)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 152: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 153: div r2.w, r3.y, r2.w
    r2.w = ((r3.yyyy)/(r2.wwww)).w;
    // 154: mad r3.y, -r0.x, r0.x, l(1.000000)
    r3.y = ((-(r0.xxxx))*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mad r3.z, r0.y, r3.y, r3.x
    r3.z = ((r0.yyyy)*(r3.yyyy)+(r3.xxxx)).z;
    // 156: mad r3.x, r3.w, r3.y, r3.x
    r3.x = ((r3.wwww)*(r3.yyyy)+(r3.xxxx)).x;
    // 157: mul r0.y, r0.y, r3.x
    r0.y = ((r0.yyyy)*(r3.xxxx)).y;
    // 158: mad r0.y, r3.w, r3.z, r0.y
    r0.y = ((r3.wwww)*(r3.zzzz)+(r0.yyyy)).y;
    // 159: rcp r0.y, r0.y
    r0.y = (1.0/(r0.yyyy)).y;
    // 160: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 161: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 162: mad r1.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r1.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 163: mad r1.xyz, r0.zzzz, r1.xyzx, r2.wwww
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r2.wwww)).xyz;
    // 164: add r0.xz, -r0.xxwx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = ((-(r0.xxwx))+(float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 165: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 166: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 167: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 168: mul_sat r0.w, r1.y, l(50.000000)
    r0.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 169: mul r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)*(r0.wwww)).w;
    // 170: max r3.xyz, r1.xyzx, r0.xxxx
    r3.xyz = (max(r1.xyzx,r0.xxxx)).xyz;
    // 171: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 172: mad r1.xyz, -r0.zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(r1.xyzx)).xyz;
    // 173: mad r0.xzw, r0.wwww, r3.xxyz, r1.xxyz
    r0.xzw = ((r0.wwww)*(r3.xxyz)+(r1.xxyz)).xzw;
    // 174: dp3 r1.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 175: add r1.x, r1.x, l(0.000100)
    r1.x = ((r1.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 176: mul r0.y, r0.y, l(0.500000)
    r0.y = ((r0.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 177: div r1.x, l(3.000000), r1.x
    r1.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r1.xxxx)).x;
    // 178: min r0.y, r0.y, r1.x
    r0.y = (min(r0.yyyy,r1.xxxx)).y;
    // 179: mul r1.xyz, r0.xzwx, r0.yyyy
    r1.xyz = ((r0.xzwx)*(r0.yyyy)).xyz;
    // 180: add r0.xyz, -r0.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xzwx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 182: mul r0.xyz, r3.wwww, r0.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)).xyz;
    // 183: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 184: mul r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r0.xyzx)).xyz;
    // 185: mul o0.xyz, r0.xyzx, cb0[14].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)).xyz;
    // 186: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 187: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 188: ret
    return output;
}

// source.character.static-map-native-218.v1 / source program 5e4ea15d52477b418db45417f48c7aef
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight218(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=float4(input.lightColor,1.0);
    source[11].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.yzxy
    r2.xyz = ((r0.xyzx)*(r1.yzxy)).xyz;
    // 8: mad r0.xyz, r0.zxyz, r1.zxyz, -r2.xyzx
    r0.xyz = ((r0.zxyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r0.xyz, r0.xyzx, v1.wwww
    r0.xyz = ((r0.xyzx)*(v1.wwww)).xyz;
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r6.xyzw, v4.xyxy, cb0[5].yyww
    r6.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r6.xy, r6.xyxx, cb0[5].zzzz
    r6.xy = ((r6.xyxx)*(source[5].zzzz)).xy;
    // 27: mad r4.xy, cb0[5].xxxx, r4.xyxx, r6.xyxx
    r4.xy = ((source[5].xxxx)*(r4.xyxx)+(r6.xyxx)).xy;
    // 28: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r4.xyw, r5.xyxz, r1.wwww
    r4.xyw = ((r5.xyxz)/(r1.wwww)).xyw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r6.zwzz, t2.xyzw, s3, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 33: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 34: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: mul r7.xy, r5.xyxx, cb0[6].xxxx
    r7.xy = ((r5.xyxx)*(source[6].xxxx)).xy;
    // 40: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 44: add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r6.x, r4.w, r4.w
    r6.x = ((r4.wwww)*(r4.wwww)).x;
    // 47: mul_sat r5.w, r5.w, r6.x
    r5.w = (saturate((r5.wwww)*(r6.xxxx))).w;
    // 48: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.zwzz, t4.xyzw, s5, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r6.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 51: mul r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)*(r6.wwww)).w;
    // 52: mul r6.w, r1.w, r5.w
    r6.w = ((r1.wwww)*(r5.wwww)).w;
    // 53: mad r3.w, r3.w, r6.w, r3.w
    r3.w = ((r3.wwww)*(r6.wwww)+(r3.wwww)).w;
    // 54: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 55: mul r6.w, r1.w, r2.w
    r6.w = ((r1.wwww)*(r2.wwww)).w;
    // 56: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 57: mad_sat r1.w, r5.w, r1.w, r6.w
    r1.w = (saturate((r5.wwww)*(r1.wwww)+(r6.wwww))).w;
    // 58: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 59: add r7.xyz, -r4.xywx, r7.xyzx
    r7.xyz = ((-(r4.xywx))+(r7.xyzx)).xyz;
    // 60: mad r7.xyz, r2.wwww, r7.xyzx, r4.xywx
    r7.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xywx)).xyz;
    // 61: dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r8.xyz, r2.wwww, r7.xyzx
    r8.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 64: dp3 r2.w, r8.xyzx, r2.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 65: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 66: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 67: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).w;
    // 68: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 69: div r9.xy, v8.xyxx, v8.wwww
    r9.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 70: mad r9.xy, r9.xyxx, cb2[0].xyxx, cb2[0].wzww
    r9.xy = ((r9.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 71: sample_indexable(texture2d)(float,float,float,float) r9.xyz, r9.xyxx, t7.xyzw, s0
    r9.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 72: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 73: else
    } else {
    // 74: mov r9.xyz, l(1.000000,1.000000,1.000000,0)
    r9.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 75: endif
    }
    // 76: dp3 r1.x, r1.xyzx, r8.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 77: dp3 r1.y, r0.xyzx, r8.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 78: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 79: mad r0.xy, cb0[6].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[6].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s6, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 81: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 82: mad r0.xyz, cb0[6].wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((source[6].wwww)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 83: add r0.xyz, r0.xyzx, -cb0[6].wwww
    r0.xyz = ((r0.xyzx)+(-(source[6].wwww))).xyz;
    // 84: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 85: mul r2.w, r4.z, cb0[7].x
    r2.w = ((r4.zzzz)*(source[7].xxxx)).w;
    // 86: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 87: add r8.xyz, -r5.xyzx, r3.wwww
    r8.xyz = ((-(r5.xyzx))+(r3.wwww)).xyz;
    // 88: mad r5.xyz, cb0[7].zzzz, r8.xyzx, r5.xyzx
    r5.xyz = ((source[7].zzzz)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 89: mul r8.xyz, cb0[2].xyzx, cb0[7].wwww
    r8.xyz = ((source[2].xyzx)*(source[7].wwww)).xyz;
    // 90: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 91: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 92: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 93: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 94: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 95: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 96: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 97: mul r1.xyz, cb0[3].xyzx, cb0[8].xxxx
    r1.xyz = ((source[3].xyzx)*(source[8].xxxx)).xyz;
    // 98: mul r5.xyz, r6.xyzx, r1.xyzx
    r5.xyz = ((r6.xyzx)*(r1.xyzx)).xyz;
    // 99: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r1.xyz, -r1.xyzx, r6.xyzx, r2.wwww
    r1.xyz = ((-(r1.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 101: mad r1.xyz, cb0[8].zzzz, r1.xyzx, r5.xyzx
    r1.xyz = ((source[8].zzzz)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 102: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 103: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 104: dp3 r1.x, r7.xyzx, r3.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 105: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 106: min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t5.xyzw, s7, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 108: mul r5.xyz, cb0[4].xyzx, cb0[8].wwww
    r5.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 109: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 110: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 111: add r5.xyz, -r6.xyzx, r1.zzzz
    r5.xyz = ((-(r6.xyzx))+(r1.zzzz)).xyz;
    // 112: mad r5.xyz, cb0[8].zzzz, r5.xyzx, r6.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 113: mad r5.xyz, cb0[9].xxxx, r5.xyzx, -r3.xyzx
    r5.xyz = ((source[9].xxxx)*(r5.xyzx)+(-(r3.xyzx))).xyz;
    // 114: mad r3.xyz, r1.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 115: mad r2.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 116: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 117: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 118: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 119: dp3 r0.w, r2.xyzx, r4.xywx
    r0.w = (dot((r2.xyzx).xyz,(r4.xywx).xyz).xxxx).w;
    // 120: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 121: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 122: mul r0.w, r0.w, cb0[9].y
    r0.w = ((r0.wwww)*(source[9].yyyy)).w;
    // 123: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 126: mul r2.xyz, r3.xyzx, r0.wwww
    r2.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 127: mad r0.xyz, r1.yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 128: mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // 129: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 130: mul r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r0.xyzx)).xyz;
    // 131: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 132: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 133: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 134: ret
    return output;
}

// source.character.static-map-native-219.v1 / source program 519b13b9860b0444a0dbf46b943a7fc1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight219(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[12]=float4(input.lightColor,1.0);
    source[13].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.yzxy
    r2.xyz = ((r0.xyzx)*(r1.yzxy)).xyz;
    // 8: mad r0.xyz, r0.zxyz, r1.zxyz, -r2.xyzx
    r0.xyz = ((r0.zxyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r0.xyz, r0.xyzx, v1.wwww
    r0.xyz = ((r0.xyzx)*(v1.wwww)).xyz;
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: mul r4.xy, v4.xyxx, cb0[1].xyxx
    r4.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 18: mad r4.zw, r5.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r5.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r4.zwzz, r4.zwzz
    r1.w = (dot((r4.zwzz).xy,(r4.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r5.xy, r4.xyxx, cb0[7].xxxx
    r5.xy = ((r4.xyxx)*(source[7].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r5.xy, r5.xyxx, cb0[7].yyyy
    r5.xy = ((r5.xyxx)*(source[7].yyyy)).xy;
    // 28: mad r4.zw, cb0[6].wwww, r4.zzzw, r5.xxxy
    r4.zw = ((source[6].wwww)*(r4.zzzw)+(r5.xxxy)).zw;
    // 29: mul r6.xy, r4.zwzz, v2.wwww
    r6.xy = ((r4.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 33: max r1.w, cb0[7].z, l(0.000000)
    r1.w = (max(source[7].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 35: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 37: add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul r4.z, r5.w, r5.w
    r4.z = ((r5.wwww)*(r5.wwww)).z;
    // 40: mul_sat r4.z, r4.z, r6.w
    r4.z = (saturate((r4.zzzz)*(r6.wwww))).z;
    // 41: add r4.z, -r4.z, l(1.000000)
    r4.z = ((-(r4.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 42: mul r7.xy, v4.xyxx, cb0[7].wwww
    r7.xy = ((v4.xyxx)*(source[7].wwww)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r4.w, r7.w, r7.w
    r4.w = ((r7.wwww)*(r7.wwww)).w;
    // 45: mul r4.z, r4.w, r4.z
    r4.z = ((r4.wwww)*(r4.zzzz)).z;
    // 46: mul r4.w, r1.w, r4.z
    r4.w = ((r1.wwww)*(r4.zzzz)).w;
    // 47: mad r3.w, r3.w, r4.w, r3.w
    r3.w = ((r3.wwww)*(r4.wwww)+(r3.wwww)).w;
    // 48: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 49: mul r4.w, r1.w, r2.w
    r4.w = ((r1.wwww)*(r2.wwww)).w;
    // 50: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 51: mad_sat r1.w, r4.z, r1.w, r4.w
    r1.w = (saturate((r4.zzzz)*(r1.wwww)+(r4.wwww))).w;
    // 52: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 53: add r8.xyz, -r5.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r5.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 54: mad r8.xyz, r2.wwww, r8.xyzx, r5.xywx
    r8.xyz = ((r2.wwww)*(r8.xyzx)+(r5.xywx)).xyz;
    // 55: dp3 r2.w, r8.xyzx, r8.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 56: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 57: mul r9.xyz, r2.wwww, r8.xyzx
    r9.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 58: dp3 r2.w, r9.xyzx, r2.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 59: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 60: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 61: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).w;
    // 62: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 63: div r4.zw, v8.xxxy, v8.wwww
    r4.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 64: mad r4.zw, r4.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r4.zw = ((r4.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 65: sample_indexable(texture2d)(float,float,float,float) r10.xyz, r4.zwzz, t6.xyzw, s0
    r10.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 66: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 67: else
    } else {
    // 68: mov r10.xyz, l(1.000000,1.000000,1.000000,0)
    r10.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 69: endif
    }
    // 70: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 71: dp3 r1.y, r0.xyzx, r9.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 72: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 73: mad r0.xy, cb0[8].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[8].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 74: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 75: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 76: mad r0.xyz, cb0[8].zzzz, r0.xyzx, r0.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 77: add r0.xyz, r0.xyzx, -cb0[8].zzzz
    r0.xyz = ((r0.xyzx)+(-(source[8].zzzz))).xyz;
    // 78: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 79: mul r2.w, r5.z, cb0[8].w
    r2.w = ((r5.zzzz)*(source[8].wwww)).w;
    // 80: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: add r9.xyz, -r6.xyzx, r3.wwww
    r9.xyz = ((-(r6.xyzx))+(r3.wwww)).xyz;
    // 82: mad r6.xyz, cb0[9].yyyy, r9.xyzx, r6.xyzx
    r6.xyz = ((source[9].yyyy)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 83: mul r9.xyz, cb0[3].xyzx, cb0[9].zzzz
    r9.xyz = ((source[3].xyzx)*(source[9].zzzz)).xyz;
    // 84: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 85: mad r1.xyz, r2.wwww, r1.xyzx, r6.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 86: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 87: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 88: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 89: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 91: mul r1.xyz, cb0[4].xyzx, cb0[9].wwww
    r1.xyz = ((source[4].xyzx)*(source[9].wwww)).xyz;
    // 92: mul r6.xyz, r7.xyzx, r1.xyzx
    r6.xyz = ((r7.xyzx)*(r1.xyzx)).xyz;
    // 93: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: mad r1.xyz, -r1.xyzx, r7.xyzx, r2.wwww
    r1.xyz = ((-(r1.xyzx))*(r7.xyzx)+(r2.wwww)).xyz;
    // 95: mad r1.xyz, cb0[10].yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((source[10].yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 96: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 97: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 98: dp3 r1.x, r8.xyzx, r3.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 99: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 100: min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 101: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r4.xyxx, t4.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 102: mul r4.xyz, cb0[5].xyzx, cb0[10].zzzz
    r4.xyz = ((source[5].xyzx)*(source[10].zzzz)).xyz;
    // 103: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 104: dp3 r1.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 105: add r4.xyz, -r7.xyzx, r1.zzzz
    r4.xyz = ((-(r7.xyzx))+(r1.zzzz)).xyz;
    // 106: mad r4.xyz, cb0[10].yyyy, r4.xyzx, r7.xyzx
    r4.xyz = ((source[10].yyyy)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 107: mad r4.xyz, cb0[10].wwww, r4.xyzx, -r3.xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(-(r3.xyzx))).xyz;
    // 108: mad r3.xyz, r1.wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 109: mad r2.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 110: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 111: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 112: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 113: dp3 r0.w, r2.xyzx, r5.xywx
    r0.w = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // 114: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 115: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 116: mul r0.w, r0.w, cb0[11].x
    r0.w = ((r0.wwww)*(source[11].xxxx)).w;
    // 117: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 118: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 120: mul r2.xyz, r3.xyzx, r0.wwww
    r2.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 121: mad r0.xyz, r1.yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 122: mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // 123: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 124: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 125: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 126: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 127: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 128: ret
    return output;
}

// source.character.static-map-native-220.v1 / source program c6db0a1eb0687643ac94b106b20f21b4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight220(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=float4(input.lightColor,1.0);
    source[12].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r5.zw, v4.xxxy, cb0[5].yyyy
    r5.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.zwzz, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 32: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 36: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 37: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 38: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 39: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 40: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 41: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 42: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mad r0.x, r0.x, l(0.500000), cb0[6].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).x;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 46: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 47: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 48: mul r1.xy, v4.xyxx, cb0[6].wwww
    r1.xy = ((v4.xyxx)*(source[6].wwww)).xy;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 51: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 52: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 53: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 54: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 55: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 56: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 57: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 58: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 59: add r1.xyz, -r5.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r5.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 60: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 61: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 64: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 65: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 66: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 67: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 68: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 69: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 70: else
    } else {
    // 71: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 72: endif
    }
    // 73: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 75: add r8.xyz, -r6.xyzx, r0.yyyy
    r8.xyz = ((-(r6.xyzx))+(r0.yyyy)).xyz;
    // 76: mad r6.xyz, cb0[7].yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((source[7].yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 77: mul r8.xyz, cb0[3].xyzx, cb0[7].zzzz
    r8.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 78: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 79: mul r10.xyz, cb0[4].xyzx, cb0[7].wwww
    r10.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 80: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 81: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 83: mad r7.xyz, cb0[8].yyyy, r7.xyzx, r11.xyzx
    r7.xyz = ((source[8].yyyy)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 84: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 85: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 86: mul r6.xyz, r0.xyzx, cb0[8].zzzz
    r6.xyz = ((r0.xyzx)*(source[8].zzzz)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 88: mul r1.w, r7.y, cb0[9].x
    r1.w = ((r7.yyyy)*(source[9].xxxx)).w;
    // 89: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 91: mul r1.w, r1.w, cb0[9].y
    r1.w = ((r1.wwww)*(source[9].yyyy)).w;
    // 92: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 93: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 94: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r0.xyz, cb0[8].wwww, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].wwww)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 96: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 97: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 98: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 99: mov_sat r2.w, cb0[9].z
    r2.w = (saturate(source[9].zzzz)).w;
    // 100: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 101: mul r3.w, r7.x, cb0[10].x
    r3.w = ((r7.xxxx)*(source[10].xxxx)).w;
    // 102: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 103: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 104: mul r3.w, r3.w, cb0[10].y
    r3.w = ((r3.wwww)*(source[10].yyyy)).w;
    // 105: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 106: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 107: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 108: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 110: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 111: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 112: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 113: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 114: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 115: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 116: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 118: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 119: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 122: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 123: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 125: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 126: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 127: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 128: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 129: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 130: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 131: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 132: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 133: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 134: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 135: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 136: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 137: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 138: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 139: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 140: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 141: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 142: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 143: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 145: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 146: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 147: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 148: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 149: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 151: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 152: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 153: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 154: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 155: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 156: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 157: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 158: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 159: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 160: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 162: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 163: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 164: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 165: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 166: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 167: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 168: ret
    return output;
}

// source.character.static-map-native-221.v1 / source program 680a6e84edc57d4a8fb05bc11ac16785
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight221(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=float4(input.lightColor,1.0);
    source[12].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r5.zw, v4.xxxy, cb0[5].yyyy
    r5.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.zwzz, t1.zwxy, s2, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r5.zw, r5.zzzw, cb0[5].zzzz
    r5.zw = ((r5.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r5.xy, cb0[5].xxxx, r5.xyxx, r5.zwzz
    r5.xy = ((source[5].xxxx)*(r5.xyxx)+(r5.zwzz)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 32: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 36: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 37: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 38: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 39: mul r0.xy, cb0[0].xyxx, cb0[6].xxxx
    r0.xy = ((source[0].xyxx)*(source[6].xxxx)).xy;
    // 40: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 43: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 44: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: mad r0.x, r0.x, l(0.500000), cb0[6].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].wwww)).x;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 48: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 49: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: mul r1.xy, v4.xyxx, cb0[7].xxxx
    r1.xy = ((v4.xyxx)*(source[7].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 53: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 54: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 55: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 56: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 57: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 58: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 59: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 60: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 61: add r1.xyz, -r5.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r5.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 63: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 64: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 65: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 66: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 67: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 68: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 69: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 70: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 71: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 72: else
    } else {
    // 73: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 74: endif
    }
    // 75: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 77: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 78: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 79: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 80: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 81: mad r7.xyz, -r10.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 82: mad r7.xyz, cb0[8].xxxx, r7.xyzx, r11.xyzx
    r7.xyz = ((source[8].xxxx)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 83: mad r6.xyz, -r6.xyzx, r8.xyzx, r7.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 84: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 85: mul r6.xyz, r0.xyzx, cb0[8].yyyy
    r6.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 87: mul r1.w, r7.y, cb0[8].w
    r1.w = ((r7.yyyy)*(source[8].wwww)).w;
    // 88: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 90: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 91: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 92: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 93: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 95: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 96: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 97: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 98: mov_sat r2.w, cb0[9].y
    r2.w = (saturate(source[9].yyyy)).w;
    // 99: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 100: mul r3.w, r7.x, cb0[9].w
    r3.w = ((r7.xxxx)*(source[9].wwww)).w;
    // 101: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 102: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 103: mul r3.w, r3.w, cb0[10].x
    r3.w = ((r3.wwww)*(source[10].xxxx)).w;
    // 104: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 105: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 106: max r3.w, r3.w, cb0[1].x
    r3.w = (max(r3.wwww,source[1].xxxx)).w;
    // 107: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 109: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 110: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 111: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 112: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 113: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 114: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 115: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 117: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 118: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 121: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 122: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 124: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 125: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 126: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 127: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 128: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 129: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 130: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 131: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 132: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 133: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 134: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 135: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 136: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 137: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 138: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 139: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 140: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 141: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 142: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 144: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 145: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 146: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 147: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 148: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 150: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 151: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 152: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 153: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 154: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 155: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 156: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 157: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 158: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 159: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 161: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 162: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 163: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 164: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 165: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 166: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 167: ret
    return output;
}

// source.character.static-map-native-222.v1 / source program b6da3a17038f4649bf5950e3b7fb955e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight222(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[3]=float4(input.lightColor,1.0);
    source[4].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[4].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[4].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s0
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: add r0.w, r2.w, l(-0.333300)
    r0.w = ((r2.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 15: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 16: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t2.zwxy, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 18: mad r4.xyz, cb0[2].yyyy, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[2].yyyy)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 19: mad r3.yzw, r3.yyyy, r4.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r3.yzw = ((r3.yyyy)*(r4.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 20: mul r2.xyz, r2.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r3.yzwy)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 22: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 23: dp2 r0.w, r3.yzyy, r3.yzyy
    r0.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 24: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 27: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: mul r4.xy, r3.yzyy, cb0[2].xxxx
    r4.xy = ((r3.yzyy)*(source[2].xxxx)).xy;
    // 29: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 30: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 31: div r3.yzw, r4.xxyz, r0.wwww
    r3.yzw = ((r4.xxyz)/(r0.wwww)).yzw;
    // 32: dp3 r0.w, r3.yzwy, r1.xyzx
    r0.w = (dot((r3.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 33: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.x, r0.w, l(1.000000)
    r1.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mul r1.xyz, r0.xyzx, r1.xxxx
    r1.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 36: mul r3.yzw, cb0[1].xxyz, cb0[2].zzzz
    r3.yzw = ((source[1].xxyz)*(source[2].zzzz)).yzw;
    // 37: mul r3.xyz, r3.yzwy, r3.xxxx
    r3.xyz = ((r3.yzwy)*(r3.xxxx)).xyz;
    // 38: mad r0.xyz, r3.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 39: mad r0.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 40: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 41: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 42: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 43: mul o0.xyz, r0.xyzx, cb0[3].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[3].xyzx)).xyz;
    // 44: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 45: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 46: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 47: ret
    return output;
}

// source.character.static-map-native-223.v1 / source program 21068ef5e2d13b4f9c415657645112d7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight223(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[9]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[12]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[13].z=(g_SourceCharacterTime.xxxx).x;
    source[14].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[14].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))).x;
    source[16]=float4(input.lightColor,1.0);
    source[17].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: add r0.xyz, v6.xyzx, cb0[0].xyzx
    r0.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 2: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 3: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 4: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 5: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r2.xyz, r0.wwww, v3.xyzx
    r2.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // 8: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[17].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[17].xxxx)) * 0xffffffffu)).w;
    // 9: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 10: mul r3.xyz, v6.yyyy, cb1[1].xywx
    r3.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 11: mad r3.xyz, cb1[0].xywx, v6.xxxx, r3.xyzx
    r3.xyz = ((projection[0].xywx)*(v6.xxxx)+(r3.xyzx)).xyz;
    // 12: mad r3.xyz, cb1[2].xywx, v6.zzzz, r3.xyzx
    r3.xyz = ((projection[2].xywx)*(v6.zzzz)+(r3.xyzx)).xyz;
    // 13: mad r3.xyz, cb1[3].xywx, v6.wwww, r3.xyzx
    r3.xyz = ((projection[3].xywx)*(v6.wwww)+(r3.xyzx)).xyz;
    // 14: div r3.xy, r3.xyxx, r3.zzzz
    r3.xy = ((r3.xyxx)/(r3.zzzz)).xy;
    // 15: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 17: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 18: else
    } else {
    // 19: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 20: endif
    }
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r5.xyz, r4.xyzx, cb0[3].xyzx
    r5.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // 23: mad r4.xyz, -r4.xyzx, cb0[3].xyzx, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(source[3].xyzx)+(r4.xyzx)).xyz;
    // 24: mad r4.xyz, r4.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 25: add r5.xyz, r0.xyzx, -cb0[1].xyzx
    r5.xyz = ((r0.xyzx)+(-(source[1].xyzx))).xyz;
    // 26: mul r0.w, cb0[2].w, cb0[13].x
    r0.w = ((source[2].wwww)*(source[13].xxxx)).w;
    // 27: div r0.w, r0.w, r5.z
    r0.w = ((r0.wwww)/(r5.zzzz)).w;
    // 28: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: mad r5.zw, |r0.wwww|, r5.xxxy, -r5.xxxy
    r5.zw = ((abs(r0.wwww))*(r5.xxxy)+(-(r5.xxxy))).zw;
    // 30: mad r5.xy, cb0[13].yyyy, r5.zwzz, r5.xyxx
    r5.xy = ((source[13].yyyy)*(r5.zwzz)+(r5.xyxx)).xy;
    // 31: div r5.xy, r5.xyxx, cb0[2].wwww
    r5.xy = ((r5.xyxx)/(source[2].wwww)).xy;
    // 32: mul r5.zw, cb0[4].xxxy, cb0[13].zzzz
    r5.zw = ((source[4].xxxy)*(source[13].zzzz)).zw;
    // 33: mul r5.zw, r5.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 34: mad r5.zw, r5.xxxy, l(0.000000, 0.000000, -0.750000, -0.750000), r5.zzzw
    r5.zw = ((r5.xxxy)*(float4(0.000000,0.000000,-0.750000,-0.750000))+(r5.zzzw)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r5.zwzz, t1.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mad r5.xy, cb0[13].zzzz, cb0[4].xyxx, r5.xyxx
    r5.xy = ((source[13].zzzz)*(source[4].xyxx)+(r5.xyxx)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: add r7.xyzw, -r6.xyzw, r5.xyzw
    r7.xyzw = ((-(r6.xyzw))+(r5.xyzw)).xyzw;
    // 39: mad r5.xyzw, r5.wwww, r7.xyzw, r6.xyzw
    r5.xyzw = ((r5.wwww)*(r7.xyzw)+(r6.xyzw)).xyzw;
    // 40: add r0.w, v2.y, l(-0.500000)
    r0.w = ((v2.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).w;
    // 41: add_sat r0.w, r0.w, r0.w
    r0.w = (saturate((r0.wwww)+(r0.wwww))).w;
    // 42: lt r1.w, r0.w, l(0.000001)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 43: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 44: mul r0.w, r0.w, cb0[13].w
    r0.w = ((r0.wwww)*(source[13].wwww)).w;
    // 45: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 46: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 47: mul r0.w, r5.w, r0.w
    r0.w = ((r5.wwww)*(r0.wwww)).w;
    // 48: mad r5.xyz, r5.xyzx, cb0[5].xyzx, -r4.xyzx
    r5.xyz = ((r5.xyzx)*(source[5].xyzx)+(-(r4.xyzx))).xyz;
    // 49: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 50: dp3 r0.w, cb0[6].xyzx, cb0[6].xyzx
    r0.w = (dot((source[6].xyzx).xyz,(source[6].xyzx).xyz).xxxx).w;
    // 51: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 52: div r5.xyz, cb0[6].xyzx, r0.wwww
    r5.xyz = ((source[6].xyzx)/(r0.wwww)).xyz;
    // 53: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 54: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 55: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 56: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 57: dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 58: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 59: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 60: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 61: mul r0.x, r0.x, cb0[14].x
    r0.x = ((r0.xxxx)*(source[14].xxxx)).x;
    // 62: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 63: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 64: mov r5.xw, l(1.000000,0,0,2.000000)
    r5.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 65: mov r5.yz, cb0[7].yyxy
    r5.yz = (source[7].yyxy).yz;
    // 66: mul r0.yz, r5.xxyx, v2.xxyx
    r0.yz = ((r5.xxyx)*(v2.xxyx)).yz;
    // 67: mad r5.xy, r5.zwzz, r0.yzyy, cb0[8].xyxx
    r5.xy = ((r5.zwzz)*(r0.yzyy)+(source[8].xyxx)).xy;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 69: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 70: dp2 r0.w, r6.xyxx, r6.xyxx
    r0.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 71: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 73: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 74: add r6.z, r0.w, l(0.000010)
    r6.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 75: mul r7.xy, v2.xyxx, cb0[7].xyxx
    r7.xy = ((v2.xyxx)*(source[7].xyxx)).xy;
    // 76: mad r7.xy, r7.xyxx, l(-1.000000, 2.000000, 0.000000, 0.000000), cb0[9].xyxx
    r7.xy = ((r7.xyxx)*(float4(-1.000000,2.000000,0.000000,0.000000))+(source[9].xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r7.zw, r7.xyxx, t2.zwxy, s3, l(0.000000)
    r7.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 78: mad r8.xy, r7.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 79: dp2 r0.w, r8.xyxx, r8.xyxx
    r0.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 80: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 82: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 83: add r8.z, r0.w, l(0.000010)
    r8.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 84: add r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)+(r8.xyzx)).xyz;
    // 85: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 86: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 87: div r9.xyz, r9.xyzx, r0.wwww
    r9.xyz = ((r9.xyzx)/(r0.wwww)).xyz;
    // 88: dp3 r0.w, r1.xyzx, r9.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 89: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: mad r0.w, -r0.w, l(0.500000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: mul r1.xyz, r0.wwww, cb0[10].xyzx
    r1.xyz = ((r0.wwww)*(source[10].xyzx)).xyz;
    // 92: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 93: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 94: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r5.xyxx, t3.xyzw, s4, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 95: mad r7.yzw, r9.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r7.xxyz
    r7.yzw = ((r9.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r7.xxyz)).yzw;
    // 96: mul r7.yzw, r7.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r7.yzw = ((r7.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 97: mad r0.yz, r5.zzwz, r0.yyzy, cb0[12].xxyx
    r0.yz = ((r5.zzwz)*(r0.yyzy)+(source[12].xxyx)).yz;
    // 98: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s4, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 99: mad_sat r0.yzw, r0.yyzw, r7.yyzw, r7.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r7.yyzw)+(r7.yyzw))).yzw;
    // 100: add r5.xyz, -r6.xyzx, r8.xyzx
    r5.xyz = ((-(r6.xyzx))+(r8.xyzx)).xyz;
    // 101: mad r5.xyz, r7.xxxx, r5.xyzx, r6.xyzx
    r5.xyz = ((r7.xxxx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 102: dp3 r1.w, r2.xyzx, r5.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 103: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 104: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 105: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 106: mul r2.xyw, r0.yzyw, r1.wwww
    r2.xyw = ((r0.yzyw)*(r1.wwww)).xyw;
    // 107: add r0.z, cb0[11].w, l(-0.030000)
    r0.z = ((source[11].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 108: mad r2.xyw, r2.xyxw, r0.zzzz, l(0.030000, 0.030000, 0.000000, 0.030000)
    r2.xyw = ((r2.xyxw)*(r0.zzzz)+(float4(0.030000,0.030000,0.000000,0.030000))).xyw;
    // 109: mul r2.xyw, r2.xyxw, cb0[11].xyxz
    r2.xyw = ((r2.xyxw)*(source[11].xyxz)).xyw;
    // 110: mad r0.xzw, r0.xxxx, r1.xxyz, r2.xxyw
    r0.xzw = ((r0.xxxx)*(r1.xxyz)+(r2.xxyw)).xzw;
    // 111: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 112: add r1.x, -r4.w, l(0.200000)
    r1.x = ((-(r4.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 113: mul_sat r1.x, r1.x, l(5.000000)
    r1.x = (saturate((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 114: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 115: add r0.xzw, -r4.xxyz, r0.xxzw
    r0.xzw = ((-(r4.xxyz))+(r0.xxzw)).xzw;
    // 116: mad r0.xyz, r0.yyyy, r0.xzwx, r4.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r4.xyzx)).xyz;
    // 117: mul r0.xyz, r0.xyzx, cb0[15].yyyy
    r0.xyz = ((r0.xyzx)*(source[15].yyyy)).xyz;
    // 118: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 119: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 120: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 121: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 122: mul o0.xyz, r0.xyzx, cb0[16].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[16].xyzx)).xyz;
    // 123: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 124: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 125: ret
    return output;
}

// source.character.static-map-native-224.v1 / source program a54c39add512e849b1b65e7fc6b1ba55
