SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked214(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[27]=1.f; source[28]=1.f; // RNM samples already contain their instance scales.
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[8].wwww
    r0.xy = ((source[0].xyxx)*(source[8].wwww)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: mul r1.xy, v4.xyxx, cb0[3].xyxx
    r1.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 6: mul r1.zw, r1.xxxy, cb0[8].xxxx
    r1.zw = ((r1.xxxy)*(source[8].xxxx)).zw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 8: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 9: mul r1.zw, r1.zzzw, cb0[8].yyyy
    r1.zw = ((r1.zzzw)*(source[8].yyyy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 11: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mad r1.zw, cb0[7].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 13: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 26: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 37: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 41: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: mul r0.zw, v4.xxxy, cb0[9].wwww
    r0.zw = ((v4.xxxy)*(source[9].wwww)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 50: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 53: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r0.yzw, -r4.xxyz, r0.yyyy
    r0.yzw = ((-(r4.xxyz))+(r0.yyyy)).yzw;
    // 58: mad r0.yzw, cb0[10].yyyy, r0.yyzw, r4.xxyz
    r0.yzw = ((source[10].yyyy)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 59: mul r4.xyz, cb0[6].xyzx, cb0[10].wwww
    r4.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 60: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 61: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 63: mad r4.xyz, cb0[11].yyyy, r4.xyzx, r8.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 64: mul r7.xyz, cb0[5].xyzx, cb0[10].zzzz
    r7.xyz = ((source[5].xyzx)*(source[10].zzzz)).xyz;
    // 65: mad r4.xyz, -r0.yzwy, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.yzwy))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 66: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 67: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 68: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 69: mul r4.xyz, r0.yzwy, cb0[11].zzzz
    r4.xyz = ((r0.yzwy)*(source[11].zzzz)).xyz;
    // 70: mad r0.yzw, cb0[11].wwww, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].wwww)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 71: mul r1.z, r1.z, cb0[12].x
    r1.z = ((r1.zzzz)*(source[12].xxxx)).z;
    // 72: mul r1.xy, r1.yxyy, cb0[13].xzxx
    r1.xy = ((r1.yxyy)*(source[13].xzxx)).xy;
    // 73: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 74: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 75: mul r1.w, r1.w, cb0[12].y
    r1.w = ((r1.wwww)*(source[12].yyyy)).w;
    // 76: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 77: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 78: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 80: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 81: add r4.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 83: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 85: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 86: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 87: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 88: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 89: mul r1.zw, r1.zzzw, cb0[13].yyyw
    r1.zw = ((r1.zzzw)*(source[13].yyyw)).zw;
    // 90: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 91: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 93: max r1.x, r1.x, cb0[1].x
    r1.x = (max(r1.xxxx,source[1].xxxx)).x;
    // 94: min r7.z, r1.x, l(1.000000)
    r7.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: mad r1.xzw, r1.yyyy, r8.xxyz, r9.xxyz
    r1.xzw = ((r1.yyyy)*(r8.xxyz)+(r9.xxyz)).xzw;
    // 96: mad r0.yzw, r1.xxzw, r1.yyyy, r0.yyzw
    r0.yzw = ((r1.xxzw)*(r1.yyyy)+(r0.yyzw)).yzw;
    // 97: mul r0.yzw, r1.yyyy, r0.yyzw
    r0.yzw = ((r1.yyyy)*(r0.yyzw)).yzw;
    // 98: max r0.yzw, r0.yyzw, r1.yyyy
    r0.yzw = (max(r0.yyzw,r1.yyyy)).yzw;
    // 99: add r1.xzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 100: mad r1.xzw, r0.xxxx, r1.xxzw, r2.xxyz
    r1.xzw = ((r0.xxxx)*(r1.xxzw)+(r2.xxyz)).xzw;
    // 101: dp3 r0.x, r1.xzwx, r1.xzwx
    r0.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 102: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 103: mul r2.xyz, r0.xxxx, r1.xzwx
    r2.xyz = ((r0.xxxx)*(r1.xzwx)).xyz;
    // 104: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 105: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 106: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 107: dp3 r0.x, r8.xyzx, r2.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 108: mad r7.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 110: mul r8.xyz, r7.yyyy, cb0[25].xyzx
    r8.xyz = ((r7.yyyy)*(source[25].xyzx)).xyz;
    // 111: mad r8.xyz, r7.xxxx, cb0[24].xyzx, r8.xyzx
    r8.xyz = ((r7.xxxx)*(source[24].xyzx)+(r8.xyzx)).xyz;
    // 112: mul r8.xyz, r8.xyzx, cb0[26].wwww
    r8.xyz = ((r8.xyzx)*(source[26].wwww)).xyz;
    // 113: mul r9.xyz, r4.xyzx, r8.xyzx
    r9.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 114: dp2_sat r10.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 115: dp3_sat r10.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 116: dp3_sat r10.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 117: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 118: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 119: mul r11.xyz, r11.xyzx, cb0[28].xyzx
    r11.xyz = ((r11.xyzx)*(source[28].xyzx)).xyz;
    // 120: dp3 r0.x, r11.xyzx, r10.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 121: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[27].xyzx
    r10.xyz = ((r10.xyzx)*(source[27].xyzx)).xyz;
    // 123: mul r12.xyz, r0.xxxx, r10.xyzx
    r12.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 124: mad r9.xyz, r4.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 125: mul r0.yzw, r0.yyzw, r9.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)).yzw;
    // 126: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 127: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 128: dp2 r12.z, r9.xyxx, cb0[15].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 129: dp3 r12.y, r5.xyzx, r2.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 130: mul r7.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r12.x, r9.xyxx, r7.xyxx
    r12.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 132: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 133: dp4 r13.x, cb0[16].xyzw, r12.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[17].xyzw, r12.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[18].xyzw, r12.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 137: dp4 r15.x, cb0[19].xyzw, r14.xyzw
    r15.x = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 138: dp4 r15.y, cb0[20].xyzw, r14.xyzw
    r15.y = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 139: dp4 r15.z, cb0[21].xyzw, r14.xyzw
    r15.z = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 140: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 141: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 142: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 143: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 144: mad r12.xyz, cb0[22].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[22].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 145: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 146: mul r12.xyz, r12.xyzx, cb0[14].xyzx
    r12.xyz = ((r12.xyzx)*(source[14].xyzx)).xyz;
    // 147: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 148: mov_sat r4.w, cb0[12].z
    r4.w = (saturate(source[12].zzzz)).w;
    // 149: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 150: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 151: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 152: mad r13.xyz, r7.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 153: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 156: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r3.w, r2.xyzx, r14.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 158: mul r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // 159: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 160: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 161: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 162: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 164: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 165: mad_sat r15.y, r4.w, l(0.300000), r7.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 166: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 168: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 169: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 170: add r2.w, r2.z, l(1.000000)
    r2.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 173: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 174: add r2.w, r1.y, r15.x
    r2.w = ((r1.yyyy)+(r15.xxxx)).w;
    // 175: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 176: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 177: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 178: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = ((((r15.wwww) != 0.f ? 1.f : 0.f) / ((r15.wwww) != 0.f ? (r15.wwww) : 1.f))).w;
    // 179: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 180: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 181: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 182: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 183: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 185: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 186: mul r0.yzw, r0.yyzw, r12.xxyz
    r0.yzw = ((r0.yyzw)*(r12.xxyz)).yzw;
    // 187: mad r0.yzw, -r0.yyzw, r7.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r7.wwww)+(r0.yyzw)).yzw;
    // 188: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 189: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 190: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 191: dp2 r6.x, r3.xyxx, r7.xyxx
    r6.x = (dot((r3.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 192: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 193: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 194: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 195: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 196: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 197: add r2.w, r1.y, r2.w
    r2.w = ((r1.yyyy)+(r2.wwww)).w;
    // 198: mov o5.y, r1.y
    output.targets[5].y = (r1.yyyy).y;
    // 199: add_sat r1.y, r2.w, l(-1.000000)
    r1.y = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 200: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 201: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s6, r3.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 202: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 203: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 204: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 205: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 206: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 207: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 208: mul r2.xyz, r5.xyzx, r5.xyzx
    r2.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 209: dp3 r2.x, r11.xyzx, r2.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 210: add r0.x, r0.x, -r2.x
    r0.x = ((r0.xxxx)+(-(r2.xxxx))).x;
    // 211: mad r0.x, r7.z, r0.x, r2.x
    r0.x = ((r7.zzzz)*(r0.xxxx)+(r2.xxxx)).x;
    // 212: mad r2.xyz, r10.xyzx, r0.xxxx, r8.xyzx
    r2.xyz = ((r10.xyzx)*(r0.xxxx)+(r8.xyzx)).xyz;
    // 213: mul r5.xyz, r0.xxxx, r10.xyzx
    r5.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 214: mad r0.x, r1.y, r13.x, r13.y
    r0.x = ((r1.yyyy)*(r13.xxxx)+(r13.yyyy)).x;
    // 215: mad r0.x, r0.x, r1.y, r13.z
    r0.x = ((r0.xxxx)*(r1.yyyy)+(r13.zzzz)).x;
    // 216: mul r0.x, r1.y, r0.x
    r0.x = ((r1.yyyy)*(r0.xxxx)).x;
    // 217: max r0.x, r0.x, r1.y
    r0.x = (max(r0.xxxx,r1.yyyy)).x;
    // 218: mul r6.xyz, r0.xxxx, r2.xyzx
    r6.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 219: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 220: div r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)/(r2.xyzx)).xyz;
    // 221: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 222: mul r2.xyz, r3.xyzx, r6.xyzx
    r2.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 223: mad r0.yzw, r2.xxyz, r15.xxzw, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r15.xxzw)+(r0.yyzw)).yzw;
    // 224: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 225: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: dp3 r1.x, r1.xzwx, r14.xyzx
    r1.x = (dot((r1.xzwx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 227: add r1.y, -|r14.z|, l(1.000000)
    r1.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 228: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 229: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 230: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 231: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 232: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 233: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 234: mul r1.xzw, r1.xxxx, cb0[4].xxyz
    r1.xzw = ((r1.xxxx)*(source[4].xxyz)).xzw;
    // 235: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 236: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 237: add r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 238: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 239: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 240: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 241: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 242: mul r1.xyz, r1.xxxx, r9.xyzx
    r1.xyz = ((r1.xxxx)*(r9.xyzx)).xyz;
    // 243: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 244: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 245: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 246: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 247: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 248: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 249: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 250: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 251: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 252: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 253: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 254: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 255: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 256: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 257: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 258: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 259: ret
    return output;
}

// source.character.static-map-native-214.v1 / source program d32603cde11930488c1f00c966e50220
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase214(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7].x=g_SourceCharacterBaseConstants[7].x;
    source[7].y=g_SourceCharacterBaseConstants[7].y;
    source[7].z=g_SourceCharacterBaseConstants[7].z;
    source[7].w=g_SourceCharacterBaseConstants[7].w;
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[9].x=g_SourceCharacterBaseConstants[9].x;
    source[9].y=g_SourceCharacterBaseConstants[9].y;
    source[9].z=g_SourceCharacterBaseConstants[9].z;
    source[9].w=g_SourceCharacterBaseConstants[9].w;
    source[10].x=g_SourceCharacterBaseConstants[10].x;
    source[10].y=g_SourceCharacterBaseConstants[10].y;
    source[10].z=g_SourceCharacterBaseConstants[10].z;
    source[10].w=g_SourceCharacterBaseConstants[10].w;
    source[11].x=g_SourceCharacterBaseConstants[11].x;
    source[11].y=g_SourceCharacterBaseConstants[11].y;
    source[11].z=g_SourceCharacterBaseConstants[11].z;
    source[11].w=g_SourceCharacterBaseConstants[11].w;
    source[12].x=g_SourceCharacterBaseConstants[12].x;
    source[12].y=g_SourceCharacterBaseConstants[12].y;
    source[12].z=g_SourceCharacterBaseConstants[12].z;
    source[12].w=g_SourceCharacterBaseConstants[12].w;
    source[13].x=g_SourceCharacterBaseConstants[13].x;
    source[13].y=g_SourceCharacterBaseConstants[13].y;
    source[13].z=g_SourceCharacterBaseConstants[13].z;
    source[13].w=g_SourceCharacterBaseConstants[13].w;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[8].wwww
    r0.xy = ((source[0].xyxx)*(source[8].wwww)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: mul r1.xy, v4.xyxx, cb0[3].xyxx
    r1.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 6: mul r1.zw, r1.xxxy, cb0[8].xxxx
    r1.zw = ((r1.xxxy)*(source[8].xxxx)).zw;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 8: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 9: mul r1.zw, r1.zzzw, cb0[8].yyyy
    r1.zw = ((r1.zzzw)*(source[8].yyyy)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 11: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mad r1.zw, cb0[7].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 13: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 26: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 28: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 37: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 40: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 41: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 42: mul r0.zw, v4.xxxy, cb0[9].wwww
    r0.zw = ((v4.xxxy)*(source[9].wwww)).zw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 50: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 53: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r0.yzw, -r4.xxyz, r0.yyyy
    r0.yzw = ((-(r4.xxyz))+(r0.yyyy)).yzw;
    // 58: mad r0.yzw, cb0[10].yyyy, r0.yyzw, r4.xxyz
    r0.yzw = ((source[10].yyyy)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 59: mul r4.xyz, cb0[6].xyzx, cb0[10].wwww
    r4.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 60: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 61: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 63: mad r4.xyz, cb0[11].yyyy, r4.xyzx, r8.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 64: mul r7.xyz, cb0[5].xyzx, cb0[10].zzzz
    r7.xyz = ((source[5].xyzx)*(source[10].zzzz)).xyz;
    // 65: mad r4.xyz, -r0.yzwy, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.yzwy))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 66: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 67: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 68: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 69: mul r4.xyz, r0.yzwy, cb0[11].zzzz
    r4.xyz = ((r0.yzwy)*(source[11].zzzz)).xyz;
    // 70: mad r0.yzw, cb0[11].wwww, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].wwww)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 71: mul r1.z, r1.z, cb0[12].x
    r1.z = ((r1.zzzz)*(source[12].xxxx)).z;
    // 72: mul r1.xy, r1.yxyy, cb0[13].xzxx
    r1.xy = ((r1.yxyy)*(source[13].xzxx)).xy;
    // 73: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 74: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 75: mul r1.w, r1.w, cb0[12].y
    r1.w = ((r1.wwww)*(source[12].yyyy)).w;
    // 76: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 77: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 78: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 80: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 81: add r4.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 83: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 85: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 86: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 87: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 88: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 89: mul r1.zw, r1.zzzw, cb0[13].yyyw
    r1.zw = ((r1.zzzw)*(source[13].yyyw)).zw;
    // 90: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 91: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 93: max r1.x, r1.x, cb0[1].x
    r1.x = (max(r1.xxxx,source[1].xxxx)).x;
    // 94: min r7.z, r1.x, l(1.000000)
    r7.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: mad r1.xzw, r1.yyyy, r8.xxyz, r9.xxyz
    r1.xzw = ((r1.yyyy)*(r8.xxyz)+(r9.xxyz)).xzw;
    // 96: mad r0.yzw, r1.xxzw, r1.yyyy, r0.yyzw
    r0.yzw = ((r1.xxzw)*(r1.yyyy)+(r0.yyzw)).yzw;
    // 97: mul r0.yzw, r1.yyyy, r0.yyzw
    r0.yzw = ((r1.yyyy)*(r0.yyzw)).yzw;
    // 98: max r0.yzw, r0.yyzw, r1.yyyy
    r0.yzw = (max(r0.yyzw,r1.yyyy)).yzw;
    // 99: add r1.xzw, -r2.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.xzw = ((-(r2.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).xzw;
    // 100: mad r1.xzw, r0.xxxx, r1.xxzw, r2.xxyz
    r1.xzw = ((r0.xxxx)*(r1.xxzw)+(r2.xxyz)).xzw;
    // 101: dp3 r0.x, r1.xzwx, r1.xzwx
    r0.x = (dot((r1.xzwx).xyz,(r1.xzwx).xyz).xxxx).x;
    // 102: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 103: mul r2.xyz, r0.xxxx, r1.xzwx
    r2.xyz = ((r0.xxxx)*(r1.xzwx)).xyz;
    // 104: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 105: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 106: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 107: dp3 r0.x, r8.xyzx, r2.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 108: mad r7.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 110: mul r9.xyz, r7.yyyy, cb0[25].xyzx
    r9.xyz = ((r7.yyyy)*(source[25].xyzx)).xyz;
    // 111: mad r9.xyz, r7.xxxx, cb0[24].xyzx, r9.xyzx
    r9.xyz = ((r7.xxxx)*(source[24].xyzx)+(r9.xyzx)).xyz;
    // 112: mul r9.xyz, r9.xyzx, cb0[26].wwww
    r9.xyz = ((r9.xyzx)*(source[26].wwww)).xyz;
    // 113: mul r9.xyz, r4.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 114: mul r0.xyz, r0.yzwy, r9.xyzx
    r0.xyz = ((r0.yzwy)*(r9.xyzx)).xyz;
    // 115: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 116: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 117: dp2 r10.z, r9.xyxx, cb0[15].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 118: dp3 r10.y, r5.xyzx, r2.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 119: mul r7.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 120: dp2 r10.x, r9.xyxx, r7.xyxx
    r10.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 121: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 122: dp4 r11.x, cb0[16].xyzw, r10.xyzw
    r11.x = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 123: dp4 r11.y, cb0[17].xyzw, r10.xyzw
    r11.y = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 124: dp4 r11.z, cb0[18].xyzw, r10.xyzw
    r11.z = (dot((source[18].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 125: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 126: dp4 r13.x, cb0[19].xyzw, r12.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 127: dp4 r13.y, cb0[20].xyzw, r12.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 128: dp4 r13.z, cb0[21].xyzw, r12.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 129: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 130: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 131: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 132: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 133: mad r10.xyz, cb0[22].xyzx, r0.wwww, r11.xyzx
    r10.xyz = ((source[22].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 134: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 136: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 137: mov_sat r4.w, cb0[12].z
    r4.w = (saturate(source[12].zzzz)).w;
    // 138: mad r11.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r11.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 139: mul r0.w, r4.w, l(0.080000)
    r0.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 140: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 141: mad r11.xyz, r7.wwww, r11.xyzx, r0.wwww
    r11.xyz = ((r7.wwww)*(r11.xyzx)+(r0.wwww)).xyz;
    // 142: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 143: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 144: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 145: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 146: dp3 r2.w, r2.xyzx, r12.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 147: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 148: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 149: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 150: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 151: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 153: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 154: mad_sat r13.y, r3.w, l(0.300000), r7.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 155: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 156: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 158: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 159: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 160: add r0.w, r2.z, l(1.000000)
    r0.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: add_sat r13.x, -r0.w, r2.w
    r13.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 163: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 164: add r0.w, r1.y, r13.x
    r0.w = ((r1.yyyy)+(r13.xxxx)).w;
    // 165: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 166: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 167: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 168: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = ((((r13.wwww) != 0.f ? 1.f : 0.f) / ((r13.wwww) != 0.f ? (r13.wwww) : 1.f))).w;
    // 169: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 170: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 171: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 172: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 173: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 175: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 176: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 177: mad r0.xyz, -r0.xyzx, r7.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r7.wwww)+(r0.xyzx)).xyz;
    // 178: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 179: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 180: dp2 r6.x, r3.xyxx, r7.xyxx
    r6.x = (dot((r3.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 181: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 182: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 183: mul r3.x, r13.y, r13.y
    r3.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 184: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 185: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 186: add r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)+(r0.wwww)).w;
    // 187: mov o5.y, r1.y
    output.targets[5].y = (r1.yyyy).y;
    // 188: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 189: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 190: dp3 r1.y, r8.xyzx, r2.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 191: mad r2.xy, r1.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 192: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 193: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s5, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 194: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 195: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 196: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 197: mad r1.y, r0.w, r11.x, r11.y
    r1.y = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).y;
    // 198: mad r1.y, r1.y, r0.w, r11.z
    r1.y = ((r1.yyyy)*(r0.wwww)+(r11.zzzz)).y;
    // 199: mul r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)*(r1.yyyy)).y;
    // 200: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 201: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 202: mad r2.xyz, cb0[24].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[24].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 203: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 204: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 205: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 206: mad r0.xyz, r2.xyzx, r13.xzwx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r13.xzwx)+(r0.xyzx)).xyz;
    // 207: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 208: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 209: dp3 r0.w, r1.xzwx, r12.xyzx
    r0.w = (dot((r1.xzwx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 210: add r1.x, -|r12.z|, l(1.000000)
    r1.x = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 211: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 212: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 213: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 214: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 215: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 216: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 217: mul r1.yzw, r0.wwww, cb0[4].xxyz
    r1.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 218: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 219: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 220: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 221: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 222: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 223: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 225: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 226: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 227: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 228: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 229: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 230: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 231: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 232: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 233: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 234: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 235: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 236: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 237: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 238: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 239: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 240: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 241: ret
    return output;
}

// source.character.static-map-native-215.v1 / source program ff6ba42b8c2fcd46acbf122c14751516
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked215(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[9]=1.f; source[10]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 7: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t2.zwxy, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 9: mul r0.w, r2.y, cb0[4].z
    r0.w = ((r2.yyyy)*(source[4].zzzz)).w;
    // 10: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 11: mad r1.xyz, cb0[4].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = ((source[4].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 12: mad r1.xyz, r2.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((r2.yyyy)*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 13: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 14: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 15: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 19: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 20: dp2 r0.w, r2.yzyy, r2.yzyy
    r0.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 21: mul r3.xy, r2.yzyy, cb0[4].xxxx
    r3.xy = ((r2.yzyy)*(source[4].xxxx)).xy;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 25: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 27: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 28: div r2.yzw, r3.xxyz, r0.wwww
    r2.yzw = ((r3.xxyz)/(r0.wwww)).yzw;
    // 29: dp3 r0.w, r2.yzwy, r2.yzwy
    r0.w = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r2.yzw, r0.wwww, r2.yyzw
    r2.yzw = ((r0.wwww)*(r2.yyzw)).yzw;
    // 32: dp3 r0.w, r1.xyzx, r2.yzwy
    r0.w = (dot((r1.xyzx).xyz,(r2.yzwy).xyz).xxxx).w;
    // 33: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 34: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 35: mul r3.xyz, cb0[3].xyzx, cb0[5].zzzz
    r3.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 36: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.yyyy
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 37: mul r5.xyz, r2.xxxx, r3.xyzx
    r5.xyz = ((r2.xxxx)*(r3.xyzx)).xyz;
    // 38: mad r1.yzw, r5.xxyz, r4.xxyz, r1.yyyy
    r1.yzw = ((r5.xxyz)*(r4.xxyz)+(r1.yyyy)).yzw;
    // 39: mul r1.yzw, r1.yyzw, cb0[7].xxyz
    r1.yzw = ((r1.yyzw)*(source[7].xxyz)).yzw;
    // 40: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.xxxx
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 41: mad r4.xyz, r5.xyzx, r4.xyzx, r1.xxxx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.xxxx)).xyz;
    // 42: mad r1.xyz, r4.xyzx, cb0[6].xyzx, r1.yzwy
    r1.xyz = ((r4.xyzx)*(source[6].xyzx)+(r1.yzwy)).xyz;
    // 43: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 44: mad r3.xyz, -r2.xxxx, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r2.xxxx))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 46: dp2_sat r6.x, r2.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 47: dp3_sat r6.y, r2.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 48: dp3_sat r6.z, r2.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 49: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 50: mad r3.xyz, r6.xyzx, r3.xyzx, r5.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 51: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t5.xyzw, s4
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 52: mul r5.xyz, r5.xyzx, cb0[10].xyzx
    r5.xyz = ((r5.xyzx)*(source[10].xyzx)).xyz;
    // 53: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 54: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s4
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 55: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // 56: mul r6.xyz, r0.wwww, r3.xyzx
    r6.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 57: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 58: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 59: div r1.xyz, r6.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)/(r1.xyzx)).xyz;
    // 60: mad r4.xyz, r0.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 61: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 63: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 64: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 65: dp3 r1.w, r2.yzwy, r1.xyzx
    r1.w = (dot((r2.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 66: mul r6.xyz, r1.wwww, r2.yzwy
    r6.xyz = ((r1.wwww)*(r2.yzwy)).xyz;
    // 67: mad r1.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 68: dp2_sat r6.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 69: dp3_sat r6.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 70: dp3_sat r6.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 71: log r1.xyz, r6.xyzx
    r1.xyz = (log2(r6.xyzx)).xyz;
    // 72: add r1.w, cb0[5].y, l(1.000000)
    r1.w = ((source[5].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 74: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 75: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, v4.xyxx, t3.wxyz, s3, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 77: mul r1.yzw, r1.yyzw, cb0[2].xxyz
    r1.yzw = ((r1.yyzw)*(source[2].xxyz)).yzw;
    // 78: mul r1.yzw, r1.yyzw, cb0[5].xxxx
    r1.yzw = ((r1.yyzw)*(source[5].xxxx)).yzw;
    // 79: mad r1.yzw, r1.yyzw, cb2[4].wwww, cb2[4].xxyz
    r1.yzw = ((r1.yyzw)*(passValues[4].wwww)+(passValues[4].xxyz)).yzw;
    // 80: mul r1.yzw, r3.xxyz, r1.yyzw
    r1.yzw = ((r3.xxyz)*(r1.yyzw)).yzw;
    // 81: mad r3.xyz, r1.yzwy, r1.xxxx, r4.xyzx
    r3.xyz = ((r1.yzwy)*(r1.xxxx)+(r4.xyzx)).xyz;
    // 82: mul r1.xyz, r1.xxxx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 83: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 84: add r1.xyz, r3.xyzx, cb0[0].xyzx
    r1.xyz = ((r3.xyzx)+(source[0].xyzx)).xyz;
    // 85: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r1.xyzx)).xyz;
    // 86: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 87: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 88: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 89: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 90: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 91: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 92: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 93: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 94: mul r4.xyz, r0.zxyz, r1.yzxy
    r4.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 95: mad r4.xyz, r0.yzxy, r1.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r4.xyzx))).xyz;
    // 96: dp3 r0.z, r0.xyzx, r2.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r2.yzwy).xyz).xxxx).z;
    // 97: dp3 r0.x, r1.xyzx, r2.yzwy
    r0.x = (dot((r1.xyzx).xyz,(r2.yzwy).xyz).xxxx).x;
    // 98: mul r1.xyz, r4.xyzx, v1.wwww
    r1.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 99: dp3 r0.y, r1.xyzx, r2.yzwy
    r0.y = (dot((r1.xyzx).xyz,(r2.yzwy).xyz).xxxx).y;
    // 100: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 101: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 102: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 103: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 104: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 105: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 106: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 107: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 108: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 109: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 110: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 111: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 112: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 113: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 114: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 115: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 116: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 117: ret
    return output;
}

// source.character.static-map-native-215.v1 / source program 6449403c587a784295a7fad9dd9ae108
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase215(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[3];
    source[3].x=g_SourceCharacterBaseConstants[4].x;
    source[3].y=g_SourceCharacterBaseConstants[4].y;
    source[3].z=g_SourceCharacterBaseConstants[4].z;
    source[3].w=g_SourceCharacterBaseConstants[4].w;
    source[4].x=g_SourceCharacterBaseConstants[5].x;
    source[4].y=g_SourceCharacterBaseConstants[5].y;
    source[4].z=g_SourceCharacterBaseConstants[5].z;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r3.xyz, cb0[2].xyzx, cb0[4].zzzz
    r3.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 28: mad r4.xyz, r1.zzzz, r3.xyzx, -r1.yyyy
    r4.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r5.xyz, r3.xyzx, r1.zzzz
    r5.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 30: mad r3.xyz, r1.zzzz, r3.xyzx, -r1.xxxx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 31: mad r3.xyz, r5.xyzx, r3.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 32: mad r1.xyz, r5.xyzx, r4.xyzx, r1.yyyy
    r1.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.yyyy)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[6].xyzx
    r1.xyz = ((r1.xyzx)*(source[6].xyzx)).xyz;
    // 34: mad r1.xyz, r3.xyzx, cb0[5].xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(source[5].xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
    // 36: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r3.xyz, -r0.xyzx, r0.wwww
    r3.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 38: mul r0.w, r1.w, cb0[3].z
    r0.w = ((r1.wwww)*(source[3].zzzz)).w;
    // 39: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 40: mad r3.xyz, cb0[3].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = ((source[3].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 41: mad r3.xyz, r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 43: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 44: mad r3.xyz, r1.xyzx, r0.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // 45: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 46: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 47: mad o0.xyz, r0.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 48: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 49: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 50: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 51: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 52: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 53: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 54: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 55: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 56: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 57: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 58: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 59: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 60: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 61: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 62: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 63: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 64: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 65: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 66: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 67: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 68: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 69: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 70: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 71: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 72: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 74: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 75: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 76: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 77: ret
    return output;
}

// source.character.static-map-native-216.v1 / source program 96bdf1e22a898146b69437ede6b94162
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked216(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[29]=1.f; source[30]=1.f; // RNM samples already contain their instance scales.
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 3: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 4: mad r0.xyz, cb0[11].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[11].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r1.xyz, cb0[6].xyzx, cb0[11].wwww
    r1.xyz = ((source[6].xyzx)*(source[11].wwww)).xyz;
    // 6: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 7: mul r1.xy, cb0[1].xyxx, cb0[9].zzzz
    r1.xy = ((source[1].xyxx)*(source[9].zzzz)).xy;
    // 8: max r1.xy, -r1.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = (max(-(r1.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: min r1.xy, r1.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r1.xy = (min(r1.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 10: mov r1.z, l(1.000000)
    r1.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 12: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mul r1.w, r2.z, cb0[11].x
    r1.w = ((r2.zzzz)*(source[11].xxxx)).w;
    // 14: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 15: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 16: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 17: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 18: add r3.z, r2.z, l(0.000010)
    r3.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 19: mul r4.xyzw, v4.xyxy, cb0[8].yyww
    r4.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r4.xyxx, t1.zwxy, s1, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 21: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 22: mul r2.zw, r2.zzzw, cb0[8].zzzz
    r2.zw = ((r2.zzzw)*(source[8].zzzz)).zw;
    // 23: mad r2.xy, cb0[8].xxxx, r2.xyxx, r2.zwzz
    r2.xy = ((source[8].xxxx)*(r2.xyxx)+(r2.zwzz)).xy;
    // 24: mul r3.xy, r2.xyxx, v2.wwww
    r3.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r2.x, r3.xyzx, r3.xyzx
    r2.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 26: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 27: div r2.xyz, r3.xyzx, r2.xxxx
    r2.xyz = ((r3.xyzx)/(r2.xxxx)).xyz;
    // 28: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 29: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 30: mul r3.xyz, r2.wwww, v0.xyzx
    r3.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 31: dp3 r5.x, r3.xyzx, r2.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 32: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 33: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 34: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 35: dp3 r5.z, r6.xyzx, r2.xyzx
    r5.z = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 36: mul r7.xyz, r3.yzxy, r6.zxyz
    r7.xyz = ((r3.yzxy)*(r6.zxyz)).xyz;
    // 37: mad r7.xyz, r6.yzxy, r3.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r3.zxyz)+(-(r7.xyzx))).xyz;
    // 38: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 39: dp3 r5.y, r7.xyzx, r2.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 40: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 41: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 42: mad r1.x, r1.x, l(0.500000), cb0[10].y
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 43: mul r1.y, r2.z, r2.z
    r1.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 44: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 45: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r4.zwzz, t4.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r4.zwzz, t2.zxyw, s2, l(0.000000)
    r1.yz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 48: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 49: mul r2.w, r5.w, r5.w
    r2.w = ((r5.wwww)*(r5.wwww)).w;
    // 50: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 51: max r2.w, cb0[9].y, l(0.000000)
    r2.w = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 53: mul r3.w, r0.w, r2.w
    r3.w = ((r0.wwww)*(r2.wwww)).w;
    // 54: mad r1.x, r1.x, r3.w, r1.x
    r1.x = ((r1.xxxx)*(r3.wwww)+(r1.xxxx)).x;
    // 55: add r3.w, -r2.w, r1.x
    r3.w = ((-(r2.wwww))+(r1.xxxx)).w;
    // 56: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((((r2.wwww) != 0.f ? 1.f : 0.f) / ((r2.wwww) != 0.f ? (r2.wwww) : 1.f))).w;
    // 58: mad r1.x, -r2.w, r3.w, r1.x
    r1.x = ((-(r2.wwww))*(r3.wwww)+(r1.xxxx)).x;
    // 59: mul r2.w, r3.w, r2.w
    r2.w = ((r3.wwww)*(r2.wwww)).w;
    // 60: mad_sat r0.w, r0.w, r1.x, r2.w
    r0.w = (saturate((r0.wwww)*(r1.xxxx)+(r2.wwww))).w;
    // 61: mul r1.x, r0.w, l(0.650000)
    r1.x = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 62: dp2 r2.w, r1.yzyy, r1.yzyy
    r2.w = (dot((r1.yzyy).xy,(r1.yzyy).xy).xxxx).w;
    // 63: mul r4.xy, r1.yzyy, cb0[9].xxxx
    r4.xy = ((r1.yzyy)*(source[9].xxxx)).xy;
    // 64: add r1.y, -r2.w, l(1.000000)
    r1.y = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 66: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 67: add r4.z, r1.y, l(0.000010)
    r4.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 68: add r4.xyz, -r2.xyzx, r4.xyzx
    r4.xyz = ((-(r2.xyzx))+(r4.xyzx)).xyz;
    // 69: mad r1.xyz, r1.xxxx, r4.xyzx, r2.xyzx
    r1.xyz = ((r1.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 70: dp3 r2.x, r1.xyzx, r1.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 71: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 72: mul r2.xyz, r1.xyzx, r2.xxxx
    r2.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 73: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 74: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 75: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 76: dp3 r2.w, r2.xyzx, r4.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 77: mul r8.xyz, r2.wwww, r2.xyzx
    r8.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 78: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 79: dp3 r9.x, r3.xyzx, r8.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 80: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 81: dp3 r9.y, r7.xyzx, r8.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 82: dp3 r3.y, r7.xyzx, r2.xyzx
    r3.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 83: mul r7.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r7.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 84: mad r7.xy, cb0[10].zzzz, r9.xyxx, r7.xyxx
    r7.xy = ((source[10].zzzz)*(r9.xyxx)+(r7.xyxx)).xy;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 86: mul r7.xyz, r7.xyzx, cb0[5].xyzx
    r7.xyz = ((r7.xyzx)*(source[5].xyzx)).xyz;
    // 87: mad r7.xyz, cb0[10].wwww, r7.xyzx, r7.xyzx
    r7.xyz = ((source[10].wwww)*(r7.xyzx)+(r7.xyzx)).xyz;
    // 88: add r7.xyz, r7.xyzx, -cb0[10].wwww
    r7.xyz = ((r7.xyzx)+(-(source[10].wwww))).xyz;
    // 89: mov_sat r10.xyz, r7.xyzx
    r10.xyz = (saturate(r7.xyzx)).xyz;
    // 90: mov_sat r7.xyz, -r7.xyzx
    r7.xyz = (saturate(-(r7.xyzx))).xyz;
    // 91: mad r7.xyz, -r1.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.wwww))*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mad r0.xyz, r1.wwww, r10.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r10.xyzx)+(r0.xyzx)).xyz;
    // 93: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 94: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 95: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 96: mul r7.xyz, cb0[7].xyzx, cb0[12].xxxx
    r7.xyz = ((source[7].xyzx)*(source[12].xxxx)).xyz;
    // 97: mul r10.xyz, r5.xyzx, r7.xyzx
    r10.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 98: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: mad r5.xyz, -r7.xyzx, r5.xyzx, r1.wwww
    r5.xyz = ((-(r7.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 100: mad r5.xyz, cb0[12].zzzz, r5.xyzx, r10.xyzx
    r5.xyz = ((source[12].zzzz)*(r5.xyzx)+(r10.xyzx)).xyz;
    // 101: add r5.xyz, -r0.xyzx, r5.xyzx
    r5.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 102: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 103: mul r5.xyz, r0.xyzx, cb0[12].wwww
    r5.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 104: mad r0.xyz, cb0[13].xxxx, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 106: mul r0.w, r7.z, cb0[13].y
    r0.w = ((r7.zzzz)*(source[13].yyyy)).w;
    // 107: mul r7.xy, r7.yxyy, cb0[14].ywyy
    r7.xy = ((r7.yxyy)*(source[14].ywyy)).xy;
    // 108: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 109: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 110: mul r1.w, r1.w, cb0[13].z
    r1.w = ((r1.wwww)*(source[13].zzzz)).w;
    // 111: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 112: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 113: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 115: mad r0.xyz, r1.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 116: add r5.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 117: mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 118: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 119: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 120: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 121: mul r5.xyz, r1.wwww, v6.xyzx
    r5.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 122: dp3 r1.w, r5.xyzx, r2.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 123: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 124: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 125: mul r5.yzw, r5.yyyy, cb0[27].xxyz
    r5.yzw = ((r5.yyyy)*(source[27].xxyz)).yzw;
    // 126: mad r5.xyz, r5.xxxx, cb0[26].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[26].xyzx)+(r5.yzwy)).xyz;
    // 127: mul r5.xyz, r5.xyzx, cb0[28].wwww
    r5.xyz = ((r5.xyzx)*(source[28].wwww)).xyz;
    // 128: mul r10.xyz, r0.xyzx, r5.xyzx
    r10.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 129: dp2_sat r11.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r11.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 130: dp3_sat r11.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r11.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 131: dp3_sat r11.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r11.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 132: dp3 r12.y, r6.xyzx, r2.xyzx
    r12.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 133: dp3 r2.y, r6.xyzx, r8.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 134: mul r6.xyz, r11.xyzx, r11.xyzx
    r6.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 135: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t10.xyzw, s7
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 136: mul r11.xyz, r11.xyzx, cb0[30].xyzx
    r11.xyz = ((r11.xyzx)*(source[30].xyzx)).xyz;
    // 137: dp3 r1.w, r11.xyzx, r6.xyzx
    r1.w = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 138: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t9.xyzw, s7
    r6.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 139: mul r6.xyz, r6.xyzx, cb0[29].xyzx
    r6.xyz = ((r6.xyzx)*(source[29].xyzx)).xyz;
    // 140: mul r13.xyz, r1.wwww, r6.xyzx
    r13.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 141: mad r10.xyz, r0.xyzx, r13.xyzx, r10.xyzx
    r10.xyz = ((r0.xyzx)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 142: mad r13.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 143: mad r14.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 144: mad r15.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 145: log r9.zw, |r7.xxxy|
    r9.zw = (log2(abs(r7.xxxy))).zw;
    // 146: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 147: mul r3.w, r9.w, cb0[15].x
    r3.w = ((r9.wwww)*(source[15].xxxx)).w;
    // 148: mul r4.w, r9.z, cb0[14].z
    r4.w = ((r9.zzzz)*(source[14].zzzz)).w;
    // 149: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 150: movc r4.w, r7.x, l(0), r4.w
    r4.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 151: max r4.w, r4.w, cb0[2].x
    r4.w = (max(r4.wwww,source[2].xxxx)).w;
    // 152: min r7.z, r4.w, l(1.000000)
    r7.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 153: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 154: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: movc r3.w, r7.y, l(0), r3.w
    r3.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 156: mad r14.xyz, r3.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 157: mad r13.xyz, r14.xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 158: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 159: max r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = (max(r3.wwww,r13.xyzx)).xyz;
    // 160: mul r10.xyz, r10.xyzx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r13.xyzx)).xyz;
    // 161: dp2 r12.z, r3.xyxx, cb0[17].xyxx
    r12.z = (dot((r3.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 162: mul r7.xy, cb0[17].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[17].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 163: dp2 r12.x, r3.xyxx, r7.xyxx
    r12.x = (dot((r3.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 164: dp2 r2.x, r9.xyxx, r7.xyxx
    r2.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 165: dp2 r2.z, r9.xyxx, cb0[17].xyxx
    r2.z = (dot((r9.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 166: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 167: dp4 r9.x, cb0[18].xyzw, r12.xyzw
    r9.x = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 168: dp4 r9.y, cb0[19].xyzw, r12.xyzw
    r9.y = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 169: dp4 r9.z, cb0[20].xyzw, r12.xyzw
    r9.z = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 170: mul r13.xyzw, r12.yzzx, r12.xyzz
    r13.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 171: dp4 r14.x, cb0[21].xyzw, r13.xyzw
    r14.x = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 172: dp4 r14.y, cb0[22].xyzw, r13.xyzw
    r14.y = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 173: dp4 r14.z, cb0[23].xyzw, r13.xyzw
    r14.z = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 174: add r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = ((r9.xyzx)+(r14.xyzx)).xyz;
    // 175: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 176: mov r3.z, r12.y
    r3.z = (r12.yyyy).z;
    // 177: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 178: mad r9.xyz, cb0[24].xyzx, r4.wwww, r9.xyzx
    r9.xyz = ((source[24].xyzx)*(r4.wwww)+(r9.xyzx)).xyz;
    // 179: max r9.xyz, r9.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 180: mul r9.xyz, r9.xyzx, cb0[16].xyzx
    r9.xyz = ((r9.xyzx)*(source[16].xyzx)).xyz;
    // 181: mad r9.xyz, r9.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r9.xyz = ((r9.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 182: deriv_rtx_coarse r7.x, r2.w
    r7.x = (ddx_coarse(r2.wwww)).x;
    // 183: deriv_rty_coarse r7.y, r2.w
    r7.y = (ddy_coarse(r2.wwww)).y;
    // 184: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: dp2 r4.w, r7.xyxx, r7.xyxx
    r4.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 186: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 187: mad_sat r7.y, r4.w, l(0.300000), r7.z
    r7.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 188: add r4.w, r8.z, l(1.000000)
    r4.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: add_sat r7.x, r2.w, -r4.w
    r7.x = (saturate((r2.wwww)+(-(r4.wwww)))).x;
    // 191: sample_indexable(texture2d)(float,float,float,float) r12.xy, r7.xyxx, t7.xyzw, s9
    r12.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 192: add r2.w, r3.w, r7.x
    r2.w = ((r3.wwww)+(r7.xxxx)).w;
    // 193: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 194: mov_sat r0.w, cb0[13].w
    r0.w = (saturate(source[13].wwww)).w;
    // 195: mad r13.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r13.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 196: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 197: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 198: mad r13.xyz, r7.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 199: mul_sat r0.w, r13.y, l(50.000000)
    r0.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 200: add r4.w, -r7.y, l(1.000000)
    r4.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: max r14.xyz, r13.xyzx, r4.wwww
    r14.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 202: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 203: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 204: mul r15.xyz, r12.yyyy, r13.xyzx
    r15.xyz = ((r12.yyyy)*(r13.xyzx)).xyz;
    // 205: mad r12.xzw, r14.xxyz, r12.xxxx, r15.xxyz
    r12.xzw = ((r14.xxyz)*(r12.xxxx)+(r15.xxyz)).xzw;
    // 206: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.y
    r0.w = ((((r12.yyyy) != 0.f ? 1.f : 0.f) / ((r12.yyyy) != 0.f ? (r12.yyyy) : 1.f))).w;
    // 207: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 208: mad r14.xyz, r13.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r13.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: dp3 r0.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: mad r13.xyz, r0.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r0.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 211: mad r15.xyz, -r12.xzwx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r12.xzwx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 212: mul r12.xyz, r12.xzwx, r14.xyzx
    r12.xyz = ((r12.xzwx)*(r14.xyzx)).xyz;
    // 213: mul r9.xyz, r9.xyzx, r15.xyzx
    r9.xyz = ((r9.xyzx)*(r15.xyzx)).xyz;
    // 214: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 215: mad r9.xyz, -r9.xyzx, r7.wwww, r9.xyzx
    r9.xyz = ((-(r9.xyzx))*(r7.wwww)+(r9.xyzx)).xyz;
    // 216: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 217: dp2_sat r10.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 218: dp3_sat r10.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 219: dp3_sat r10.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 220: mul r8.xyz, r10.xyzx, r10.xyzx
    r8.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 221: dp3 r0.w, r11.xyzx, r8.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 222: add r1.w, -r0.w, r1.w
    r1.w = ((-(r0.wwww))+(r1.wwww)).w;
    // 223: mad r0.w, r7.z, r1.w, r0.w
    r0.w = ((r7.zzzz)*(r1.wwww)+(r0.wwww)).w;
    // 224: mad r5.xyz, r6.xyzx, r0.wwww, r5.xyzx
    r5.xyz = ((r6.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 225: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 226: mul r0.w, r7.y, r7.y
    r0.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 227: mul r1.w, r7.y, l(5.000000)
    r1.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 228: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r2.xyzx, t8.xyzw, s8, r1.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 229: mul r2.xyz, r7.xyzx, r7.wwww
    r2.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 230: mul r2.xyz, r2.xyzx, cb0[16].xyzx
    r2.xyz = ((r2.xyzx)*(source[16].xyzx)).xyz;
    // 231: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 232: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 233: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 234: add r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)+(r0.wwww)).w;
    // 235: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 236: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 237: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 238: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 239: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 240: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 241: mul r7.xyz, r0.wwww, r5.xyzx
    r7.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 242: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 243: div r5.xyz, r6.xyzx, r5.xyzx
    r5.xyz = ((r6.xyzx)/(r5.xyzx)).xyz;
    // 244: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 245: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 246: mad r5.xyz, r2.xyzx, r12.xyzx, r9.xyzx
    r5.xyz = ((r2.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 247: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 248: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 249: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 250: add r1.y, -|r4.z|, l(1.000000)
    r1.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 251: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 252: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 253: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 254: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 255: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 256: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 257: mul r1.xzw, r1.xxxx, cb0[4].xxyz
    r1.xzw = ((r1.xxxx)*(source[4].xxyz)).xzw;
    // 258: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 259: add r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(source[3].xyzx)).xyz;
    // 260: add r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)+(r1.xyzx)).xyz;
    // 261: mad o0.xyz, r0.xyzx, cb0[28].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[28].xyzx)+(r1.xyzx)).xyz;
    // 262: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 263: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 264: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 265: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 266: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 267: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 268: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 269: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 270: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 271: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 272: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 273: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 274: mul o4.z, r0.w, r5.x
    output.targets[4].z = ((r0.wwww)*(r5.xxxx)).z;
    // 275: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 276: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 277: ftou r0.x, cb0[25].z
    r0.x = (asfloat((uint4)(source[25].zzzz))).x;
    // 278: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 279: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 280: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 281: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 282: ret
    return output;
}

// source.character.static-map-native-216.v1 / source program b7b825359e2df541bd7ee56448d24725
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase216(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[9].x=g_SourceCharacterBaseConstants[9].x;
    source[9].y=g_SourceCharacterBaseConstants[9].y;
    source[9].z=g_SourceCharacterBaseConstants[9].z;
    source[9].w=g_SourceCharacterBaseConstants[9].w;
    source[10].x=g_SourceCharacterBaseConstants[10].x;
    source[10].y=g_SourceCharacterBaseConstants[10].y;
    source[10].z=g_SourceCharacterBaseConstants[10].z;
    source[10].w=g_SourceCharacterBaseConstants[10].w;
    source[11].x=g_SourceCharacterBaseConstants[11].x;
    source[11].y=g_SourceCharacterBaseConstants[11].y;
    source[11].z=g_SourceCharacterBaseConstants[11].z;
    source[11].w=g_SourceCharacterBaseConstants[11].w;
    source[12].x=g_SourceCharacterBaseConstants[12].x;
    source[12].y=g_SourceCharacterBaseConstants[12].y;
    source[12].z=g_SourceCharacterBaseConstants[12].z;
    source[12].w=g_SourceCharacterBaseConstants[12].w;
    source[13].x=g_SourceCharacterBaseConstants[13].x;
    source[13].y=g_SourceCharacterBaseConstants[13].y;
    source[13].z=g_SourceCharacterBaseConstants[13].z;
    source[13].w=g_SourceCharacterBaseConstants[13].w;
    source[14].x=g_SourceCharacterBaseConstants[14].x;
    source[14].y=g_SourceCharacterBaseConstants[14].y;
    source[14].z=g_SourceCharacterBaseConstants[14].z;
    source[14].w=g_SourceCharacterBaseConstants[14].w;
    source[15].x=g_SourceCharacterBaseConstants[15].x;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: mul r0.xyzw, v4.xyxy, cb0[8].yyww
    r0.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 5: mul r2.xy, r1.xyxx, cb0[9].xxxx
    r2.xy = ((r1.xyxx)*(source[9].xxxx)).xy;
    // 6: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 9: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mul r0.xy, r0.xyxx, cb0[8].zzzz
    r0.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 15: mad r0.zw, r3.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r3.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 16: mul r2.w, r3.z, cb0[11].x
    r2.w = ((r3.zzzz)*(source[11].xxxx)).w;
    // 17: mad r0.xy, cb0[8].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[8].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 18: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 19: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 20: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 21: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 22: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r3.xy, r0.xyxx, v2.wwww
    r3.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 24: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 25: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 26: div r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 27: add r2.xyz, -r0.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xyzx))+(r2.xyzx)).xyz;
    // 28: mul r3.xy, cb0[1].xyxx, cb0[9].zzzz
    r3.xy = ((source[1].xyxx)*(source[9].zzzz)).xy;
    // 29: max r3.xy, -r3.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = (max(-(r3.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: min r3.xy, r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = (min(r3.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 31: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 32: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 33: mul r4.xyz, r0.wwww, v0.xyzx
    r4.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 34: dp3 r5.x, r4.xyzx, r0.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 36: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 37: mul r6.xyz, r0.wwww, v1.xyzx
    r6.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 38: dp3 r5.z, r6.xyzx, r0.xyzx
    r5.z = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 39: mul r7.xyz, r4.yzxy, r6.zxyz
    r7.xyz = ((r4.yzxy)*(r6.zxyz)).xyz;
    // 40: mad r7.xyz, r6.yzxy, r4.zxyz, -r7.xyzx
    r7.xyz = ((r6.yzxy)*(r4.zxyz)+(-(r7.xyzx))).xyz;
    // 41: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 42: dp3 r5.y, r7.xyzx, r0.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 43: mov r3.zw, l(0,0,1.000000,1.000000)
    r3.zw = (float4(asfloat(0u),asfloat(0u),1.000000,1.000000)).zw;
    // 44: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 45: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: mad r0.w, r0.w, l(0.500000), cb0[10].y
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).w;
    // 47: mul r3.x, r0.z, r0.z
    r3.x = ((r0.zzzz)*(r0.zzzz)).x;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 49: mul_sat r3.x, r3.x, r5.w
    r3.x = (saturate((r3.xxxx)*(r5.wwww))).x;
    // 50: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 52: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 53: max r3.x, cb0[9].y, l(0.000000)
    r3.x = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 54: min r3.x, r3.x, l(0.990000)
    r3.x = (min(r3.xxxx,float4(0.990000,0.990000,0.990000,0.990000))).x;
    // 55: mul r3.y, r1.w, r3.x
    r3.y = ((r1.wwww)*(r3.xxxx)).y;
    // 56: mad r0.w, r0.w, r3.y, r0.w
    r0.w = ((r0.wwww)*(r3.yyyy)+(r0.wwww)).w;
    // 57: add r3.y, -r3.x, r0.w
    r3.y = ((-(r3.xxxx))+(r0.wwww)).y;
    // 58: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 59: div r3.x, l(1.000000, 1.000000, 1.000000, 1.000000), r3.x
    r3.x = ((((r3.xxxx) != 0.f ? 1.f : 0.f) / ((r3.xxxx) != 0.f ? (r3.xxxx) : 1.f))).x;
    // 60: mad r0.w, -r3.x, r3.y, r0.w
    r0.w = ((-(r3.xxxx))*(r3.yyyy)+(r0.wwww)).w;
    // 61: mul r3.x, r3.y, r3.x
    r3.x = ((r3.yyyy)*(r3.xxxx)).x;
    // 62: mad_sat r0.w, r1.w, r0.w, r3.x
    r0.w = (saturate((r1.wwww)*(r0.wwww)+(r3.xxxx))).w;
    // 63: mul r1.w, r0.w, l(0.650000)
    r1.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 64: mad r0.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 65: dp3 r1.w, r0.xyzx, r0.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 67: mul r2.xyz, r0.xyzx, r1.wwww
    r2.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 68: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 69: dp3 r8.x, r4.xyzx, r2.xyzx
    r8.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 70: dp3 r8.y, r7.xyzx, r2.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 71: dp2 r3.z, r8.xyxx, cb0[17].xyxx
    r3.z = (dot((r8.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 72: mul r9.xy, cb0[17].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((source[17].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 73: dp2 r3.x, r8.xyxx, r9.xyxx
    r3.x = (dot((r8.xyxx).xy,(r9.xyxx).xy).xxxx).x;
    // 74: dp4 r10.x, cb0[18].xyzw, r3.xyzw
    r10.x = (dot((source[18].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).x;
    // 75: dp4 r10.y, cb0[19].xyzw, r3.xyzw
    r10.y = (dot((source[19].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).y;
    // 76: dp4 r10.z, cb0[20].xyzw, r3.xyzw
    r10.z = (dot((source[20].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).z;
    // 77: mul r11.xyzw, r3.yzzx, r3.xyzz
    r11.xyzw = ((r3.yzzx)*(r3.xyzz)).xyzw;
    // 78: dp4 r12.x, cb0[21].xyzw, r11.xyzw
    r12.x = (dot((source[21].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 79: dp4 r12.y, cb0[22].xyzw, r11.xyzw
    r12.y = (dot((source[22].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 80: dp4 r12.z, cb0[23].xyzw, r11.xyzw
    r12.z = (dot((source[23].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 81: add r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)+(r12.xyzx)).xyz;
    // 82: mul r1.w, r3.y, r3.y
    r1.w = ((r3.yyyy)*(r3.yyyy)).w;
    // 83: mov r8.z, r3.y
    r8.z = (r3.yyyy).z;
    // 84: mad r1.w, r3.x, r3.x, -r1.w
    r1.w = ((r3.xxxx)*(r3.xxxx)+(-(r1.wwww))).w;
    // 85: mad r3.xyz, cb0[24].xyzx, r1.wwww, r10.xyzx
    r3.xyz = ((source[24].xyzx)*(r1.wwww)+(r10.xyzx)).xyz;
    // 86: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 87: mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // 88: mad r3.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r3.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 89: mul r10.xyz, cb0[7].xyzx, cb0[12].xxxx
    r10.xyz = ((source[7].xyzx)*(source[12].xxxx)).xyz;
    // 90: mul r11.xyz, r1.xyzx, r10.xyzx
    r11.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 91: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: mad r1.xyz, -r10.xyzx, r1.xyzx, r1.wwww
    r1.xyz = ((-(r10.xyzx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 93: mad r1.xyz, cb0[12].zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((source[12].zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 94: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r10.xyz, -r5.xyzx, r1.wwww
    r10.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 96: mad r5.xyz, cb0[11].zzzz, r10.xyzx, r5.xyzx
    r5.xyz = ((source[11].zzzz)*(r10.xyzx)+(r5.xyzx)).xyz;
    // 97: mul r10.xyz, cb0[6].xyzx, cb0[11].wwww
    r10.xyz = ((source[6].xyzx)*(source[11].wwww)).xyz;
    // 98: mul r5.xyz, r5.xyzx, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 99: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 100: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 101: mul r10.xyz, r1.wwww, v5.xyzx
    r10.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 102: dp3 r1.w, r2.xyzx, r10.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 103: mul r11.xyz, r1.wwww, r2.xyzx
    r11.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 104: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 105: dp3 r4.x, r4.xyzx, r11.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 106: dp3 r4.y, r7.xyzx, r11.xyzx
    r4.y = (dot((r7.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 107: mul r4.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r4.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 108: mad r4.zw, cb0[10].zzzz, r4.xxxy, r4.zzzw
    r4.zw = ((source[10].zzzz)*(r4.xxxy)+(r4.zzzw)).zw;
    // 109: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r4.zwzz, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 110: mul r7.xyz, r7.xyzx, cb0[5].xyzx
    r7.xyz = ((r7.xyzx)*(source[5].xyzx)).xyz;
    // 111: mad r7.xyz, cb0[10].wwww, r7.xyzx, r7.xyzx
    r7.xyz = ((source[10].wwww)*(r7.xyzx)+(r7.xyzx)).xyz;
    // 112: add r7.xyz, r7.xyzx, -cb0[10].wwww
    r7.xyz = ((r7.xyzx)+(-(source[10].wwww))).xyz;
    // 113: mov_sat r12.xyz, r7.xyzx
    r12.xyz = (saturate(r7.xyzx)).xyz;
    // 114: mov_sat r7.xyz, -r7.xyzx
    r7.xyz = (saturate(-(r7.xyzx))).xyz;
    // 115: mad r7.xyz, -r2.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r2.wwww))*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 116: mad r5.xyz, r2.wwww, r12.xyzx, r5.xyzx
    r5.xyz = ((r2.wwww)*(r12.xyzx)+(r5.xyzx)).xyz;
    // 117: mul r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 118: max r5.xyz, r5.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r5.xyz = (max(r5.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 119: min r5.xyz, r5.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 120: add r1.xyz, r1.xyzx, -r5.xyzx
    r1.xyz = ((r1.xyzx)+(-(r5.xyzx))).xyz;
    // 121: mad r1.xyz, r0.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 122: mul r5.xyz, r1.xyzx, cb0[12].wwww
    r5.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 123: mad r1.xyz, cb0[13].xxxx, r1.xyzx, -r5.xyzx
    r1.xyz = ((source[13].xxxx)*(r1.xyzx)+(-(r5.xyzx))).xyz;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 125: mul r0.w, r7.z, cb0[13].y
    r0.w = ((r7.zzzz)*(source[13].yyyy)).w;
    // 126: mul r4.zw, r7.yyyx, cb0[14].yyyw
    r4.zw = ((r7.yyyx)*(source[14].yyyw)).zw;
    // 127: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 128: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 129: mul r2.w, r2.w, cb0[13].z
    r2.w = ((r2.wwww)*(source[13].zzzz)).w;
    // 130: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 131: movc r0.w, r0.w, l(0), r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 132: min r2.w, r0.w, l(1.000000)
    r2.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 134: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 135: add r5.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 136: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 137: mad_sat r5.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 138: mov_sat r5.w, cb0[13].w
    r5.w = (saturate(source[13].wwww)).w;
    // 139: mad r1.xyz, -r5.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r1.xyz = ((-(r5.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 140: mul r0.w, r5.w, l(0.080000)
    r0.w = ((r5.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 141: mov o3.xyzw, r5.xyzw
    output.targets[3].xyzw = (r5.xyzw).xyzw;
    // 142: mad r1.xyz, r7.wwww, r1.xyzx, r0.wwww
    r1.xyz = ((r7.wwww)*(r1.xyzx)+(r0.wwww)).xyz;
    // 143: deriv_rtx_coarse r7.x, r1.w
    r7.x = (ddx_coarse(r1.wwww)).x;
    // 144: deriv_rty_coarse r7.y, r1.w
    r7.y = (ddy_coarse(r1.wwww)).y;
    // 145: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: dp2 r1.w, r7.xyxx, r7.xyxx
    r1.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 147: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 148: log r7.xy, |r4.zwzz|
    r7.xy = (log2(abs(r4.zwzz))).xy;
    // 149: lt r4.zw, |r4.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r4.zw = (asfloat((uint4)((abs(r4.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 150: mul r2.w, r7.x, cb0[14].z
    r2.w = ((r7.xxxx)*(source[14].zzzz)).w;
    // 151: mul r3.w, r7.y, cb0[15].x
    r3.w = ((r7.yyyy)*(source[15].xxxx)).w;
    // 152: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 153: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 155: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 156: movc r2.w, r4.z, l(0), r2.w
    r2.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 157: max r2.w, r2.w, cb0[2].x
    r2.w = (max(r2.wwww,source[2].xxxx)).w;
    // 158: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 159: mad_sat r7.y, r1.w, l(0.300000), r7.z
    r7.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 160: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 161: add r1.w, -r7.y, l(1.000000)
    r1.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: max r12.xyz, r1.xyzx, r1.wwww
    r12.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 163: add r12.xyz, -r1.xyzx, r12.xyzx
    r12.xyz = ((-(r1.xyzx))+(r12.xyzx)).xyz;
    // 164: mul_sat r1.w, r1.y, l(50.000000)
    r1.w = (saturate((r1.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 165: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 166: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add_sat r7.x, r0.w, -r1.w
    r7.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 169: sample_indexable(texture2d)(float,float,float,float) r4.zw, r7.xyxx, t7.zwxy, s8
    r4.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 170: add r0.w, r3.w, r7.x
    r0.w = ((r3.wwww)+(r7.xxxx)).w;
    // 171: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 172: mul r13.xyz, r1.xyzx, r4.wwww
    r13.xyz = ((r1.xyzx)*(r4.wwww)).xyz;
    // 173: mad r12.xyz, r12.xyzx, r4.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r4.zzzz)+(r13.xyzx)).xyz;
    // 174: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r1.w = ((((r4.wwww) != 0.f ? 1.f : 0.f) / ((r4.wwww) != 0.f ? (r4.wwww) : 1.f))).w;
    // 175: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 176: mad r13.xyz, r1.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r1.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: dp3 r1.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 178: mad r1.xyz, r1.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r1.xyz = ((r1.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 179: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 181: mul r3.xyz, r3.xyzx, r14.xyzx
    r3.xyz = ((r3.xyzx)*(r14.xyzx)).xyz;
    // 182: mad r13.xyz, r5.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r5.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 183: mad r14.xyz, r5.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r5.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 184: mad r15.xyz, r5.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r5.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 185: mad r14.xyz, r3.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 186: mad r13.xyz, r14.xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 187: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 188: max r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = (max(r3.wwww,r13.xyzx)).xyz;
    // 189: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 190: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 191: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 192: dp3 r1.w, r14.xyzx, r2.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 193: dp3 r2.x, r14.xyzx, r11.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 194: dp3 r6.y, r6.xyzx, r11.xyzx
    r6.y = (dot((r6.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 195: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 196: mad r2.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 197: mul r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)).xyzw;
    // 198: mul r11.xyz, r2.wwww, cb0[27].xyzx
    r11.xyz = ((r2.wwww)*(source[27].xyzx)).xyz;
    // 199: mad r11.xyz, r2.zzzz, cb0[26].xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(source[26].xyzx)+(r11.xyzx)).xyz;
    // 200: mul r11.xyz, r11.xyzx, cb0[28].wwww
    r11.xyz = ((r11.xyzx)*(source[28].wwww)).xyz;
    // 201: mul r11.xyz, r5.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r11.xyzx)).xyz;
    // 202: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 203: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 204: mad r3.xyz, -r3.xyzx, r7.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r7.wwww)+(r3.xyzx)).xyz;
    // 205: dp2 r6.x, r4.xyxx, r9.xyxx
    r6.x = (dot((r4.xyxx).xy,(r9.xyxx).xy).xxxx).x;
    // 206: dp2 r6.z, r4.xyxx, cb0[17].xyxx
    r6.z = (dot((r4.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 207: mul r1.w, r7.y, l(5.000000)
    r1.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 208: mul r2.z, r7.y, r7.y
    r2.z = ((r7.yyyy)*(r7.yyyy)).z;
    // 209: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 210: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 211: add r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)+(r0.wwww)).w;
    // 212: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 213: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 214: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r6.xyzx, t8.xyzw, s7, r1.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 215: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 216: mul r4.xyz, r4.xyzx, cb0[16].xyzx
    r4.xyz = ((r4.xyzx)*(source[16].xyzx)).xyz;
    // 217: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 218: mad r1.x, r0.w, r1.x, r1.y
    r1.x = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).x;
    // 219: mad r1.x, r1.x, r0.w, r1.z
    r1.x = ((r1.xxxx)*(r0.wwww)+(r1.zzzz)).x;
    // 220: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 221: max r0.w, r0.w, r1.x
    r0.w = (max(r0.wwww,r1.xxxx)).w;
    // 222: mul r1.xyz, r2.yyyy, cb0[27].xyzx
    r1.xyz = ((r2.yyyy)*(source[27].xyzx)).xyz;
    // 223: mad r1.xyz, cb0[26].xyzx, r2.xxxx, r1.xyzx
    r1.xyz = ((source[26].xyzx)*(r2.xxxx)+(r1.xyzx)).xyz;
    // 224: mul r1.xyz, r1.xyzx, cb0[28].wwww
    r1.xyz = ((r1.xyzx)*(source[28].wwww)).xyz;
    // 225: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 226: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 227: mad r2.xyz, r1.xyzx, r12.xyzx, r3.xyzx
    r2.xyz = ((r1.xyzx)*(r12.xyzx)+(r3.xyzx)).xyz;
    // 228: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 229: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 230: dp3 r0.x, r0.xyzx, r10.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 231: add r0.y, -|r10.z|, l(1.000000)
    r0.y = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 232: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 233: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 234: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 235: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 236: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 237: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 238: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 239: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 240: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 241: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 242: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 243: mad o0.xyz, r5.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 244: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 245: dp3 r0.x, r8.xyzx, r8.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 246: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 247: mul r0.xyz, r0.xxxx, r8.xyzx
    r0.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 248: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 249: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 250: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 251: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 252: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 253: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 254: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 255: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 256: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 257: ftou r0.x, cb0[25].z
    r0.x = (asfloat((uint4)(source[25].zzzz))).x;
    // 258: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 259: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 260: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 261: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 262: ret
    return output;
}

// source.character.static-map-native-217.v1 / source program f4e17d22164c6345b5570333844f4511
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked217(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[28]=1.f; source[29]=1.f; // RNM samples already contain their instance scales.
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[15]=g_SourceCharacterEnvironmentColor;source[16]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0;
    // 1: mul r0.xy, cb0[1].xyxx, cb0[9].zzzz
    r0.xy = ((source[1].xyxx)*(source[9].zzzz)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r0.w, r1.z, cb0[11].x
    r0.w = ((r1.zzzz)*(source[11].xxxx)).w;
    // 8: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 9: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 11: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 12: add r2.z, r1.z, l(0.000010)
    r2.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: mul r3.xyzw, v4.xyxy, cb0[8].yyww
    r3.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 15: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 16: mul r1.zw, r1.zzzw, cb0[8].zzzz
    r1.zw = ((r1.zzzw)*(source[8].zzzz)).zw;
    // 17: mad r1.xy, cb0[8].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[8].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
    // 18: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 19: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 21: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 22: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 26: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v1.xyzx
    r5.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 43: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[9].y, l(0.000000)
    r0.z = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 50: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 53: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 57: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 58: mul r3.xy, r3.xyxx, cb0[9].xxxx
    r3.xy = ((r3.xyxx)*(source[9].xxxx)).xy;
    // 59: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 61: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 62: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 63: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 64: mad r1.xyz, r0.yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 65: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r3.xyz, r0.yyyy, r1.xyzx
    r3.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 68: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 69: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 70: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 71: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 72: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t10.xyzw, s7
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 73: mul r9.xyz, r9.xyzx, cb0[29].xyzx
    r9.xyz = ((r9.xyzx)*(source[29].xyzx)).xyz;
    // 74: dp3 r0.y, r9.xyzx, r8.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 75: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t9.xyzw, s7
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 76: mul r8.xyz, r8.xyzx, cb0[28].xyzx
    r8.xyz = ((r8.xyzx)*(source[28].xyzx)).xyz;
    // 77: mul r10.xyz, r0.yyyy, r8.xyzx
    r10.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 78: mul r11.xyz, cb0[7].xyzx, cb0[11].zzzz
    r11.xyz = ((source[7].xyzx)*(source[11].zzzz)).xyz;
    // 79: mul r12.xyz, r7.xyzx, r11.xyzx
    r12.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 80: dp3 r0.z, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 81: mad r7.xyz, -r11.xyzx, r7.xyzx, r0.zzzz
    r7.xyz = ((-(r11.xyzx))*(r7.xyzx)+(r0.zzzz)).xyz;
    // 82: mad r7.xyz, cb0[12].xxxx, r7.xyzx, r12.xyzx
    r7.xyz = ((source[12].xxxx)*(r7.xyzx)+(r12.xyzx)).xyz;
    // 83: mul r11.xyz, cb0[6].xyzx, cb0[11].yyyy
    r11.xyz = ((source[6].xyzx)*(source[11].yyyy)).xyz;
    // 84: mul r4.xyz, r4.xyzx, r11.xyzx
    r4.xyz = ((r4.xyzx)*(r11.xyzx)).xyz;
    // 85: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 86: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 87: mul r11.xyz, r0.zzzz, v5.xyzx
    r11.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 88: dp3 r0.z, r3.xyzx, r11.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).z;
    // 89: mul r12.xyz, r0.zzzz, r3.xyzx
    r12.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 90: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 91: dp3 r13.x, r2.xyzx, r12.xyzx
    r13.x = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 92: dp3 r2.x, r2.xyzx, r3.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 93: dp3 r13.y, r6.xyzx, r12.xyzx
    r13.y = (dot((r6.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 94: dp3 r2.y, r6.xyzx, r3.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 95: mul r6.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r6.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 96: mad r6.xy, cb0[10].zzzz, r13.xyxx, r6.xyxx
    r6.xy = ((source[10].zzzz)*(r13.xyxx)+(r6.xyxx)).xy;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 98: mul r6.xyz, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r6.xyzx)*(source[5].xyzx)).xyz;
    // 99: mad r6.xyz, cb0[10].wwww, r6.xyzx, r6.xyzx
    r6.xyz = ((source[10].wwww)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 100: add r6.xyz, r6.xyzx, -cb0[10].wwww
    r6.xyz = ((r6.xyzx)+(-(source[10].wwww))).xyz;
    // 101: mov_sat r14.xyz, r6.xyzx
    r14.xyz = (saturate(r6.xyzx)).xyz;
    // 102: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 103: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 104: mad r4.xyz, r0.wwww, r14.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r14.xyzx)+(r4.xyzx)).xyz;
    // 105: mul r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)).xyz;
    // 106: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 107: min r4.xyz, r4.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 108: add r6.xyz, -r4.xyzx, r7.xyzx
    r6.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 109: mad r4.xyz, r0.xxxx, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 110: mul r6.xyz, r4.xyzx, cb0[12].yyyy
    r6.xyz = ((r4.xyzx)*(source[12].yyyy)).xyz;
    // 111: mad r4.xyz, cb0[12].zzzz, r4.xyzx, -r6.xyzx
    r4.xyz = ((source[12].zzzz)*(r4.xyzx)+(-(r6.xyzx))).xyz;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 113: mul r0.x, r7.z, cb0[12].w
    r0.x = ((r7.zzzz)*(source[12].wwww)).x;
    // 114: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 115: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 116: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 117: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 118: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 119: min r0.w, r0.x, l(1.000000)
    r0.w = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: mul_sat r7.w, r0.x, cb2[3].w
    r7.w = (saturate((r0.xxxx)*(passValues[3].wwww))).w;
    // 121: mad r4.xyz, r0.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 122: add r6.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 123: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 124: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 125: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 126: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 127: mul r6.xyz, r0.xxxx, v6.xyzx
    r6.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 128: dp3 r0.x, r6.xyzx, r3.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 129: dp3 r3.y, r5.xyzx, r3.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 130: dp3 r5.y, r5.xyzx, r12.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 131: mad r0.xw, r0.xxxx, l(0.500000, 0.000000, 0.000000, -0.500000), l(0.500000, 0.000000, 0.000000, 0.500000)
    r0.xw = ((r0.xxxx)*(float4(0.500000,0.000000,0.000000,-0.500000))+(float4(0.500000,0.000000,0.000000,0.500000))).xw;
    // 132: mul r0.xw, r0.xxxw, r0.xxxw
    r0.xw = ((r0.xxxw)*(r0.xxxw)).xw;
    // 133: mul r6.xyz, r0.wwww, cb0[26].xyzx
    r6.xyz = ((r0.wwww)*(source[26].xyzx)).xyz;
    // 134: mad r6.xyz, r0.xxxx, cb0[25].xyzx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(source[25].xyzx)+(r6.xyzx)).xyz;
    // 135: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 136: mul r14.xyz, r4.xyzx, r6.xyzx
    r14.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 137: mad r10.xyz, r4.xyzx, r10.xyzx, r14.xyzx
    r10.xyz = ((r4.xyzx)*(r10.xyzx)+(r14.xyzx)).xyz;
    // 138: mad r14.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 139: mad r15.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r15.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 140: mad r16.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r16.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 141: mul r0.x, r7.x, cb0[14].y
    r0.x = ((r7.xxxx)*(source[14].yyyy)).x;
    // 142: mul r0.w, r7.y, cb0[13].w
    r0.w = ((r7.yyyy)*(source[13].wwww)).w;
    // 143: log r1.w, |r0.x|
    r1.w = (log2(abs(r0.xxxx))).w;
    // 144: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 145: mul r1.w, r1.w, cb0[14].z
    r1.w = ((r1.wwww)*(source[14].zzzz)).w;
    // 146: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 147: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: movc r0.x, r0.x, l(0), r1.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 149: mad r15.xyz, r0.xxxx, r15.xyzx, r16.xyzx
    r15.xyz = ((r0.xxxx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 150: mad r14.xyz, r15.xyzx, r0.xxxx, r14.xyzx
    r14.xyz = ((r15.xyzx)*(r0.xxxx)+(r14.xyzx)).xyz;
    // 151: mul r14.xyz, r0.xxxx, r14.xyzx
    r14.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 152: max r14.xyz, r0.xxxx, r14.xyzx
    r14.xyz = (max(r0.xxxx,r14.xyzx)).xyz;
    // 153: mul r10.xyz, r10.xyzx, r14.xyzx
    r10.xyz = ((r10.xyzx)*(r14.xyzx)).xyz;
    // 154: dp2 r3.z, r2.xyxx, cb0[16].xyxx
    r3.z = (dot((r2.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 155: mul r7.xy, cb0[16].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[16].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 156: dp2 r3.x, r2.xyxx, r7.xyxx
    r3.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 157: dp2 r5.x, r13.xyxx, r7.xyxx
    r5.x = (dot((r13.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 158: dp2 r5.z, r13.xyxx, cb0[16].xyxx
    r5.z = (dot((r13.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 159: mov r3.w, l(1.000000)
    r3.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 160: dp4 r13.x, cb0[17].xyzw, r3.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).x;
    // 161: dp4 r13.y, cb0[18].xyzw, r3.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).y;
    // 162: dp4 r13.z, cb0[19].xyzw, r3.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r3.xyzw).xyzw).xxxx).z;
    // 163: mul r14.xyzw, r3.yzzx, r3.xyzz
    r14.xyzw = ((r3.yzzx)*(r3.xyzz)).xyzw;
    // 164: dp4 r15.x, cb0[20].xyzw, r14.xyzw
    r15.x = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 165: dp4 r15.y, cb0[21].xyzw, r14.xyzw
    r15.y = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 166: dp4 r15.z, cb0[22].xyzw, r14.xyzw
    r15.z = (dot((source[22].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 167: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 168: mul r1.w, r3.y, r3.y
    r1.w = ((r3.yyyy)*(r3.yyyy)).w;
    // 169: mov r2.z, r3.y
    r2.z = (r3.yyyy).z;
    // 170: mad r1.w, r3.x, r3.x, -r1.w
    r1.w = ((r3.xxxx)*(r3.xxxx)+(-(r1.wwww))).w;
    // 171: mad r3.xyz, cb0[23].xyzx, r1.wwww, r13.xyzx
    r3.xyz = ((source[23].xyzx)*(r1.wwww)+(r13.xyzx)).xyz;
    // 172: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 173: mul r3.xyz, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((r3.xyzx)*(source[15].xyzx)).xyz;
    // 174: mad r3.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r3.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 175: deriv_rtx_coarse r7.x, r0.z
    r7.x = (ddx_coarse(r0.zzzz)).x;
    // 176: deriv_rty_coarse r7.y, r0.z
    r7.y = (ddy_coarse(r0.zzzz)).y;
    // 177: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 178: dp2 r1.w, r7.xyxx, r7.xyxx
    r1.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 179: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 180: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 181: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 182: mul r2.w, r2.w, cb0[14].x
    r2.w = ((r2.wwww)*(source[14].xxxx)).w;
    // 183: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 184: movc r0.w, r0.w, l(0), r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 185: max r0.w, r0.w, cb0[2].x
    r0.w = (max(r0.wwww,source[2].xxxx)).w;
    // 186: min r7.z, r0.w, l(1.000000)
    r7.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 187: mad_sat r7.y, r1.w, l(0.300000), r7.z
    r7.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 188: add r0.w, r12.z, l(1.000000)
    r0.w = ((r12.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 190: add_sat r7.x, -r0.w, r0.z
    r7.x = (saturate((-(r0.wwww))+(r0.zzzz))).x;
    // 191: sample_indexable(texture2d)(float,float,float,float) r0.zw, r7.xyxx, t7.zwxy, s9
    r0.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 192: add r1.w, r0.x, r7.x
    r1.w = ((r0.xxxx)+(r7.xxxx)).w;
    // 193: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 194: mov_sat r4.w, cb0[13].y
    r4.w = (saturate(source[13].yyyy)).w;
    // 195: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 196: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 197: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 198: mad r13.xyz, r7.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 199: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 200: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 201: max r14.xyz, r13.xyzx, r3.wwww
    r14.xyz = (max(r13.xyzx,r3.wwww)).xyz;
    // 202: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 203: mul r14.xyz, r2.wwww, r14.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 204: mul r15.xyz, r0.wwww, r13.xyzx
    r15.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 205: mad r14.xyz, r14.xyzx, r0.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r0.zzzz)+(r15.xyzx)).xyz;
    // 206: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.z = ((((r0.wwww) != 0.f ? 1.f : 0.f) / ((r0.wwww) != 0.f ? (r0.wwww) : 1.f))).z;
    // 207: add r0.z, r0.z, l(-1.000000)
    r0.z = ((r0.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 208: mad r15.xyz, r13.xyzx, r0.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r13.xyzx)*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 210: mad r13.xyz, r0.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r0.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 211: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 212: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 213: mul r3.xyz, r3.xyzx, r16.xyzx
    r3.xyz = ((r3.xyzx)*(r16.xyzx)).xyz;
    // 214: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 215: mad r3.xyz, -r3.xyzx, r7.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r7.wwww)+(r3.xyzx)).xyz;
    // 216: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 217: dp2_sat r10.x, r12.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r12.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 218: dp3_sat r10.y, r12.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r12.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 219: dp3_sat r10.z, r12.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r12.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 220: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 221: dp3 r0.z, r9.xyzx, r10.xyzx
    r0.z = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 222: add r0.y, -r0.z, r0.y
    r0.y = ((-(r0.zzzz))+(r0.yyyy)).y;
    // 223: mad r0.y, r7.z, r0.y, r0.z
    r0.y = ((r7.zzzz)*(r0.yyyy)+(r0.zzzz)).y;
    // 224: mad r6.xyz, r8.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 225: mul r0.yzw, r0.yyyy, r8.xxyz
    r0.yzw = ((r0.yyyy)*(r8.xxyz)).yzw;
    // 226: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 227: mul r3.w, r7.y, l(5.000000)
    r3.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 228: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r5.xyzx, t8.xyzw, s8, r3.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 229: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 230: mul r5.xyz, r5.xyzx, cb0[15].xyzx
    r5.xyz = ((r5.xyzx)*(source[15].xyzx)).xyz;
    // 231: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 232: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 233: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 234: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 235: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 236: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 237: mad r1.w, r0.x, r13.x, r13.y
    r1.w = ((r0.xxxx)*(r13.xxxx)+(r13.yyyy)).w;
    // 238: mad r1.w, r1.w, r0.x, r13.z
    r1.w = ((r1.wwww)*(r0.xxxx)+(r13.zzzz)).w;
    // 239: mul r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)*(r1.wwww)).w;
    // 240: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 241: mul r7.xyz, r0.xxxx, r6.xyzx
    r7.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 242: add r6.xyz, r6.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 243: div r0.xyz, r0.yzwy, r6.xyzx
    r0.xyz = ((r0.yzwy)/(r6.xyzx)).xyz;
    // 244: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 245: mul r0.yzw, r5.xxyz, r7.xxyz
    r0.yzw = ((r5.xxyz)*(r7.xxyz)).yzw;
    // 246: mad r3.xyz, r0.yzwy, r14.xyzx, r3.xyzx
    r3.xyz = ((r0.yzwy)*(r14.xyzx)+(r3.xyzx)).xyz;
    // 247: mul r0.yzw, r14.xxyz, r0.yyzw
    r0.yzw = ((r14.xxyz)*(r0.yyzw)).yzw;
    // 248: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 249: dp3 r0.y, r1.xyzx, r11.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 250: add r0.z, -|r11.z|, l(1.000000)
    r0.z = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 251: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 252: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 253: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 254: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 255: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 256: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 257: mul r1.xyz, r0.yyyy, cb0[4].xyzx
    r1.xyz = ((r0.yyyy)*(source[4].xyzx)).xyz;
    // 258: movc r0.yzw, r0.zzzz, l(0,0,0,0), r1.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyz)).yzw;
    // 259: add r0.yzw, r0.yyzw, cb0[3].xxyz
    r0.yzw = ((r0.yyzw)+(source[3].xxyz)).yzw;
    // 260: add r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)+(r0.yyzw)).yzw;
    // 261: mad o0.xyz, r4.xyzx, cb0[27].xyzx, r0.yzwy
    output.targets[0].xyz = ((r4.xyzx)*(source[27].xyzx)+(r0.yzwy)).xyz;
    // 262: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 263: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 264: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 265: mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // 266: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 267: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 268: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 269: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 270: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 271: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 272: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 273: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 274: mul o4.z, r0.x, r3.x
    output.targets[4].z = ((r0.xxxx)*(r3.xxxx)).z;
    // 275: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 276: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 277: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
    // 278: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 279: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 280: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 281: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 282: ret
    return output;
}

// source.character.static-map-native-217.v1 / source program 35c0025257042b4eb168844543f915b4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase217(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[9].x=g_SourceCharacterBaseConstants[9].x;
    source[9].y=g_SourceCharacterBaseConstants[9].y;
    source[9].z=g_SourceCharacterBaseConstants[9].z;
    source[9].w=g_SourceCharacterBaseConstants[9].w;
    source[10].x=g_SourceCharacterBaseConstants[10].x;
    source[10].y=g_SourceCharacterBaseConstants[10].y;
    source[10].z=g_SourceCharacterBaseConstants[10].z;
    source[10].w=g_SourceCharacterBaseConstants[10].w;
    source[11].x=g_SourceCharacterBaseConstants[11].x;
    source[11].y=g_SourceCharacterBaseConstants[11].y;
    source[11].z=g_SourceCharacterBaseConstants[11].z;
    source[11].w=g_SourceCharacterBaseConstants[11].w;
    source[12].x=g_SourceCharacterBaseConstants[12].x;
    source[12].y=g_SourceCharacterBaseConstants[12].y;
    source[12].z=g_SourceCharacterBaseConstants[12].z;
    source[12].w=g_SourceCharacterBaseConstants[12].w;
    source[13].x=g_SourceCharacterBaseConstants[13].x;
    source[13].y=g_SourceCharacterBaseConstants[13].y;
    source[13].z=g_SourceCharacterBaseConstants[13].z;
    source[13].w=g_SourceCharacterBaseConstants[13].w;
    source[14].x=g_SourceCharacterBaseConstants[14].x;
    source[14].y=g_SourceCharacterBaseConstants[14].y;
    source[14].z=g_SourceCharacterBaseConstants[14].z;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[15]=g_SourceCharacterEnvironmentColor;source[16]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: mul r0.xy, cb0[1].xyxx, cb0[9].zzzz
    r0.xy = ((source[1].xyxx)*(source[9].zzzz)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r0.w, r1.z, cb0[11].x
    r0.w = ((r1.zzzz)*(source[11].xxxx)).w;
    // 8: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 9: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 11: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 12: add r2.z, r1.z, l(0.000010)
    r2.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: mul r3.xyzw, v4.xyxy, cb0[8].yyww
    r3.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 15: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 16: mul r1.zw, r1.zzzw, cb0[8].zzzz
    r1.zw = ((r1.zzzw)*(source[8].zzzz)).zw;
    // 17: mad r1.xy, cb0[8].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[8].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
    // 18: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 19: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 21: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 22: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 25: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 26: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v1.xyzx
    r5.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 30: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 31: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 32: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 33: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 43: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 44: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 45: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 46: max r0.z, cb0[9].y, l(0.000000)
    r0.z = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 47: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 48: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 49: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 50: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 51: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 52: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 53: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 54: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 55: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 56: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 57: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 58: mul r3.xy, r3.xyxx, cb0[9].xxxx
    r3.xy = ((r3.xyxx)*(source[9].xxxx)).xy;
    // 59: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 61: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 62: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 63: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 64: mad r1.xyz, r0.yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 65: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r3.xyz, r0.yyyy, r1.xyzx
    r3.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 68: dp3 r8.y, r5.xyzx, r3.xyzx
    r8.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 69: dp3 r9.x, r2.xyzx, r3.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 70: dp3 r9.y, r6.xyzx, r3.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 71: dp2 r8.z, r9.xyxx, cb0[16].xyxx
    r8.z = (dot((r9.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 72: mul r0.yz, cb0[16].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r0.yz = ((source[16].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 73: dp2 r8.x, r9.xyxx, r0.yzyy
    r8.x = (dot((r9.xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 74: mov r8.w, l(1.000000)
    r8.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 75: dp4 r10.x, cb0[17].xyzw, r8.xyzw
    r10.x = (dot((source[17].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).x;
    // 76: dp4 r10.y, cb0[18].xyzw, r8.xyzw
    r10.y = (dot((source[18].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).y;
    // 77: dp4 r10.z, cb0[19].xyzw, r8.xyzw
    r10.z = (dot((source[19].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).z;
    // 78: mul r11.xyzw, r8.yzzx, r8.xyzz
    r11.xyzw = ((r8.yzzx)*(r8.xyzz)).xyzw;
    // 79: dp4 r12.x, cb0[20].xyzw, r11.xyzw
    r12.x = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 80: dp4 r12.y, cb0[21].xyzw, r11.xyzw
    r12.y = (dot((source[21].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 81: dp4 r12.z, cb0[22].xyzw, r11.xyzw
    r12.z = (dot((source[22].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 82: add r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)+(r12.xyzx)).xyz;
    // 83: mul r1.w, r8.y, r8.y
    r1.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 84: mov r9.z, r8.y
    r9.z = (r8.yyyy).z;
    // 85: mad r1.w, r8.x, r8.x, -r1.w
    r1.w = ((r8.xxxx)*(r8.xxxx)+(-(r1.wwww))).w;
    // 86: mad r8.xyz, cb0[23].xyzx, r1.wwww, r10.xyzx
    r8.xyz = ((source[23].xyzx)*(r1.wwww)+(r10.xyzx)).xyz;
    // 87: max r8.xyz, r8.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 88: mul r8.xyz, r8.xyzx, cb0[15].xyzx
    r8.xyz = ((r8.xyzx)*(source[15].xyzx)).xyz;
    // 89: mad r8.xyz, r8.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r8.xyz = ((r8.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 90: mul r10.xyz, cb0[7].xyzx, cb0[11].zzzz
    r10.xyz = ((source[7].xyzx)*(source[11].zzzz)).xyz;
    // 91: mul r11.xyz, r7.xyzx, r10.xyzx
    r11.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 92: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: mad r7.xyz, -r10.xyzx, r7.xyzx, r1.wwww
    r7.xyz = ((-(r10.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 94: mad r7.xyz, cb0[12].xxxx, r7.xyzx, r11.xyzx
    r7.xyz = ((source[12].xxxx)*(r7.xyzx)+(r11.xyzx)).xyz;
    // 95: mul r10.xyz, cb0[6].xyzx, cb0[11].yyyy
    r10.xyz = ((source[6].xyzx)*(source[11].yyyy)).xyz;
    // 96: mul r4.xyz, r4.xyzx, r10.xyzx
    r4.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 97: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 98: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 99: mul r10.xyz, r1.wwww, v5.xyzx
    r10.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 100: dp3 r1.w, r3.xyzx, r10.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 101: mul r11.xyz, r1.wwww, r3.xyzx
    r11.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 102: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 103: dp3 r2.x, r2.xyzx, r11.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 104: dp3 r2.y, r6.xyzx, r11.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 105: mul r2.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r2.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 106: mad r2.zw, cb0[10].zzzz, r2.xxxy, r2.zzzw
    r2.zw = ((source[10].zzzz)*(r2.xxxy)+(r2.zzzw)).zw;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r2.zwzz, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 108: mul r6.xyz, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r6.xyzx)*(source[5].xyzx)).xyz;
    // 109: mad r6.xyz, cb0[10].wwww, r6.xyzx, r6.xyzx
    r6.xyz = ((source[10].wwww)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 110: add r6.xyz, r6.xyzx, -cb0[10].wwww
    r6.xyz = ((r6.xyzx)+(-(source[10].wwww))).xyz;
    // 111: mov_sat r12.xyz, r6.xyzx
    r12.xyz = (saturate(r6.xyzx)).xyz;
    // 112: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 113: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 114: mad r4.xyz, r0.wwww, r12.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r12.xyzx)+(r4.xyzx)).xyz;
    // 115: mul r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)).xyz;
    // 116: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 117: min r4.xyz, r4.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 118: add r6.xyz, -r4.xyzx, r7.xyzx
    r6.xyz = ((-(r4.xyzx))+(r7.xyzx)).xyz;
    // 119: mad r4.xyz, r0.xxxx, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 120: mul r6.xyz, r4.xyzx, cb0[12].yyyy
    r6.xyz = ((r4.xyzx)*(source[12].yyyy)).xyz;
    // 121: mad r4.xyz, cb0[12].zzzz, r4.xyzx, -r6.xyzx
    r4.xyz = ((source[12].zzzz)*(r4.xyzx)+(-(r6.xyzx))).xyz;
    // 122: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 123: mul r0.x, r7.z, cb0[12].w
    r0.x = ((r7.zzzz)*(source[12].wwww)).x;
    // 124: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 125: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 126: mul r0.w, r0.w, cb0[13].x
    r0.w = ((r0.wwww)*(source[13].xxxx)).w;
    // 127: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 128: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 129: min r0.w, r0.x, l(1.000000)
    r0.w = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul_sat r2.w, r0.x, cb2[3].w
    r2.w = (saturate((r0.xxxx)*(passValues[3].wwww))).w;
    // 131: mad r4.xyz, r0.wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 132: add r6.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 133: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 134: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 135: mov_sat r4.w, cb0[13].y
    r4.w = (saturate(source[13].yyyy)).w;
    // 136: mad r6.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r6.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 137: mul r0.x, r4.w, l(0.080000)
    r0.x = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 138: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 139: mad r6.xyz, r2.wwww, r6.xyzx, r0.xxxx
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(r0.xxxx)).xyz;
    // 140: deriv_rtx_coarse r12.x, r1.w
    r12.x = (ddx_coarse(r1.wwww)).x;
    // 141: deriv_rty_coarse r12.y, r1.w
    r12.y = (ddy_coarse(r1.wwww)).y;
    // 142: add r0.x, r1.w, l(1.000000)
    r0.x = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: dp2 r0.w, r12.xyxx, r12.xyxx
    r0.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 144: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 145: mul r1.w, r7.y, cb0[13].w
    r1.w = ((r7.yyyy)*(source[13].wwww)).w;
    // 146: mul r3.w, r7.x, cb0[14].y
    r3.w = ((r7.xxxx)*(source[14].yyyy)).w;
    // 147: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 148: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 149: mul r4.w, r4.w, cb0[14].x
    r4.w = ((r4.wwww)*(source[14].xxxx)).w;
    // 150: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 151: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 152: max r1.w, r1.w, cb0[2].x
    r1.w = (max(r1.wwww,source[2].xxxx)).w;
    // 153: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 154: mad_sat r7.y, r0.w, l(0.300000), r2.z
    r7.y = (saturate((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 155: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 156: add r0.w, -r7.y, l(1.000000)
    r0.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r12.xyz, r6.xyzx, r0.wwww
    r12.xyz = (max(r6.xyzx,r0.wwww)).xyz;
    // 158: add r12.xyz, -r6.xyzx, r12.xyzx
    r12.xyz = ((-(r6.xyzx))+(r12.xyzx)).xyz;
    // 159: mul_sat r0.w, r6.y, l(50.000000)
    r0.w = (saturate((r6.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 160: mul r12.xyz, r0.wwww, r12.xyzx
    r12.xyz = ((r0.wwww)*(r12.xyzx)).xyz;
    // 161: add r0.w, r11.z, l(1.000000)
    r0.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: add_sat r7.x, -r0.w, r0.x
    r7.x = (saturate((-(r0.wwww))+(r0.xxxx))).x;
    // 164: sample_indexable(texture2d)(float,float,float,float) r0.xw, r7.xyxx, t7.xzwy, s8
    r0.xw = ((float4(0.0,0.0,0.0,0.0)).xzwy).xw;
    // 165: mul r13.xyz, r0.wwww, r6.xyzx
    r13.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 166: mad r12.xyz, r12.xyzx, r0.xxxx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r0.xxxx)+(r13.xyzx)).xyz;
    // 167: div r0.x, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.x = ((((r0.wwww) != 0.f ? 1.f : 0.f) / ((r0.wwww) != 0.f ? (r0.wwww) : 1.f))).x;
    // 168: add r0.x, r0.x, l(-1.000000)
    r0.x = ((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 169: mad r13.xyz, r6.xyzx, r0.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r6.xyzx)*(r0.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 171: mad r6.xyz, r0.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r6.xyz = ((r0.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 172: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 173: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 174: mul r8.xyz, r8.xyzx, r14.xyzx
    r8.xyz = ((r8.xyzx)*(r14.xyzx)).xyz;
    // 175: mad r13.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 176: mad r14.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 177: mad r15.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 178: log r0.x, |r3.w|
    r0.x = (log2(abs(r3.wwww))).x;
    // 179: lt r0.w, |r3.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 180: mul r0.x, r0.x, cb0[14].z
    r0.x = ((r0.xxxx)*(source[14].zzzz)).x;
    // 181: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 182: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 183: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 184: mad r14.xyz, r0.xxxx, r14.xyzx, r15.xyzx
    r14.xyz = ((r0.xxxx)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 185: mad r13.xyz, r14.xyzx, r0.xxxx, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r0.xxxx)+(r13.xyzx)).xyz;
    // 186: mul r13.xyz, r0.xxxx, r13.xyzx
    r13.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 187: max r13.xyz, r0.xxxx, r13.xyzx
    r13.xyz = (max(r0.xxxx,r13.xyzx)).xyz;
    // 188: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 189: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 190: mul r14.xyz, r0.wwww, v6.xyzx
    r14.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 191: dp3 r0.w, r14.xyzx, r3.xyzx
    r0.w = (dot((r14.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 192: dp3 r1.w, r14.xyzx, r11.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 193: dp3 r3.y, r5.xyzx, r11.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 194: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: mad r5.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r5.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 196: mul r5.xyzw, r5.xyzw, r5.xyzw
    r5.xyzw = ((r5.xyzw)*(r5.xyzw)).xyzw;
    // 197: mul r11.xyz, r5.wwww, cb0[26].xyzx
    r11.xyz = ((r5.wwww)*(source[26].xyzx)).xyz;
    // 198: mad r11.xyz, r5.zzzz, cb0[25].xyzx, r11.xyzx
    r11.xyz = ((r5.zzzz)*(source[25].xyzx)+(r11.xyzx)).xyz;
    // 199: mul r11.xyz, r11.xyzx, cb0[27].wwww
    r11.xyz = ((r11.xyzx)*(source[27].wwww)).xyz;
    // 200: mul r11.xyz, r4.xyzx, r11.xyzx
    r11.xyz = ((r4.xyzx)*(r11.xyzx)).xyz;
    // 201: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 202: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 203: mad r8.xyz, -r8.xyzx, r2.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r2.wwww)+(r8.xyzx)).xyz;
    // 204: dp2 r3.x, r2.xyxx, r0.yzyy
    r3.x = (dot((r2.xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 205: dp2 r3.z, r2.xyxx, cb0[16].xyxx
    r3.z = (dot((r2.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 206: mul r0.y, r7.y, l(5.000000)
    r0.y = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 207: mul r0.z, r7.y, r7.y
    r0.z = ((r7.yyyy)*(r7.yyyy)).z;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r3.xyzx, t8.xyzw, s7, r0.y
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r0.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[15].xyzx
    r2.xyz = ((r2.xyzx)*(source[15].xyzx)).xyz;
    // 211: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 212: add r0.y, r0.x, r7.x
    r0.y = ((r0.xxxx)+(r7.xxxx)).y;
    // 213: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 214: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 215: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 216: add r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)+(r0.yyyy)).y;
    // 217: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 218: add_sat r0.x, r0.y, l(-1.000000)
    r0.x = (saturate((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 219: mad r0.y, r0.x, r6.x, r6.y
    r0.y = ((r0.xxxx)*(r6.xxxx)+(r6.yyyy)).y;
    // 220: mad r0.y, r0.y, r0.x, r6.z
    r0.y = ((r0.yyyy)*(r0.xxxx)+(r6.zzzz)).y;
    // 221: mul r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)*(r0.yyyy)).y;
    // 222: max r0.x, r0.y, r0.x
    r0.x = (max(r0.yyyy,r0.xxxx)).x;
    // 223: mul r0.yzw, r5.yyyy, cb0[26].xxyz
    r0.yzw = ((r5.yyyy)*(source[26].xxyz)).yzw;
    // 224: mad r0.yzw, cb0[25].xxyz, r5.xxxx, r0.yyzw
    r0.yzw = ((source[25].xxyz)*(r5.xxxx)+(r0.yyzw)).yzw;
    // 225: mul r0.yzw, r0.yyzw, cb0[27].wwww
    r0.yzw = ((r0.yyzw)*(source[27].wwww)).yzw;
    // 226: mul r0.xyz, r0.xxxx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 227: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 228: mad r2.xyz, r0.xyzx, r12.xyzx, r8.xyzx
    r2.xyz = ((r0.xyzx)*(r12.xyzx)+(r8.xyzx)).xyz;
    // 229: mul r0.xyz, r12.xyzx, r0.xyzx
    r0.xyz = ((r12.xyzx)*(r0.xyzx)).xyz;
    // 230: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 231: dp3 r0.x, r1.xyzx, r10.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 232: add r0.y, -|r10.z|, l(1.000000)
    r0.y = ((-(abs(r10.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 233: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 234: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 235: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 236: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 237: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 238: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 239: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 240: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 241: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 242: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 243: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 244: mad o0.xyz, r4.xyzx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 245: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 246: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 247: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 248: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 249: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 250: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 251: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 252: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 253: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 254: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 255: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 256: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 257: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 258: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
    // 259: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 260: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 261: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 262: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 263: ret
    return output;
}

// source.character.static-map-native-218.v1 / source program 1feb3e8eb3d0f143b778429770b5556b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked218(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=1.f; source[15]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 3: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 4: mad r0.xyz, cb0[8].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[8].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r1.xyz, cb0[3].xyzx, cb0[8].wwww
    r1.xyz = ((source[3].xyzx)*(source[8].wwww)).xyz;
    // 6: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r1.z, r1.z, cb0[8].x
    r1.z = ((r1.zzzz)*(source[8].xxxx)).z;
    // 10: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 11: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 14: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: mul r3.xy, r3.xyxx, cb0[6].zzzz
    r3.xy = ((r3.xyxx)*(source[6].zzzz)).xy;
    // 19: mad r1.xy, cb0[6].xxxx, r1.xyxx, r3.xyxx
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r3.xyxx)).xy;
    // 20: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 21: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 22: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 23: div r1.xyw, r2.xyxz, r1.xxxx
    r1.xyw = ((r2.xyxz)/(r1.xxxx)).xyw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r3.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 28: mul r4.xy, r2.xyxx, cb0[7].xxxx
    r4.xy = ((r2.xyxx)*(source[7].xxxx)).xy;
    // 29: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 31: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 32: add r4.z, r2.x, l(0.000010)
    r4.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: add r2.xyz, -r1.xywx, r4.xyzx
    r2.xyz = ((-(r1.xywx))+(r4.xyzx)).xyz;
    // 34: mul r2.w, r1.w, r1.w
    r2.w = ((r1.wwww)*(r1.wwww)).w;
    // 35: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 36: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r2.w, r3.w, r3.w
    r2.w = ((r3.wwww)*(r3.wwww)).w;
    // 38: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 39: max r2.w, cb0[7].y, l(0.000000)
    r2.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 41: mul r3.w, r0.w, r2.w
    r3.w = ((r0.wwww)*(r2.wwww)).w;
    // 42: add r4.x, -v2.x, l(1.000000)
    r4.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 43: mad r3.w, r4.x, r3.w, r4.x
    r3.w = ((r4.xxxx)*(r3.wwww)+(r4.xxxx)).w;
    // 44: add r4.x, -r2.w, r3.w
    r4.x = ((-(r2.wwww))+(r3.wwww)).x;
    // 45: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((((r2.wwww) != 0.f ? 1.f : 0.f) / ((r2.wwww) != 0.f ? (r2.wwww) : 1.f))).w;
    // 47: mad r3.w, -r2.w, r4.x, r3.w
    r3.w = ((-(r2.wwww))*(r4.xxxx)+(r3.wwww)).w;
    // 48: mul r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)*(r2.wwww)).w;
    // 49: mad_sat r0.w, r0.w, r3.w, r2.w
    r0.w = (saturate((r0.wwww)*(r3.wwww)+(r2.wwww))).w;
    // 50: mul r2.w, r0.w, l(0.650000)
    r2.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 51: mad r1.xyw, r2.wwww, r2.xyxz, r1.xyxw
    r1.xyw = ((r2.wwww)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 52: dp3 r2.x, r1.xywx, r1.xywx
    r2.x = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).x;
    // 53: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 54: mul r1.xyw, r1.xyxw, r2.xxxx
    r1.xyw = ((r1.xyxw)*(r2.xxxx)).xyw;
    // 55: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 56: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 57: mul r2.xyz, r2.xxxx, v5.xyzx
    r2.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 58: dp3 r2.w, r1.xywx, r2.xyzx
    r2.w = (dot((r1.xywx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 59: mul r4.xyz, r1.xywx, r2.wwww
    r4.xyz = ((r1.xywx)*(r2.wwww)).xyz;
    // 60: mad r2.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 61: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 64: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 65: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 66: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 67: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 68: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 69: dp3 r4.z, r4.xyzx, r1.xywx
    r4.z = (dot((r4.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // 70: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 71: dp3 r7.y, r6.xyzx, r2.xyzx
    r7.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 72: dp3 r4.y, r6.xyzx, r1.xywx
    r4.y = (dot((r6.xyzx).xyz,(r1.xywx).xyz).xxxx).y;
    // 73: dp3 r7.x, r5.xyzx, r2.xyzx
    r7.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 74: dp3 r4.x, r5.xyzx, r1.xywx
    r4.x = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // 75: mul r5.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r5.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 76: mad r5.xy, cb0[7].zzzz, r7.xyxx, r5.xyxx
    r5.xy = ((source[7].zzzz)*(r7.xyxx)+(r5.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 78: mul r5.xyz, r5.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)*(source[2].xyzx)).xyz;
    // 79: mad r5.xyz, cb0[7].wwww, r5.xyzx, r5.xyzx
    r5.xyz = ((source[7].wwww)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 80: add r5.xyz, r5.xyzx, -cb0[7].wwww
    r5.xyz = ((r5.xyzx)+(-(source[7].wwww))).xyz;
    // 81: mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // 82: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 83: mad r5.xyz, -r1.zzzz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.zzzz))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r0.xyz, r1.zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 85: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 86: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 87: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 88: mul r5.xyz, cb0[4].xyzx, cb0[9].xxxx
    r5.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 89: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 90: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 91: mad r5.xyz, -r5.xyzx, r3.xyzx, r1.zzzz
    r5.xyz = ((-(r5.xyzx))*(r3.xyzx)+(r1.zzzz)).xyz;
    // 92: mad r5.xyz, cb0[9].zzzz, r5.xyzx, r6.xyzx
    r5.xyz = ((source[9].zzzz)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 93: add r5.xyz, -r0.xyzx, r5.xyzx
    r5.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 94: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 95: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 97: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 98: mul r5.xyz, r1.zzzz, v6.xyzx
    r5.xyz = ((r1.zzzz)*(v6.xyzx)).xyz;
    // 99: dp3 r1.z, r5.xyzx, r1.xywx
    r1.z = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // 100: mad r5.xy, r1.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 101: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 102: mul r5.yzw, r5.yyyy, cb0[12].xxyz
    r5.yzw = ((r5.yyyy)*(source[12].xxyz)).yzw;
    // 103: mad r5.xyz, r5.xxxx, cb0[11].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[11].xyzx)+(r5.yzwy)).xyz;
    // 104: mul r5.xyz, r5.xyzx, cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(source[13].wwww)).xyz;
    // 105: mul r6.xyz, r0.xyzx, r5.xyzx
    r6.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 106: dp2_sat r7.x, r1.ywyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r1.ywyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 107: dp3_sat r7.y, r1.xywx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r1.xywx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 108: dp3_sat r7.z, r1.xywx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r1.xywx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 109: mul r1.xyz, r7.xyzx, r7.xyzx
    r1.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 110: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s7
    r7.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 111: mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // 112: dp3 r1.x, r7.xyzx, r1.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 113: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t7.wxyz, s7
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 114: mul r1.yzw, r1.yyzw, cb0[14].xxyz
    r1.yzw = ((r1.yyzw)*(source[14].xxyz)).yzw;
    // 115: mul r8.xyz, r1.xxxx, r1.yzwy
    r8.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 116: mad r5.xyz, r1.yzwy, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.yzwy)*(r1.xxxx)+(r5.xyzx)).xyz;
    // 117: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 118: div r5.xyz, r8.xyzx, r5.xyzx
    r5.xyz = ((r8.xyzx)/(r5.xyzx)).xyz;
    // 119: mad r6.xyz, r0.xyzx, r8.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 120: dp3 r1.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 121: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r5.xyz, -r3.xyzx, r2.wwww
    r5.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 123: mad r3.xyz, cb0[9].zzzz, r5.xyzx, r3.xyzx
    r3.xyz = ((source[9].zzzz)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 124: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 125: mul r8.xyz, cb0[5].xyzx, cb0[9].wwww
    r8.xyz = ((source[5].xyzx)*(source[9].wwww)).xyz;
    // 126: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 127: mad r3.xyz, cb0[10].xxxx, r3.xyzx, -r5.xyzx
    r3.xyz = ((source[10].xxxx)*(r3.xyzx)+(-(r5.xyzx))).xyz;
    // 128: mad r3.xyz, r0.wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 129: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 130: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 131: dp2_sat r3.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 132: dp3_sat r3.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 133: dp3_sat r3.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 134: log r2.xyz, r3.xyzx
    r2.xyz = (log2(r3.xyzx)).xyz;
    // 135: add r0.w, cb0[10].y, l(1.000000)
    r0.w = ((source[10].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: mul r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)*(r0.wwww)).xyz;
    // 137: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 138: dp3 r0.w, r7.xyzx, r2.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 139: mad r2.xyz, r1.yzwy, r0.wwww, r6.xyzx
    r2.xyz = ((r1.yzwy)*(r0.wwww)+(r6.xyzx)).xyz;
    // 140: mul r1.yzw, r0.wwww, r1.yyzw
    r1.yzw = ((r0.wwww)*(r1.yyzw)).yzw;
    // 141: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 142: add r1.yzw, r2.xxyz, cb0[1].xxyz
    r1.yzw = ((r2.xxyz)+(source[1].xxyz)).yzw;
    // 143: mad o0.xyz, r0.xyzx, cb0[13].xyzx, r1.yzwy
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)+(r1.yzwy)).xyz;
    // 144: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 145: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 146: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 147: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 148: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 149: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 150: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 151: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 152: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 153: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 154: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 155: movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 156: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 157: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 158: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 159: mul o4.z, r1.x, r2.x
    output.targets[4].z = ((r1.xxxx)*(r2.xxxx)).z;
    // 160: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 161: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 162: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 163: ret
    return output;
}

// source.character.static-map-native-218.v1 / source program 6477c88c71a565499d0dcd66cf86b727
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase218(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5].x=g_SourceCharacterBaseConstants[6].x;
    source[5].y=g_SourceCharacterBaseConstants[6].y;
    source[5].z=g_SourceCharacterBaseConstants[6].z;
    source[5].w=g_SourceCharacterBaseConstants[6].w;
    source[6].x=g_SourceCharacterBaseConstants[7].x;
    source[6].y=g_SourceCharacterBaseConstants[7].y;
    source[6].z=g_SourceCharacterBaseConstants[7].z;
    source[6].w=g_SourceCharacterBaseConstants[7].w;
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
    source[8].x=g_SourceCharacterBaseConstants[9].x;
    source[8].y=g_SourceCharacterBaseConstants[9].y;
    source[8].z=g_SourceCharacterBaseConstants[9].z;
    source[8].w=g_SourceCharacterBaseConstants[9].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: mul r0.z, r0.z, cb0[7].x
    r0.z = ((r0.zzzz)*(source[7].xxxx)).z;
    // 4: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 5: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 6: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 9: mul r2.xyzw, v4.xyxy, cb0[5].yyww
    r2.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 11: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: mul r2.xy, r2.xyxx, cb0[5].zzzz
    r2.xy = ((r2.xyxx)*(source[5].zzzz)).xy;
    // 13: mad r0.xy, cb0[5].xxxx, r0.xyxx, r2.xyxx
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r2.xyxx)).xy;
    // 14: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 15: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 16: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 17: div r0.xyw, r1.xyxz, r0.xxxx
    r0.xyw = ((r1.xyxz)/(r0.xxxx)).xyw;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 22: mul r3.xy, r1.xyxx, cb0[6].xxxx
    r3.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 23: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 24: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 25: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 26: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: add r1.xyz, -r0.xywx, r3.xyzx
    r1.xyz = ((-(r0.xywx))+(r3.xyzx)).xyz;
    // 28: mul r1.w, r0.w, r0.w
    r1.w = ((r0.wwww)*(r0.wwww)).w;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 30: mul_sat r1.w, r1.w, r3.w
    r1.w = (saturate((r1.wwww)*(r3.wwww))).w;
    // 31: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 32: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 33: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 34: max r2.w, cb0[6].y, l(0.000000)
    r2.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 36: mul r3.w, r1.w, r2.w
    r3.w = ((r1.wwww)*(r2.wwww)).w;
    // 37: add r4.x, -v2.x, l(1.000000)
    r4.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: mad r3.w, r4.x, r3.w, r4.x
    r3.w = ((r4.xxxx)*(r3.wwww)+(r4.xxxx)).w;
    // 39: add r4.x, -r2.w, r3.w
    r4.x = ((-(r2.wwww))+(r3.wwww)).x;
    // 40: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((((r2.wwww) != 0.f ? 1.f : 0.f) / ((r2.wwww) != 0.f ? (r2.wwww) : 1.f))).w;
    // 42: mad r3.w, -r2.w, r4.x, r3.w
    r3.w = ((-(r2.wwww))*(r4.xxxx)+(r3.wwww)).w;
    // 43: mul r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)*(r2.wwww)).w;
    // 44: mad_sat r1.w, r1.w, r3.w, r2.w
    r1.w = (saturate((r1.wwww)*(r3.wwww)+(r2.wwww))).w;
    // 45: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 46: mad r0.xyw, r2.wwww, r1.xyxz, r0.xyxw
    r0.xyw = ((r2.wwww)*(r1.xyxz)+(r0.xyxw)).xyw;
    // 47: dp3 r1.x, r0.xywx, r0.xywx
    r1.x = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).x;
    // 48: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 49: mul r0.xyw, r0.xyxw, r1.xxxx
    r0.xyw = ((r0.xyxw)*(r1.xxxx)).xyw;
    // 50: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 51: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 52: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 53: dp3 r2.w, r0.xywx, r1.xyzx
    r2.w = (dot((r0.xywx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 54: mul r4.xyz, r0.xywx, r2.wwww
    r4.xyz = ((r0.xywx)*(r2.wwww)).xyz;
    // 55: mad r1.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 56: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 57: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 58: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 59: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 60: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 61: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 62: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 63: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 64: dp3 r4.z, r4.xyzx, r0.xywx
    r4.z = (dot((r4.xyzx).xyz,(r0.xywx).xyz).xxxx).z;
    // 65: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r7.y, r6.xyzx, r1.xyzx
    r7.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 67: dp3 r7.x, r5.xyzx, r1.xyzx
    r7.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 68: dp3 r4.x, r5.xyzx, r0.xywx
    r4.x = (dot((r5.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 69: dp3 r4.y, r6.xyzx, r0.xywx
    r4.y = (dot((r6.xyzx).xyz,(r0.xywx).xyz).xxxx).y;
    // 70: mul r1.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r1.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 71: mad r1.xy, cb0[6].zzzz, r7.xyxx, r1.xyxx
    r1.xy = ((source[6].zzzz)*(r7.xyxx)+(r1.xyxx)).xy;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 74: mad r1.xyz, cb0[6].wwww, r1.xyzx, r1.xyzx
    r1.xyz = ((source[6].wwww)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 75: add r1.xyz, r1.xyzx, -cb0[6].wwww
    r1.xyz = ((r1.xyzx)+(-(source[6].wwww))).xyz;
    // 76: mov_sat r5.xyz, r1.xyzx
    r5.xyz = (saturate(r1.xyzx)).xyz;
    // 77: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 78: mad r1.xyz, -r0.zzzz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.zzzz))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: add r6.xyz, -r3.xyzx, r2.wwww
    r6.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 81: mad r3.xyz, cb0[7].zzzz, r6.xyzx, r3.xyzx
    r3.xyz = ((source[7].zzzz)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r6.xyz, cb0[3].xyzx, cb0[7].wwww
    r6.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 83: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 84: mad r3.xyz, r0.zzzz, r5.xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 85: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 86: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 87: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 88: mul r3.xyz, cb0[4].xyzx, cb0[8].xxxx
    r3.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 89: mul r5.xyz, r2.xyzx, r3.xyzx
    r5.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 90: dp3 r0.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 91: mad r2.xyz, -r3.xyzx, r2.xyzx, r0.zzzz
    r2.xyz = ((-(r3.xyzx))*(r2.xyzx)+(r0.zzzz)).xyz;
    // 92: mad r2.xyz, cb0[8].zzzz, r2.xyzx, r5.xyzx
    r2.xyz = ((source[8].zzzz)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 93: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 94: mad r1.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 95: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 97: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 98: mul r2.xyz, r0.zzzz, v6.xyzx
    r2.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 99: dp3 r0.x, r2.xyzx, r0.xywx
    r0.x = (dot((r2.xyzx).xyz,(r0.xywx).xyz).xxxx).x;
    // 100: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 101: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 102: mul r0.yzw, r0.yyyy, cb0[10].xxyz
    r0.yzw = ((r0.yyyy)*(source[10].xxyz)).yzw;
    // 103: mad r0.xyz, r0.xxxx, cb0[9].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[9].xyzx)+(r0.yzwy)).xyz;
    // 104: mul r0.xyz, r0.xyzx, cb0[11].wwww
    r0.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 105: mad r2.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 106: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 107: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 108: mad o0.xyz, r1.xyzx, cb0[11].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[11].xyzx)+(r2.xyzx)).xyz;
    // 109: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 110: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 111: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 112: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 113: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 114: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 115: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 116: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 117: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 118: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 119: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 120: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 121: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 122: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 123: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 124: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 125: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 126: ret
    return output;
}

// source.character.static-map-native-219.v1 / source program 7884fe3989fc5e42925ef98ce171b181
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked219(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=1.f; source[19]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: mul r0.zw, r0.xxxy, cb0[10].xxxx
    r0.zw = ((r0.xxxy)*(source[10].xxxx)).zw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 4: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 5: mul r0.zw, r0.zzzw, cb0[10].yyyy
    r0.zw = ((r0.zzzw)*(source[10].yyyy)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 9: mad r0.zw, cb0[9].wwww, r1.xxxy, r0.zzzw
    r0.zw = ((source[9].wwww)*(r1.xxxy)+(r0.zzzw)).zw;
    // 10: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 11: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 14: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.xy, r0.zwzz, v2.wwww
    r2.xy = ((r0.zwzz)*(v2.wwww)).xy;
    // 16: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 17: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 18: div r1.xyw, r2.xyxz, r0.zzzz
    r1.xyw = ((r2.xyxz)/(r0.zzzz)).xyw;
    // 19: add r2.xyz, -r1.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r1.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 20: mul r0.z, r1.w, r1.w
    r0.z = ((r1.wwww)*(r1.wwww)).z;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.xyw, r0.xyxx, t5.xywz, s6, l(0.000000)
    r0.xyw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // 23: mul_sat r0.z, r0.z, r3.w
    r0.z = (saturate((r0.zzzz)*(r3.wwww))).z;
    // 24: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 25: mul r4.xy, v4.xyxx, cb0[10].wwww
    r4.xy = ((v4.xyxx)*(source[10].wwww)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: mul r2.w, r4.w, r4.w
    r2.w = ((r4.wwww)*(r4.wwww)).w;
    // 28: mul r0.z, r0.z, r2.w
    r0.z = ((r0.zzzz)*(r2.wwww)).z;
    // 29: max r2.w, cb0[10].z, l(0.000000)
    r2.w = (max(source[10].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 31: mul r3.w, r0.z, r2.w
    r3.w = ((r0.zzzz)*(r2.wwww)).w;
    // 32: add r4.w, -v2.x, l(1.000000)
    r4.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r3.w, r4.w, r3.w, r4.w
    r3.w = ((r4.wwww)*(r3.wwww)+(r4.wwww)).w;
    // 34: add r4.w, -r2.w, r3.w
    r4.w = ((-(r2.wwww))+(r3.wwww)).w;
    // 35: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((((r2.wwww) != 0.f ? 1.f : 0.f) / ((r2.wwww) != 0.f ? (r2.wwww) : 1.f))).w;
    // 37: mad r3.w, -r2.w, r4.w, r3.w
    r3.w = ((-(r2.wwww))*(r4.wwww)+(r3.wwww)).w;
    // 38: mul r2.w, r4.w, r2.w
    r2.w = ((r4.wwww)*(r2.wwww)).w;
    // 39: mad_sat r0.z, r0.z, r3.w, r2.w
    r0.z = (saturate((r0.zzzz)*(r3.wwww)+(r2.wwww))).z;
    // 40: mul r2.w, r0.z, l(0.650000)
    r2.w = ((r0.zzzz)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 41: mad r1.xyw, r2.wwww, r2.xyxz, r1.xyxw
    r1.xyw = ((r2.wwww)*(r2.xyxz)+(r1.xyxw)).xyw;
    // 42: dp3 r2.x, r1.xywx, r1.xywx
    r2.x = (dot((r1.xywx).xyz,(r1.xywx).xyz).xxxx).x;
    // 43: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 44: mul r1.xyw, r1.xyxw, r2.xxxx
    r1.xyw = ((r1.xyxw)*(r2.xxxx)).xyw;
    // 45: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 46: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 47: mul r2.xyz, r2.xxxx, v5.xyzx
    r2.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 48: dp3 r2.w, r1.xywx, r2.xyzx
    r2.w = (dot((r1.xywx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 49: mul r5.xyz, r1.xywx, r2.wwww
    r5.xyz = ((r1.xywx)*(r2.wwww)).xyz;
    // 50: mad r2.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 51: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 52: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 53: mul r5.xyz, r2.wwww, v1.xyzx
    r5.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 54: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r6.xyz, r2.wwww, v0.xyzx
    r6.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 57: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 58: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 59: dp3 r5.z, r5.xyzx, r1.xywx
    r5.z = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // 60: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 61: dp3 r8.y, r7.xyzx, r2.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 62: dp3 r5.y, r7.xyzx, r1.xywx
    r5.y = (dot((r7.xyzx).xyz,(r1.xywx).xyz).xxxx).y;
    // 63: mul r7.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r7.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 64: dp3 r8.x, r6.xyzx, r2.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 65: dp3 r5.x, r6.xyzx, r1.xywx
    r5.x = (dot((r6.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // 66: mad r6.xy, cb0[11].yyyy, r8.xyxx, r7.xyxx
    r6.xy = ((source[11].yyyy)*(r8.xyxx)+(r7.xyxx)).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r6.xyz, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r6.xyzx)*(source[5].xyzx)).xyz;
    // 69: mad r6.xyz, cb0[11].zzzz, r6.xyzx, r6.xyzx
    r6.xyz = ((source[11].zzzz)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 70: add r6.xyz, r6.xyzx, -cb0[11].zzzz
    r6.xyz = ((r6.xyzx)+(-(source[11].zzzz))).xyz;
    // 71: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 72: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 73: mad r6.xyz, -r1.zzzz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r1.zzzz))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: add r8.xyz, -r3.xyzx, r2.wwww
    r8.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 76: mad r3.xyz, cb0[12].yyyy, r8.xyzx, r3.xyzx
    r3.xyz = ((source[12].yyyy)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 77: mul r8.xyz, cb0[6].xyzx, cb0[12].zzzz
    r8.xyz = ((source[6].xyzx)*(source[12].zzzz)).xyz;
    // 78: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 79: mad r3.xyz, r1.zzzz, r7.xyzx, r3.xyzx
    r3.xyz = ((r1.zzzz)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 80: mul r3.xyz, r6.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 81: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 82: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 83: mul r6.xyz, cb0[7].xyzx, cb0[12].wwww
    r6.xyz = ((source[7].xyzx)*(source[12].wwww)).xyz;
    // 84: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 85: dp3 r1.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 86: mad r6.xyz, -r6.xyzx, r4.xyzx, r1.zzzz
    r6.xyz = ((-(r6.xyzx))*(r4.xyzx)+(r1.zzzz)).xyz;
    // 87: mad r6.xyz, cb0[13].yyyy, r6.xyzx, r7.xyzx
    r6.xyz = ((source[13].yyyy)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 88: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 89: mad r3.xyz, r0.zzzz, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 90: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 91: dp3 r1.z, v6.xyzx, v6.xyzx
    r1.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 92: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 93: mul r6.xyz, r1.zzzz, v6.xyzx
    r6.xyz = ((r1.zzzz)*(v6.xyzx)).xyz;
    // 94: dp3 r1.z, r6.xyzx, r1.xywx
    r1.z = (dot((r6.xyzx).xyz,(r1.xywx).xyz).xxxx).z;
    // 95: mad r6.xy, r1.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 97: mul r6.yzw, r6.yyyy, cb0[16].xxyz
    r6.yzw = ((r6.yyyy)*(source[16].xxyz)).yzw;
    // 98: mad r6.xyz, r6.xxxx, cb0[15].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[15].xyzx)+(r6.yzwy)).xyz;
    // 99: mul r6.xyz, r6.xyzx, cb0[17].wwww
    r6.xyz = ((r6.xyzx)*(source[17].wwww)).xyz;
    // 100: mul r7.xyz, r3.xyzx, r6.xyzx
    r7.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 101: dp2_sat r8.x, r1.ywyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r1.ywyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 102: dp3_sat r8.y, r1.xywx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r1.xywx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 103: dp3_sat r8.z, r1.xywx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r1.xywx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 104: mul r1.xyz, r8.xyzx, r8.xyzx
    r1.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 105: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s7
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 106: mul r8.xyz, r8.xyzx, cb0[19].xyzx
    r8.xyz = ((r8.xyzx)*(source[19].xyzx)).xyz;
    // 107: dp3 r1.x, r8.xyzx, r1.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 108: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t7.wxyz, s7
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 109: mul r1.yzw, r1.yyzw, cb0[18].xxyz
    r1.yzw = ((r1.yyzw)*(source[18].xxyz)).yzw;
    // 110: mul r9.xyz, r1.xxxx, r1.yzwy
    r9.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 111: mad r6.xyz, r1.yzwy, r1.xxxx, r6.xyzx
    r6.xyz = ((r1.yzwy)*(r1.xxxx)+(r6.xyzx)).xyz;
    // 112: add r6.xyz, r6.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 113: div r6.xyz, r9.xyzx, r6.xyzx
    r6.xyz = ((r9.xyzx)/(r6.xyzx)).xyz;
    // 114: mad r7.xyz, r3.xyzx, r9.xyzx, r7.xyzx
    r7.xyz = ((r3.xyzx)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 115: dp3 r1.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 116: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 117: add r6.xyz, -r4.xyzx, r2.wwww
    r6.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 118: mad r4.xyz, cb0[13].yyyy, r6.xyzx, r4.xyzx
    r4.xyz = ((source[13].yyyy)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 119: mul r6.xyz, cb0[8].xyzx, cb0[13].zzzz
    r6.xyz = ((source[8].xyzx)*(source[13].zzzz)).xyz;
    // 120: mul r0.xyw, r0.xyxw, r6.xyxz
    r0.xyw = ((r0.xyxw)*(r6.xyxz)).xyw;
    // 121: mad r4.xyz, cb0[13].wwww, r4.xyzx, -r0.xywx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(-(r0.xywx))).xyz;
    // 122: mad r0.xyz, r0.zzzz, r4.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r4.xyzx)+(r0.xywx)).xyz;
    // 123: mad r0.xyz, r0.xyzx, cb2[4].wwww, cb2[4].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 124: mul r0.xyz, r1.yzwy, r0.xyzx
    r0.xyz = ((r1.yzwy)*(r0.xyzx)).xyz;
    // 125: dp2_sat r4.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 126: dp3_sat r4.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 127: dp3_sat r4.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 128: log r1.yzw, r4.xxyz
    r1.yzw = (log2(r4.xxyz)).yzw;
    // 129: add r0.w, cb0[14].x, l(1.000000)
    r0.w = ((source[14].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: mul r1.yzw, r1.yyzw, r0.wwww
    r1.yzw = ((r1.yyzw)*(r0.wwww)).yzw;
    // 131: exp r1.yzw, r1.yyzw
    r1.yzw = (exp2(r1.yyzw)).yzw;
    // 132: dp3 r0.w, r8.xyzx, r1.yzwy
    r0.w = (dot((r8.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 133: mad r1.yzw, r0.xxyz, r0.wwww, r7.xxyz
    r1.yzw = ((r0.xxyz)*(r0.wwww)+(r7.xxyz)).yzw;
    // 134: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 135: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 136: mul r0.xy, v4.xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 137: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t6.xyzw, s4, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 138: mul r2.xyz, cb0[4].xyzx, cb0[11].xxxx
    r2.xyz = ((source[4].xyzx)*(source[11].xxxx)).xyz;
    // 139: mad r0.xyz, r0.xyzx, r2.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 140: add r0.xyz, r1.yzwy, r0.xyzx
    r0.xyz = ((r1.yzwy)+(r0.xyzx)).xyz;
    // 141: mad o0.xyz, r3.xyzx, cb0[17].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[17].xyzx)+(r0.xyzx)).xyz;
    // 142: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 143: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 144: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 145: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 146: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 147: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 148: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 149: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 150: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 151: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 152: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 153: movc r0.xy, r0.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 154: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 155: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 156: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 157: mul o4.z, r1.x, r1.y
    output.targets[4].z = ((r1.xxxx)*(r1.yyyy)).z;
    // 158: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 159: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 160: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 161: ret
    return output;
}

// source.character.static-map-native-219.v1 / source program b0237c59e377294986aeb4156d07831d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase219(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8].x=g_SourceCharacterBaseConstants[9].x;
    source[8].y=g_SourceCharacterBaseConstants[9].y;
    source[8].z=g_SourceCharacterBaseConstants[9].z;
    source[8].w=g_SourceCharacterBaseConstants[9].w;
    source[9].x=g_SourceCharacterBaseConstants[10].x;
    source[9].y=g_SourceCharacterBaseConstants[10].y;
    source[9].z=g_SourceCharacterBaseConstants[10].z;
    source[9].w=g_SourceCharacterBaseConstants[10].w;
    source[10].x=g_SourceCharacterBaseConstants[11].x;
    source[10].y=g_SourceCharacterBaseConstants[11].y;
    source[10].z=g_SourceCharacterBaseConstants[11].z;
    source[10].w=g_SourceCharacterBaseConstants[11].w;
    source[11].x=g_SourceCharacterBaseConstants[12].x;
    source[11].y=g_SourceCharacterBaseConstants[12].y;
    source[11].z=g_SourceCharacterBaseConstants[12].z;
    source[11].w=g_SourceCharacterBaseConstants[12].w;
    source[12].x=g_SourceCharacterBaseConstants[13].x;
    source[12].y=g_SourceCharacterBaseConstants[13].y;
    source[12].z=g_SourceCharacterBaseConstants[13].z;
    source[12].w=g_SourceCharacterBaseConstants[13].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: mul r0.zw, r0.xxxy, cb0[9].xxxx
    r0.zw = ((r0.xxxy)*(source[9].xxxx)).zw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 4: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 5: mul r0.zw, r0.zzzw, cb0[9].yyyy
    r0.zw = ((r0.zzzw)*(source[9].yyyy)).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 8: mad r0.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: mul r1.x, r1.z, cb0[10].w
    r1.x = ((r1.zzzz)*(source[10].wwww)).x;
    // 10: mad r0.zw, cb0[8].wwww, r0.xxxy, r0.zzzw
    r0.zw = ((source[8].wwww)*(r0.xxxy)+(r0.zzzw)).zw;
    // 11: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 12: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 14: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 15: add r3.z, r0.x, l(0.000010)
    r3.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r3.xy, r0.zwzz, v2.wwww
    r3.xy = ((r0.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 18: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 19: div r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 20: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 21: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mul r1.yz, v4.xxyx, cb0[9].wwww
    r1.yz = ((v4.xxyx)*(source[9].wwww)).yz;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 25: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 26: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 27: max r1.y, cb0[9].z, l(0.000000)
    r1.y = (max(source[9].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 28: min r1.y, r1.y, l(0.990000)
    r1.y = (min(r1.yyyy,float4(0.990000,0.990000,0.990000,0.990000))).y;
    // 29: mul r1.z, r0.w, r1.y
    r1.z = ((r0.wwww)*(r1.yyyy)).z;
    // 30: add r1.w, -v2.x, l(1.000000)
    r1.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mad r1.z, r1.w, r1.z, r1.w
    r1.z = ((r1.wwww)*(r1.zzzz)+(r1.wwww)).z;
    // 32: add r1.w, -r1.y, r1.z
    r1.w = ((-(r1.yyyy))+(r1.zzzz)).w;
    // 33: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = ((((r1.yyyy) != 0.f ? 1.f : 0.f) / ((r1.yyyy) != 0.f ? (r1.yyyy) : 1.f))).y;
    // 35: mad r1.z, -r1.y, r1.w, r1.z
    r1.z = ((-(r1.yyyy))*(r1.wwww)+(r1.zzzz)).z;
    // 36: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 37: mad_sat r0.w, r0.w, r1.z, r1.y
    r0.w = (saturate((r0.wwww)*(r1.zzzz)+(r1.yyyy))).w;
    // 38: mul r1.y, r0.w, l(0.650000)
    r1.y = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 39: add r4.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 40: mad r0.xyz, r1.yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 41: dp3 r1.y, r0.xyzx, r0.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 42: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 43: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 44: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 45: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 46: mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // 47: dp3 r2.w, r0.xyzx, r1.yzwy
    r2.w = (dot((r0.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 48: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 49: mad r1.yzw, r4.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), -r1.yyzw
    r1.yzw = ((r4.xxyz)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r1.yyzw))).yzw;
    // 50: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 51: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 52: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 53: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 54: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 55: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 56: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 57: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 58: dp3 r4.z, r4.xyzx, r0.xyzx
    r4.z = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 59: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 60: dp3 r7.y, r6.xyzx, r1.yzwy
    r7.y = (dot((r6.xyzx).xyz,(r1.yzwy).xyz).xxxx).y;
    // 61: dp3 r7.x, r5.xyzx, r1.yzwy
    r7.x = (dot((r5.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 62: dp3 r4.x, r5.xyzx, r0.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 63: dp3 r4.y, r6.xyzx, r0.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 64: mul r1.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r1.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 65: mad r1.yz, cb0[10].yyyy, r7.xxyx, r1.yyzy
    r1.yz = ((source[10].yyyy)*(r7.xxyx)+(r1.yyzy)).yz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s5, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 67: mul r1.yzw, r1.yyzw, cb0[5].xxyz
    r1.yzw = ((r1.yyzw)*(source[5].xxyz)).yzw;
    // 68: mad r1.yzw, cb0[10].zzzz, r1.yyzw, r1.yyzw
    r1.yzw = ((source[10].zzzz)*(r1.yyzw)+(r1.yyzw)).yzw;
    // 69: add r1.yzw, r1.yyzw, -cb0[10].zzzz
    r1.yzw = ((r1.yyzw)+(-(source[10].zzzz))).yzw;
    // 70: mov_sat r5.xyz, r1.yzwy
    r5.xyz = (saturate(r1.yzwy)).xyz;
    // 71: mov_sat r1.yzw, -r1.yyzw
    r1.yzw = (saturate(-(r1.yyzw))).yzw;
    // 72: mad r1.yzw, -r1.xxxx, r1.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(r1.xxxx))*(r1.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 73: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r6.xyz, -r2.xyzx, r2.wwww
    r6.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 75: mad r2.xyz, cb0[11].yyyy, r6.xyzx, r2.xyzx
    r2.xyz = ((source[11].yyyy)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 76: mul r6.xyz, cb0[6].xyzx, cb0[11].zzzz
    r6.xyz = ((source[6].xyzx)*(source[11].zzzz)).xyz;
    // 77: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 78: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 79: mul r1.xyz, r1.yzwy, r2.xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)).xyz;
    // 80: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 81: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 82: mul r2.xyz, cb0[7].xyzx, cb0[11].wwww
    r2.xyz = ((source[7].xyzx)*(source[11].wwww)).xyz;
    // 83: mul r5.xyz, r3.xyzx, r2.xyzx
    r5.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 84: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 85: mad r2.xyz, -r2.xyzx, r3.xyzx, r1.wwww
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 86: mad r2.xyz, cb0[12].yyyy, r2.xyzx, r5.xyzx
    r2.xyz = ((source[12].yyyy)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 87: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 88: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 89: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 90: mul r2.xy, v4.xyxx, cb0[3].xyxx
    r2.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 91: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 92: mul r3.xyz, cb0[4].xyzx, cb0[10].xxxx
    r3.xyz = ((source[4].xyzx)*(source[10].xxxx)).xyz;
    // 93: mad r2.xyz, r2.xyzx, r3.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // 94: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 97: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 98: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 100: mul r0.yzw, r0.yyyy, cb0[14].xxyz
    r0.yzw = ((r0.yyyy)*(source[14].xxyz)).yzw;
    // 101: mad r0.xyz, r0.xxxx, cb0[13].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[13].xyzx)+(r0.yzwy)).xyz;
    // 102: mul r0.xyz, r0.xyzx, cb0[15].wwww
    r0.xyz = ((r0.xyzx)*(source[15].wwww)).xyz;
    // 103: mad r2.xyz, r0.xyzx, r1.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 104: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 105: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 106: mad o0.xyz, r1.xyzx, cb0[15].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[15].xyzx)+(r2.xyzx)).xyz;
    // 107: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 108: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 110: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 111: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 112: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 113: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 114: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 115: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 116: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 117: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 118: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 119: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 120: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 121: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 122: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 123: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 124: ret
    return output;
}

// source.character.static-map-native-220.v1 / source program 79a3889502ce5d47a0993b92eca23510
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked220(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[25]=1.f; source[26]=1.f; // RNM samples already contain their instance scales.
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[6].yyyy
    r1.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r1.xy, r1.xyxx, cb0[6].zzzz
    r1.xy = ((r1.xyxx)*(source[6].zzzz)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 8: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 9: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 10: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 17: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 18: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 22: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 28: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 29: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: mul r0.zw, v4.xxxy, cb0[7].wwww
    r0.zw = ((v4.xxxy)*(source[7].wwww)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 41: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 42: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 44: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 45: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 46: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 47: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 49: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 50: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 51: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 52: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: add r0.yzw, -r3.xxyz, r0.yyyy
    r0.yzw = ((-(r3.xxyz))+(r0.yyyy)).yzw;
    // 54: mad r0.yzw, cb0[8].yyyy, r0.yyzw, r3.xxyz
    r0.yzw = ((source[8].yyyy)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 55: mul r3.xyz, cb0[5].xyzx, cb0[8].wwww
    r3.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 56: mul r7.xyz, r6.xyzx, r3.xyzx
    r7.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 57: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 58: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 59: mad r3.xyz, cb0[9].yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((source[9].yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 60: mul r6.xyz, cb0[4].xyzx, cb0[8].zzzz
    r6.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 61: mad r3.xyz, -r0.yzwy, r6.xyzx, r3.xyzx
    r3.xyz = ((-(r0.yzwy))*(r6.xyzx)+(r3.xyzx)).xyz;
    // 62: mul r0.yzw, r0.yyzw, r6.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)).yzw;
    // 63: mad r0.yzw, r0.xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 64: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 65: mul r3.xyz, r0.yzwy, cb0[9].zzzz
    r3.xyz = ((r0.yzwy)*(source[9].zzzz)).xyz;
    // 66: mad r0.yzw, cb0[9].wwww, r0.yyzw, -r3.xxyz
    r0.yzw = ((source[9].wwww)*(r0.yyzw)+(-(r3.xxyz))).yzw;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r1.w, r6.z, cb0[10].x
    r1.w = ((r6.zzzz)*(source[10].xxxx)).w;
    // 69: mul r6.xy, r6.yxyy, cb0[11].xzxx
    r6.xy = ((r6.yxyy)*(source[11].xzxx)).xy;
    // 70: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 71: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 72: mul r2.w, r2.w, cb0[10].y
    r2.w = ((r2.wwww)*(source[10].yyyy)).w;
    // 73: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 74: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 75: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 77: mad r0.yzw, r2.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r2.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 78: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 80: mad_sat r3.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 81: mad r0.yzw, r3.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 82: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 83: mad r8.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 84: log r9.xy, |r6.xyxx|
    r9.xy = (log2(abs(r6.xyxx))).xy;
    // 85: lt r6.xy, |r6.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r6.xy = (asfloat((uint4)((abs(r6.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: mul r9.xy, r9.xyxx, cb0[11].ywyy
    r9.xy = ((r9.xyxx)*(source[11].ywyy)).xy;
    // 87: exp r9.xy, r9.xyxx
    r9.xy = (exp2(r9.xyxx)).xy;
    // 88: min r1.w, r9.y, l(1.000000)
    r1.w = (min(r9.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: movc r2.w, r6.x, l(0), r9.x
    r2.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xxxx)).w;
    // 90: movc r1.w, r6.y, l(0), r1.w
    r1.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 91: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 92: min r6.z, r2.w, l(1.000000)
    r6.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 93: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 94: mad r0.yzw, r7.xxyz, r1.wwww, r0.yyzw
    r0.yzw = ((r7.xxyz)*(r1.wwww)+(r0.yyzw)).yzw;
    // 95: mul r0.yzw, r1.wwww, r0.yyzw
    r0.yzw = ((r1.wwww)*(r0.yyzw)).yzw;
    // 96: max r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = (max(r0.yyzw,r1.wwww)).yzw;
    // 97: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 98: mad r1.xyz, r0.xxxx, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 99: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 102: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 103: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 104: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 105: dp3 r0.x, r8.xyzx, r7.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 106: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 108: mul r8.xyz, r6.yyyy, cb0[23].xyzx
    r8.xyz = ((r6.yyyy)*(source[23].xyzx)).xyz;
    // 109: mad r8.xyz, r6.xxxx, cb0[22].xyzx, r8.xyzx
    r8.xyz = ((r6.xxxx)*(source[22].xyzx)+(r8.xyzx)).xyz;
    // 110: mul r8.xyz, r8.xyzx, cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(source[24].wwww)).xyz;
    // 111: mul r9.xyz, r3.xyzx, r8.xyzx
    r9.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 112: dp2_sat r10.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 113: dp3_sat r10.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 114: dp3_sat r10.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 115: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 116: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 117: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 118: dp3 r0.x, r11.xyzx, r10.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 119: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 120: mul r10.xyz, r10.xyzx, cb0[25].xyzx
    r10.xyz = ((r10.xyzx)*(source[25].xyzx)).xyz;
    // 121: mul r12.xyz, r0.xxxx, r10.xyzx
    r12.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 122: mad r9.xyz, r3.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 123: mul r0.yzw, r0.yyzw, r9.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)).yzw;
    // 124: dp3 r9.x, r2.xyzx, r7.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 125: dp3 r9.y, r5.xyzx, r7.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 126: dp2 r12.z, r9.xyxx, cb0[13].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 127: dp3 r12.y, r4.xyzx, r7.xyzx
    r12.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 128: mul r6.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 129: dp2 r12.x, r9.xyxx, r6.xyxx
    r12.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 130: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 131: dp4 r13.x, cb0[14].xyzw, r12.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 132: dp4 r13.y, cb0[15].xyzw, r12.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 133: dp4 r13.z, cb0[16].xyzw, r12.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 134: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 135: dp4 r15.x, cb0[17].xyzw, r14.xyzw
    r15.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 136: dp4 r15.y, cb0[18].xyzw, r14.xyzw
    r15.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 137: dp4 r15.z, cb0[19].xyzw, r14.xyzw
    r15.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 138: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 139: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 140: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 141: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 142: mad r12.xyz, cb0[20].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[20].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 143: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 144: mul r12.xyz, r12.xyzx, cb0[12].xyzx
    r12.xyz = ((r12.xyzx)*(source[12].xyzx)).xyz;
    // 145: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 146: mov_sat r3.w, cb0[10].z
    r3.w = (saturate(source[10].zzzz)).w;
    // 147: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 148: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 149: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 150: mad r13.xyz, r6.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r6.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 151: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 152: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 153: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 154: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 155: dp3 r3.w, r7.xyzx, r14.xyzx
    r3.w = (dot((r7.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 156: mul r7.xyz, r3.wwww, r7.xyzx
    r7.xyz = ((r3.wwww)*(r7.xyzx)).xyz;
    // 157: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 158: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 159: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 160: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 162: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 163: mad_sat r15.y, r4.w, l(0.300000), r6.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 164: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 166: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 167: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 168: add r2.w, r7.z, l(1.000000)
    r2.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 171: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 172: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 173: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 174: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 175: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 176: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = ((((r15.wwww) != 0.f ? 1.f : 0.f) / ((r15.wwww) != 0.f ? (r15.wwww) : 1.f))).w;
    // 177: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 178: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 179: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 180: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 181: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 183: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 184: mul r0.yzw, r0.yyzw, r12.xxyz
    r0.yzw = ((r0.yyzw)*(r12.xxyz)).yzw;
    // 185: mad r0.yzw, -r0.yyzw, r6.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r6.wwww)+(r0.yyzw)).yzw;
    // 186: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 187: dp3 r2.x, r2.xyzx, r7.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 188: dp3 r2.y, r5.xyzx, r7.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 189: dp2 r5.x, r2.xyxx, r6.xyxx
    r5.x = (dot((r2.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 190: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 191: mul r2.x, r15.y, l(5.000000)
    r2.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 192: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 193: mul r2.y, r2.w, r2.y
    r2.y = ((r2.wwww)*(r2.yyyy)).y;
    // 194: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 195: add r2.y, r1.w, r2.y
    r2.y = ((r1.wwww)+(r2.yyyy)).y;
    // 196: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 197: add_sat r1.w, r2.y, l(-1.000000)
    r1.w = (saturate((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 198: dp3 r5.y, r4.xyzx, r7.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 199: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s6, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 200: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 201: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 202: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 203: dp2_sat r4.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 204: dp3_sat r4.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 205: dp3_sat r4.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 206: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 207: dp3 r2.w, r11.xyzx, r4.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 208: add r0.x, r0.x, -r2.w
    r0.x = ((r0.xxxx)+(-(r2.wwww))).x;
    // 209: mad r0.x, r6.z, r0.x, r2.w
    r0.x = ((r6.zzzz)*(r0.xxxx)+(r2.wwww)).x;
    // 210: mad r4.xyz, r10.xyzx, r0.xxxx, r8.xyzx
    r4.xyz = ((r10.xyzx)*(r0.xxxx)+(r8.xyzx)).xyz;
    // 211: mul r5.xyz, r0.xxxx, r10.xyzx
    r5.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 212: mad r0.x, r1.w, r13.x, r13.y
    r0.x = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).x;
    // 213: mad r0.x, r0.x, r1.w, r13.z
    r0.x = ((r0.xxxx)*(r1.wwww)+(r13.zzzz)).x;
    // 214: mul r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)*(r0.xxxx)).x;
    // 215: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 216: mul r6.xyz, r0.xxxx, r4.xyzx
    r6.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 217: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 218: div r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)/(r4.xyzx)).xyz;
    // 219: dp3 r0.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 220: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 221: mad r0.yzw, r2.xxyz, r15.xxzw, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r15.xxzw)+(r0.yyzw)).yzw;
    // 222: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 223: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 224: dp3 r1.x, r1.xyzx, r14.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 225: add r1.y, -|r14.z|, l(1.000000)
    r1.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 226: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 227: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 228: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 229: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 230: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 231: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 232: mul r1.xzw, r1.xxxx, cb0[3].xxyz
    r1.xzw = ((r1.xxxx)*(source[3].xxyz)).xzw;
    // 233: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 234: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 235: add r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 236: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 237: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 238: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 239: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 240: mul r1.xyz, r1.xxxx, r9.xyzx
    r1.xyz = ((r1.xxxx)*(r9.xyzx)).xyz;
    // 241: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 242: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 243: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 244: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 245: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 246: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 247: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 248: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 249: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 250: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 251: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 252: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 253: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 254: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 255: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 256: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 257: ret
    return output;
}

// source.character.static-map-native-220.v1 / source program 13794a555393054d8c8e645595dd7b09
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase220(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6].x=g_SourceCharacterBaseConstants[6].x;
    source[6].y=g_SourceCharacterBaseConstants[6].y;
    source[6].z=g_SourceCharacterBaseConstants[6].z;
    source[6].w=g_SourceCharacterBaseConstants[6].w;
    source[7].x=g_SourceCharacterBaseConstants[7].x;
    source[7].y=g_SourceCharacterBaseConstants[7].y;
    source[7].z=g_SourceCharacterBaseConstants[7].z;
    source[7].w=g_SourceCharacterBaseConstants[7].w;
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[9].x=g_SourceCharacterBaseConstants[9].x;
    source[9].y=g_SourceCharacterBaseConstants[9].y;
    source[9].z=g_SourceCharacterBaseConstants[9].z;
    source[9].w=g_SourceCharacterBaseConstants[9].w;
    source[10].x=g_SourceCharacterBaseConstants[10].x;
    source[10].y=g_SourceCharacterBaseConstants[10].y;
    source[10].z=g_SourceCharacterBaseConstants[10].z;
    source[10].w=g_SourceCharacterBaseConstants[10].w;
    source[11].x=g_SourceCharacterBaseConstants[11].x;
    source[11].y=g_SourceCharacterBaseConstants[11].y;
    source[11].z=g_SourceCharacterBaseConstants[11].z;
    source[11].w=g_SourceCharacterBaseConstants[11].w;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[6].yyyy
    r1.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r1.xy, r1.xyxx, cb0[6].zzzz
    r1.xy = ((r1.xyxx)*(source[6].zzzz)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 8: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 9: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 10: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 16: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 17: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 18: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 22: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 28: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 29: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: mul r0.zw, v4.xxxy, cb0[7].wwww
    r0.zw = ((v4.xxxy)*(source[7].wwww)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 41: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 42: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 43: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 44: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 45: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 46: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 47: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 48: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 49: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 50: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 51: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 52: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: add r0.yzw, -r3.xxyz, r0.yyyy
    r0.yzw = ((-(r3.xxyz))+(r0.yyyy)).yzw;
    // 54: mad r0.yzw, cb0[8].yyyy, r0.yyzw, r3.xxyz
    r0.yzw = ((source[8].yyyy)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 55: mul r3.xyz, cb0[5].xyzx, cb0[8].wwww
    r3.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 56: mul r7.xyz, r6.xyzx, r3.xyzx
    r7.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 57: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 58: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 59: mad r3.xyz, cb0[9].yyyy, r3.xyzx, r7.xyzx
    r3.xyz = ((source[9].yyyy)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 60: mul r6.xyz, cb0[4].xyzx, cb0[8].zzzz
    r6.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 61: mad r3.xyz, -r0.yzwy, r6.xyzx, r3.xyzx
    r3.xyz = ((-(r0.yzwy))*(r6.xyzx)+(r3.xyzx)).xyz;
    // 62: mul r0.yzw, r0.yyzw, r6.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)).yzw;
    // 63: mad r0.yzw, r0.xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 64: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 65: mul r3.xyz, r0.yzwy, cb0[9].zzzz
    r3.xyz = ((r0.yzwy)*(source[9].zzzz)).xyz;
    // 66: mad r0.yzw, cb0[9].wwww, r0.yyzw, -r3.xxyz
    r0.yzw = ((source[9].wwww)*(r0.yyzw)+(-(r3.xxyz))).yzw;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r1.w, r6.z, cb0[10].x
    r1.w = ((r6.zzzz)*(source[10].xxxx)).w;
    // 69: mul r6.xy, r6.yxyy, cb0[11].xzxx
    r6.xy = ((r6.yxyy)*(source[11].xzxx)).xy;
    // 70: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 71: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 72: mul r2.w, r2.w, cb0[10].y
    r2.w = ((r2.wwww)*(source[10].yyyy)).w;
    // 73: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 74: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 75: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 77: mad r0.yzw, r2.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r2.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 78: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 80: mad_sat r3.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 81: mad r0.yzw, r3.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 82: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 83: mad r8.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 84: log r9.xy, |r6.xyxx|
    r9.xy = (log2(abs(r6.xyxx))).xy;
    // 85: lt r6.xy, |r6.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r6.xy = (asfloat((uint4)((abs(r6.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: mul r9.xy, r9.xyxx, cb0[11].ywyy
    r9.xy = ((r9.xyxx)*(source[11].ywyy)).xy;
    // 87: exp r9.xy, r9.xyxx
    r9.xy = (exp2(r9.xyxx)).xy;
    // 88: min r1.w, r9.y, l(1.000000)
    r1.w = (min(r9.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: movc r2.w, r6.x, l(0), r9.x
    r2.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r9.xxxx)).w;
    // 90: movc r1.w, r6.y, l(0), r1.w
    r1.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 91: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 92: min r6.z, r2.w, l(1.000000)
    r6.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 93: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 94: mad r0.yzw, r7.xxyz, r1.wwww, r0.yyzw
    r0.yzw = ((r7.xxyz)*(r1.wwww)+(r0.yyzw)).yzw;
    // 95: mul r0.yzw, r1.wwww, r0.yyzw
    r0.yzw = ((r1.wwww)*(r0.yyzw)).yzw;
    // 96: max r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = (max(r0.yyzw,r1.wwww)).yzw;
    // 97: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 98: mad r1.xyz, r0.xxxx, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 99: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 102: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 103: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 104: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 105: dp3 r0.x, r8.xyzx, r7.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 106: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 108: mul r9.xyz, r6.yyyy, cb0[23].xyzx
    r9.xyz = ((r6.yyyy)*(source[23].xyzx)).xyz;
    // 109: mad r9.xyz, r6.xxxx, cb0[22].xyzx, r9.xyzx
    r9.xyz = ((r6.xxxx)*(source[22].xyzx)+(r9.xyzx)).xyz;
    // 110: mul r9.xyz, r9.xyzx, cb0[24].wwww
    r9.xyz = ((r9.xyzx)*(source[24].wwww)).xyz;
    // 111: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 112: mul r0.xyz, r0.yzwy, r9.xyzx
    r0.xyz = ((r0.yzwy)*(r9.xyzx)).xyz;
    // 113: dp3 r9.x, r2.xyzx, r7.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 114: dp3 r9.y, r5.xyzx, r7.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 115: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 116: dp3 r10.y, r4.xyzx, r7.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 117: mul r6.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 118: dp2 r10.x, r9.xyxx, r6.xyxx
    r10.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 119: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 120: dp4 r11.x, cb0[14].xyzw, r10.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 121: dp4 r11.y, cb0[15].xyzw, r10.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 122: dp4 r11.z, cb0[16].xyzw, r10.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 123: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 124: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 125: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 126: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 127: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 128: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 129: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 130: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 131: mad r10.xyz, cb0[20].xyzx, r0.wwww, r11.xyzx
    r10.xyz = ((source[20].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 132: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 133: mul r10.xyz, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((r10.xyzx)*(source[12].xyzx)).xyz;
    // 134: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 135: mov_sat r3.w, cb0[10].z
    r3.w = (saturate(source[10].zzzz)).w;
    // 136: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 137: mul r0.w, r3.w, l(0.080000)
    r0.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 138: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 139: mad r11.xyz, r6.wwww, r11.xyzx, r0.wwww
    r11.xyz = ((r6.wwww)*(r11.xyzx)+(r0.wwww)).xyz;
    // 140: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 141: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 142: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 143: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 144: dp3 r2.w, r7.xyzx, r12.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 145: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 146: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 147: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 148: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 149: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 151: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 152: mad_sat r13.y, r3.w, l(0.300000), r6.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 153: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 154: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 156: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 157: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 158: add r0.w, r7.z, l(1.000000)
    r0.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add_sat r13.x, -r0.w, r2.w
    r13.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 161: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 162: add r0.w, r1.w, r13.x
    r0.w = ((r1.wwww)+(r13.xxxx)).w;
    // 163: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 164: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 165: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 166: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = ((((r13.wwww) != 0.f ? 1.f : 0.f) / ((r13.wwww) != 0.f ? (r13.wwww) : 1.f))).w;
    // 167: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 168: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 169: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 171: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 173: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 174: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 175: mad r0.xyz, -r0.xyzx, r6.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r6.wwww)+(r0.xyzx)).xyz;
    // 176: dp3 r2.x, r2.xyzx, r7.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 177: dp3 r2.y, r5.xyzx, r7.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 178: dp2 r5.x, r2.xyxx, r6.xyxx
    r5.x = (dot((r2.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 179: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 180: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 181: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 182: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 183: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 184: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 185: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 186: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 187: dp3 r5.y, r4.xyzx, r7.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 188: dp3 r1.w, r8.xyzx, r7.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 189: mad r2.yz, r1.wwww, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r1.wwww)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 190: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 191: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r5.xyzx, t6.xyzw, s5, r2.x
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 192: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 193: mul r4.xyz, r4.xyzx, cb0[12].xyzx
    r4.xyz = ((r4.xyzx)*(source[12].xyzx)).xyz;
    // 194: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 195: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 196: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 197: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 198: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 199: mul r2.xzw, r2.zzzz, cb0[23].xxyz
    r2.xzw = ((r2.zzzz)*(source[23].xxyz)).xzw;
    // 200: mad r2.xyz, cb0[22].xyzx, r2.yyyy, r2.xzwx
    r2.xyz = ((source[22].xyzx)*(r2.yyyy)+(r2.xzwx)).xyz;
    // 201: mul r2.xyz, r2.xyzx, cb0[24].wwww
    r2.xyz = ((r2.xyzx)*(source[24].wwww)).xyz;
    // 202: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 203: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 204: mad r0.xyz, r2.xyzx, r13.xzwx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r13.xzwx)+(r0.xyzx)).xyz;
    // 205: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 206: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 207: dp3 r0.w, r1.xyzx, r12.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 208: add r1.x, -|r12.z|, l(1.000000)
    r1.x = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 209: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 211: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 212: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 213: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 214: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 215: mul r1.yzw, r0.wwww, cb0[3].xxyz
    r1.yzw = ((r0.wwww)*(source[3].xxyz)).yzw;
    // 216: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 217: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 218: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 219: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 220: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 221: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 222: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 223: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 224: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 225: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 226: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 227: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 228: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 229: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 230: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 231: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 232: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 233: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 234: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 235: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 236: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 237: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 238: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 239: ret
    return output;
}

// source.character.static-map-native-221.v1 / source program 4586962aee07dc4882638663fccddb19
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked221(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[25]=1.f; source[26]=1.f; // RNM samples already contain their instance scales.
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0, r16=0.0, r17=0.0;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[7].xxxx
    r0.xy = ((source[0].xyxx)*(source[7].xxxx)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, v4.xyxx, cb0[6].yyyy
    r1.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r1.xy, r1.xyxx, cb0[6].zzzz
    r1.xy = ((r1.xyxx)*(source[6].zzzz)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 9: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 11: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 24: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 27: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 28: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 29: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 30: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 31: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 36: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[8].xxxx
    r0.zw = ((v4.xxxy)*(source[8].xxxx)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 48: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 51: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.yzw, cb0[5].xxyz, cb0[8].zzzz
    r0.yzw = ((source[5].xxyz)*(source[8].zzzz)).yzw;
    // 55: mul r7.xyz, r6.xyzx, r0.yzwy
    r7.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 56: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: mad r0.yzw, -r0.yyzw, r6.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r6.xxyz)+(r1.wwww)).yzw;
    // 58: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r7.xxyz
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r7.xxyz)).yzw;
    // 59: mul r6.xyz, cb0[4].xyzx, cb0[8].yyyy
    r6.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 60: mad r0.yzw, -r3.xxyz, r6.xxyz, r0.yyzw
    r0.yzw = ((-(r3.xxyz))*(r6.xxyz)+(r0.yyzw)).yzw;
    // 61: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 62: mad r0.yzw, r0.xxxx, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 63: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 64: mul r3.xyz, r0.yzwy, cb0[9].yyyy
    r3.xyz = ((r0.yzwy)*(source[9].yyyy)).xyz;
    // 65: mad r0.yzw, cb0[9].zzzz, r0.yyzw, -r3.xxyz
    r0.yzw = ((source[9].zzzz)*(r0.yyzw)+(-(r3.xxyz))).yzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r1.w, r6.z, cb0[9].w
    r1.w = ((r6.zzzz)*(source[9].wwww)).w;
    // 68: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 69: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 70: mul r2.w, r2.w, cb0[10].x
    r2.w = ((r2.wwww)*(source[10].xxxx)).w;
    // 71: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 72: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 73: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 75: mad r0.yzw, r2.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r2.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 76: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 78: mad_sat r3.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 79: mad r0.yzw, r3.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 80: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 81: mad r8.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 82: mul r1.w, r6.x, cb0[11].y
    r1.w = ((r6.xxxx)*(source[11].yyyy)).w;
    // 83: mul r2.w, r6.y, cb0[10].w
    r2.w = ((r6.yyyy)*(source[10].wwww)).w;
    // 84: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 85: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: mul r4.w, r4.w, cb0[11].z
    r4.w = ((r4.wwww)*(source[11].zzzz)).w;
    // 87: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 88: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 90: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 91: mad r0.yzw, r7.xxyz, r1.wwww, r0.yyzw
    r0.yzw = ((r7.xxyz)*(r1.wwww)+(r0.yyzw)).yzw;
    // 92: mul r0.yzw, r1.wwww, r0.yyzw
    r0.yzw = ((r1.wwww)*(r0.yyzw)).yzw;
    // 93: max r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = (max(r0.yyzw,r1.wwww)).yzw;
    // 94: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 95: mad r1.xyz, r0.xxxx, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 96: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 97: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 98: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 99: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 102: dp3 r0.x, r8.xyzx, r7.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 103: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 104: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 105: mul r8.xyz, r6.yyyy, cb0[23].xyzx
    r8.xyz = ((r6.yyyy)*(source[23].xyzx)).xyz;
    // 106: mad r8.xyz, r6.xxxx, cb0[22].xyzx, r8.xyzx
    r8.xyz = ((r6.xxxx)*(source[22].xyzx)+(r8.xyzx)).xyz;
    // 107: mul r8.xyz, r8.xyzx, cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(source[24].wwww)).xyz;
    // 108: mul r9.xyz, r3.xyzx, r8.xyzx
    r9.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 109: dp2_sat r10.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 110: dp3_sat r10.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 111: dp3_sat r10.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 112: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 113: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 114: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 115: dp3 r0.x, r11.xyzx, r10.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 116: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 117: mul r10.xyz, r10.xyzx, cb0[25].xyzx
    r10.xyz = ((r10.xyzx)*(source[25].xyzx)).xyz;
    // 118: mul r12.xyz, r0.xxxx, r10.xyzx
    r12.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 119: mad r9.xyz, r3.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 120: mul r0.yzw, r0.yyzw, r9.xxyz
    r0.yzw = ((r0.yyzw)*(r9.xxyz)).yzw;
    // 121: dp3 r9.x, r2.xyzx, r7.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 122: dp3 r9.y, r5.xyzx, r7.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 123: dp2 r12.z, r9.xyxx, cb0[13].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 124: dp3 r12.y, r4.xyzx, r7.xyzx
    r12.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 125: mul r6.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 126: dp2 r12.x, r9.xyxx, r6.xyxx
    r12.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 127: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 128: dp4 r13.x, cb0[14].xyzw, r12.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 129: dp4 r13.y, cb0[15].xyzw, r12.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 130: dp4 r13.z, cb0[16].xyzw, r12.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 131: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 132: dp4 r15.x, cb0[17].xyzw, r14.xyzw
    r15.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 133: dp4 r15.y, cb0[18].xyzw, r14.xyzw
    r15.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 134: dp4 r15.z, cb0[19].xyzw, r14.xyzw
    r15.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 135: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 136: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 137: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 138: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 139: mad r12.xyz, cb0[20].xyzx, r4.wwww, r13.xyzx
    r12.xyz = ((source[20].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 140: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 141: mul r12.xyz, r12.xyzx, cb0[12].xyzx
    r12.xyz = ((r12.xyzx)*(source[12].xyzx)).xyz;
    // 142: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 143: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 144: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 145: mul r4.w, r3.w, l(0.080000)
    r4.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 146: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 147: mad r13.xyz, r6.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r6.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 148: mul_sat r3.w, r13.y, l(50.000000)
    r3.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 149: log r4.w, |r2.w|
    r4.w = (log2(abs(r2.wwww))).w;
    // 150: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 151: mul r4.w, r4.w, cb0[11].x
    r4.w = ((r4.wwww)*(source[11].xxxx)).w;
    // 152: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 153: movc r2.w, r2.w, l(0), r4.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 154: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 155: min r6.z, r2.w, l(1.000000)
    r6.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 156: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 157: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 158: mul r14.xyz, r2.wwww, v5.xyzx
    r14.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 159: dp3 r2.w, r7.xyzx, r14.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 160: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 161: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 162: deriv_rtx_coarse r15.x, r2.w
    r15.x = (ddx_coarse(r2.wwww)).x;
    // 163: deriv_rty_coarse r15.y, r2.w
    r15.y = (ddy_coarse(r2.wwww)).y;
    // 164: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 166: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 167: mad_sat r15.y, r4.w, l(0.300000), r6.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 168: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 170: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 171: mul r16.xyz, r3.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r16.xyzx)).xyz;
    // 172: add r3.w, r7.z, l(1.000000)
    r3.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: add_sat r15.x, r2.w, -r3.w
    r15.x = (saturate((r2.wwww)+(-(r3.wwww)))).x;
    // 175: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 176: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 177: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 178: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 179: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 180: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = ((((r15.wwww) != 0.f ? 1.f : 0.f) / ((r15.wwww) != 0.f ? (r15.wwww) : 1.f))).w;
    // 181: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 182: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 183: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 184: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 185: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 186: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 187: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 188: mul r0.yzw, r0.yyzw, r12.xxyz
    r0.yzw = ((r0.yyzw)*(r12.xxyz)).yzw;
    // 189: mad r0.yzw, -r0.yyzw, r6.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r6.wwww)+(r0.yyzw)).yzw;
    // 190: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 191: dp3 r2.x, r2.xyzx, r7.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 192: dp3 r2.y, r5.xyzx, r7.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 193: dp2 r5.x, r2.xyxx, r6.xyxx
    r5.x = (dot((r2.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 194: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 195: mul r2.x, r15.y, l(5.000000)
    r2.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 196: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 197: mul r2.y, r2.w, r2.y
    r2.y = ((r2.wwww)*(r2.yyyy)).y;
    // 198: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 199: add r2.y, r1.w, r2.y
    r2.y = ((r1.wwww)+(r2.yyyy)).y;
    // 200: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 201: add_sat r1.w, r2.y, l(-1.000000)
    r1.w = (saturate((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 202: dp3 r5.y, r4.xyzx, r7.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 203: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s6, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 204: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 205: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 206: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 207: dp2_sat r4.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 208: dp3_sat r4.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 209: dp3_sat r4.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 210: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 211: dp3 r2.w, r11.xyzx, r4.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 212: add r0.x, r0.x, -r2.w
    r0.x = ((r0.xxxx)+(-(r2.wwww))).x;
    // 213: mad r0.x, r6.z, r0.x, r2.w
    r0.x = ((r6.zzzz)*(r0.xxxx)+(r2.wwww)).x;
    // 214: mad r4.xyz, r10.xyzx, r0.xxxx, r8.xyzx
    r4.xyz = ((r10.xyzx)*(r0.xxxx)+(r8.xyzx)).xyz;
    // 215: mul r5.xyz, r0.xxxx, r10.xyzx
    r5.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 216: mad r0.x, r1.w, r13.x, r13.y
    r0.x = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).x;
    // 217: mad r0.x, r0.x, r1.w, r13.z
    r0.x = ((r0.xxxx)*(r1.wwww)+(r13.zzzz)).x;
    // 218: mul r0.x, r1.w, r0.x
    r0.x = ((r1.wwww)*(r0.xxxx)).x;
    // 219: max r0.x, r0.x, r1.w
    r0.x = (max(r0.xxxx,r1.wwww)).x;
    // 220: mul r6.xyz, r0.xxxx, r4.xyzx
    r6.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 221: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 222: div r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)/(r4.xyzx)).xyz;
    // 223: dp3 r0.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 224: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 225: mad r0.yzw, r2.xxyz, r15.xxzw, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r15.xxzw)+(r0.yyzw)).yzw;
    // 226: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 227: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 228: dp3 r1.x, r1.xyzx, r14.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 229: add r1.y, -|r14.z|, l(1.000000)
    r1.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 230: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 232: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 233: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 234: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 235: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 236: mul r1.xzw, r1.xxxx, cb0[3].xxyz
    r1.xzw = ((r1.xxxx)*(source[3].xxyz)).xzw;
    // 237: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 238: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 239: add r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 240: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 241: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 242: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 243: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 244: mul r1.xyz, r1.xxxx, r9.xyzx
    r1.xyz = ((r1.xxxx)*(r9.xyzx)).xyz;
    // 245: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 246: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 247: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 248: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 249: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 250: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 251: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 252: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 253: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 254: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 255: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 256: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 257: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 258: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 259: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 260: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 261: ret
    return output;
}

// source.character.static-map-native-221.v1 / source program 7af4912b3f3201498ff32236c2388f23
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase221(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6].x=g_SourceCharacterBaseConstants[6].x;
    source[6].y=g_SourceCharacterBaseConstants[6].y;
    source[6].z=g_SourceCharacterBaseConstants[6].z;
    source[6].w=g_SourceCharacterBaseConstants[6].w;
    source[7].x=g_SourceCharacterBaseConstants[7].x;
    source[7].y=g_SourceCharacterBaseConstants[7].y;
    source[7].z=g_SourceCharacterBaseConstants[7].z;
    source[7].w=g_SourceCharacterBaseConstants[7].w;
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[9].x=g_SourceCharacterBaseConstants[9].x;
    source[9].y=g_SourceCharacterBaseConstants[9].y;
    source[9].z=g_SourceCharacterBaseConstants[9].z;
    source[9].w=g_SourceCharacterBaseConstants[9].w;
    source[10].x=g_SourceCharacterBaseConstants[10].x;
    source[10].y=g_SourceCharacterBaseConstants[10].y;
    source[10].z=g_SourceCharacterBaseConstants[10].z;
    source[10].w=g_SourceCharacterBaseConstants[10].w;
    source[11].x=g_SourceCharacterBaseConstants[11].x;
    source[11].y=g_SourceCharacterBaseConstants[11].y;
    source[11].z=g_SourceCharacterBaseConstants[11].z;
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0, r15=0.0;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[7].xxxx
    r0.xy = ((source[0].xyxx)*(source[7].xxxx)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, v4.xyxx, cb0[6].yyyy
    r1.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r1.xy, r1.xyxx, cb0[6].zzzz
    r1.xy = ((r1.xyxx)*(source[6].zzzz)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 9: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 11: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 24: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 27: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 28: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 29: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 30: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 31: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 36: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[8].xxxx
    r0.zw = ((v4.xxxy)*(source[8].xxxx)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 48: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((((r0.zzzz) != 0.f ? 1.f : 0.f) / ((r0.zzzz) != 0.f ? (r0.zzzz) : 1.f))).z;
    // 51: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.yzw, cb0[5].xxyz, cb0[8].zzzz
    r0.yzw = ((source[5].xxyz)*(source[8].zzzz)).yzw;
    // 55: mul r7.xyz, r6.xyzx, r0.yzwy
    r7.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 56: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: mad r0.yzw, -r0.yyzw, r6.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r6.xxyz)+(r1.wwww)).yzw;
    // 58: mad r0.yzw, cb0[9].xxxx, r0.yyzw, r7.xxyz
    r0.yzw = ((source[9].xxxx)*(r0.yyzw)+(r7.xxyz)).yzw;
    // 59: mul r6.xyz, cb0[4].xyzx, cb0[8].yyyy
    r6.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 60: mad r0.yzw, -r3.xxyz, r6.xxyz, r0.yyzw
    r0.yzw = ((-(r3.xxyz))*(r6.xxyz)+(r0.yyzw)).yzw;
    // 61: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 62: mad r0.yzw, r0.xxxx, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 63: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 64: mul r3.xyz, r0.yzwy, cb0[9].yyyy
    r3.xyz = ((r0.yzwy)*(source[9].yyyy)).xyz;
    // 65: mad r0.yzw, cb0[9].zzzz, r0.yyzw, -r3.xxyz
    r0.yzw = ((source[9].zzzz)*(r0.yyzw)+(-(r3.xxyz))).yzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r1.w, r6.z, cb0[9].w
    r1.w = ((r6.zzzz)*(source[9].wwww)).w;
    // 68: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 69: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 70: mul r2.w, r2.w, cb0[10].x
    r2.w = ((r2.wwww)*(source[10].xxxx)).w;
    // 71: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 72: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 73: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 75: mad r0.yzw, r2.wwww, r0.yyzw, r3.xxyz
    r0.yzw = ((r2.wwww)*(r0.yyzw)+(r3.xxyz)).yzw;
    // 76: add r3.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 77: mul r0.yzw, r0.yyzw, r3.xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)).yzw;
    // 78: mad_sat r3.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 79: mad r0.yzw, r3.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r3.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 80: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 81: mad r8.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 82: mul r1.w, r6.x, cb0[11].y
    r1.w = ((r6.xxxx)*(source[11].yyyy)).w;
    // 83: mul r2.w, r6.y, cb0[10].w
    r2.w = ((r6.yyyy)*(source[10].wwww)).w;
    // 84: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 85: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: mul r4.w, r4.w, cb0[11].z
    r4.w = ((r4.wwww)*(source[11].zzzz)).w;
    // 87: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 88: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 89: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 90: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 91: mad r0.yzw, r7.xxyz, r1.wwww, r0.yyzw
    r0.yzw = ((r7.xxyz)*(r1.wwww)+(r0.yyzw)).yzw;
    // 92: mul r0.yzw, r1.wwww, r0.yyzw
    r0.yzw = ((r1.wwww)*(r0.yyzw)).yzw;
    // 93: max r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = (max(r0.yyzw,r1.wwww)).yzw;
    // 94: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 95: mad r1.xyz, r0.xxxx, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 96: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 97: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 98: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 99: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r8.xyz, r0.xxxx, v6.xyzx
    r8.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 102: dp3 r0.x, r8.xyzx, r7.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 103: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 104: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 105: mul r9.xyz, r6.yyyy, cb0[23].xyzx
    r9.xyz = ((r6.yyyy)*(source[23].xyzx)).xyz;
    // 106: mad r9.xyz, r6.xxxx, cb0[22].xyzx, r9.xyzx
    r9.xyz = ((r6.xxxx)*(source[22].xyzx)+(r9.xyzx)).xyz;
    // 107: mul r9.xyz, r9.xyzx, cb0[24].wwww
    r9.xyz = ((r9.xyzx)*(source[24].wwww)).xyz;
    // 108: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 109: mul r0.xyz, r0.yzwy, r9.xyzx
    r0.xyz = ((r0.yzwy)*(r9.xyzx)).xyz;
    // 110: dp3 r9.x, r2.xyzx, r7.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 111: dp3 r9.y, r5.xyzx, r7.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 112: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 113: dp3 r10.y, r4.xyzx, r7.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 114: mul r6.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 115: dp2 r10.x, r9.xyxx, r6.xyxx
    r10.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 116: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 117: dp4 r11.x, cb0[14].xyzw, r10.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 118: dp4 r11.y, cb0[15].xyzw, r10.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 119: dp4 r11.z, cb0[16].xyzw, r10.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 120: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 121: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 122: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 123: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 124: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 125: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 126: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 127: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 128: mad r10.xyz, cb0[20].xyzx, r0.wwww, r11.xyzx
    r10.xyz = ((source[20].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 129: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((r10.xyzx)*(source[12].xyzx)).xyz;
    // 131: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 132: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 133: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 134: mul r0.w, r3.w, l(0.080000)
    r0.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 135: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 136: mad r11.xyz, r6.wwww, r11.xyzx, r0.wwww
    r11.xyz = ((r6.wwww)*(r11.xyzx)+(r0.wwww)).xyz;
    // 137: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 138: log r3.w, |r2.w|
    r3.w = (log2(abs(r2.wwww))).w;
    // 139: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 140: mul r3.w, r3.w, cb0[11].x
    r3.w = ((r3.wwww)*(source[11].xxxx)).w;
    // 141: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 142: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 143: max r2.w, r2.w, cb0[1].x
    r2.w = (max(r2.wwww,source[1].xxxx)).w;
    // 144: min r6.z, r2.w, l(1.000000)
    r6.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 145: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 146: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 147: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 148: dp3 r2.w, r7.xyzx, r12.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 149: mul r7.xyz, r2.wwww, r7.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 150: mad r7.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r7.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 151: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 152: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 153: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 155: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 156: mad_sat r13.y, r3.w, l(0.300000), r6.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 157: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 158: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 160: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 161: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 162: add r0.w, r7.z, l(1.000000)
    r0.w = ((r7.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: add_sat r13.x, -r0.w, r2.w
    r13.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 165: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 166: add r0.w, r1.w, r13.x
    r0.w = ((r1.wwww)+(r13.xxxx)).w;
    // 167: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 168: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 169: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 170: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = ((((r13.wwww) != 0.f ? 1.f : 0.f) / ((r13.wwww) != 0.f ? (r13.wwww) : 1.f))).w;
    // 171: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 172: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 173: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 174: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 175: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 177: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 178: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 179: mad r0.xyz, -r0.xyzx, r6.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r6.wwww)+(r0.xyzx)).xyz;
    // 180: dp3 r2.x, r2.xyzx, r7.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 181: dp3 r2.y, r5.xyzx, r7.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 182: dp2 r5.x, r2.xyxx, r6.xyxx
    r5.x = (dot((r2.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 183: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 184: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 185: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 186: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 187: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 188: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 189: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 190: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 191: dp3 r5.y, r4.xyzx, r7.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 192: dp3 r1.w, r8.xyzx, r7.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 193: mad r2.yz, r1.wwww, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r1.wwww)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 194: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 195: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r5.xyzx, t6.xyzw, s5, r2.x
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 196: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 197: mul r4.xyz, r4.xyzx, cb0[12].xyzx
    r4.xyz = ((r4.xyzx)*(source[12].xyzx)).xyz;
    // 198: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 199: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 200: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 201: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 202: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 203: mul r2.xzw, r2.zzzz, cb0[23].xxyz
    r2.xzw = ((r2.zzzz)*(source[23].xxyz)).xzw;
    // 204: mad r2.xyz, cb0[22].xyzx, r2.yyyy, r2.xzwx
    r2.xyz = ((source[22].xyzx)*(r2.yyyy)+(r2.xzwx)).xyz;
    // 205: mul r2.xyz, r2.xyzx, cb0[24].wwww
    r2.xyz = ((r2.xyzx)*(source[24].wwww)).xyz;
    // 206: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 207: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 208: mad r0.xyz, r2.xyzx, r13.xzwx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r13.xzwx)+(r0.xyzx)).xyz;
    // 209: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 210: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 211: dp3 r0.w, r1.xyzx, r12.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 212: add r1.x, -|r12.z|, l(1.000000)
    r1.x = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 215: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 216: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 217: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 218: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 219: mul r1.yzw, r0.wwww, cb0[3].xxyz
    r1.yzw = ((r0.wwww)*(source[3].xxyz)).yzw;
    // 220: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 221: add r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)+(source[2].xyzx)).xyz;
    // 222: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 223: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 224: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r1.xyzx)).xyz;
    // 225: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 226: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 227: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 228: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 229: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 230: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 231: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 232: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 233: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 234: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 235: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 236: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 237: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 238: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 239: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 240: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 241: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 242: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 243: ret
    return output;
}

// source.character.static-map-native-222.v1 / source program d5a076b514f7cf4589b7c06889c277de
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked222(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[7]=1.f; source[8]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r3.xyz, cb0[2].xyzx, cb0[3].zzzz
    r3.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 28: mad r4.xyz, r1.zzzz, r3.xyzx, -r1.yyyy
    r4.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r5.xyz, r3.xyzx, r1.zzzz
    r5.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 30: mad r4.xyz, r5.xyzx, r4.xyzx, r1.yyyy
    r4.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.yyyy)).xyz;
    // 31: mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // 32: mad r6.xyz, r1.zzzz, r3.xyzx, -r1.xxxx
    r6.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 33: mad r6.xyz, r5.xyzx, r6.xyzx, r1.xxxx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)+(r1.xxxx)).xyz;
    // 34: mad r4.xyz, r6.xyzx, cb0[4].xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(source[4].xyzx)+(r4.xyzx)).xyz;
    // 35: mul r4.xyz, r4.xyzx, cb0[6].wwww
    r4.xyz = ((r4.xyzx)*(source[6].wwww)).xyz;
    // 36: mad r1.xyz, -r1.zzzz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r1.zzzz))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: mad r3.xyz, cb0[3].yyyy, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = ((source[3].yyyy)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 38: mad r3.xyz, r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 40: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 41: mul r3.xyz, r0.xyzx, r4.xyzx
    r3.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 42: dp2_sat r6.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 43: dp3_sat r6.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 44: dp3_sat r6.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 45: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 46: mad r1.xyz, r6.xyzx, r1.xyzx, r5.xyzx
    r1.xyz = ((r6.xyzx)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 47: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 48: mul r5.xyz, r5.xyzx, cb0[8].xyzx
    r5.xyz = ((r5.xyzx)*(source[8].xyzx)).xyz;
    // 49: dp3 r0.w, r5.xyzx, r1.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 50: sample_indexable(texture2d)(float,float,float,float) r1.xyz, v3.zwzz, t3.xyzw, s3
    r1.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 51: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 52: mul r6.xyz, r0.wwww, r1.xyzx
    r6.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 53: mad r4.xyz, r1.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r1.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 54: mul r1.xyz, r1.xyzx, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].xyzx)).xyz;
    // 55: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 56: div r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)/(r4.xyzx)).xyz;
    // 57: mad r3.xyz, r0.xyzx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 58: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 59: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 60: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 61: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 62: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: mul r6.xyz, r1.wwww, r2.xyzx
    r6.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 64: mad r4.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 65: dp2_sat r6.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 66: dp3_sat r6.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 67: dp3_sat r6.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 68: mul r4.xyz, r6.xyzx, r6.xyzx
    r4.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 69: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 70: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 71: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 72: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 73: mad r3.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 74: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 75: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 76: add r1.xyz, r3.xyzx, cb0[0].xyzx
    r1.xyz = ((r3.xyzx)+(source[0].xyzx)).xyz;
    // 77: mad o0.xyz, r0.xyzx, cb0[6].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)+(r1.xyzx)).xyz;
    // 78: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 79: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 80: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 81: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 82: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 83: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 84: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 85: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 86: mul r4.xyz, r0.zxyz, r1.yzxy
    r4.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 87: mad r4.xyz, r0.yzxy, r1.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r4.xyzx))).xyz;
    // 88: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 89: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 90: mul r1.xyz, r4.xyzx, v1.wwww
    r1.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 91: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 92: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 93: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 94: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 95: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 96: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 97: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 98: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 99: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 100: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 101: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 102: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 103: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 104: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 106: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 107: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 108: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 109: ret
    return output;
}

// source.character.static-map-native-222.v1 / source program d50c5137bd483c4ebdaee519b77e5d44
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase222(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3].x=g_SourceCharacterBaseConstants[3].x;
    source[3].y=g_SourceCharacterBaseConstants[3].y;
    source[3].z=g_SourceCharacterBaseConstants[3].z;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[3].xxxx
    r2.xy = ((r2.xyxx)*(source[3].xxxx)).xy;
    // 13: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 23: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 24: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 26: mul r3.xyz, cb0[2].xyzx, cb0[3].zzzz
    r3.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 28: mad r4.xyz, r1.zzzz, r3.xyzx, -r1.yyyy
    r4.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r5.xyz, r3.xyzx, r1.zzzz
    r5.xyz = ((r3.xyzx)*(r1.zzzz)).xyz;
    // 30: mad r3.xyz, r1.zzzz, r3.xyzx, -r1.xxxx
    r3.xyz = ((r1.zzzz)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 31: mad r3.xyz, r5.xyzx, r3.xyzx, r1.xxxx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r1.xxxx)).xyz;
    // 32: mad r1.xyz, r5.xyzx, r4.xyzx, r1.yyyy
    r1.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.yyyy)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[5].xyzx
    r1.xyz = ((r1.xyzx)*(source[5].xyzx)).xyz;
    // 34: mad r1.xyz, r3.xyzx, cb0[4].xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(source[4].xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[6].wwww
    r1.xyz = ((r1.xyzx)*(source[6].wwww)).xyz;
    // 36: mad r3.xyz, cb0[3].yyyy, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = ((source[3].yyyy)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 37: mad r3.xyz, r1.wwww, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 38: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 39: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 40: mad r3.xyz, r1.xyzx, r0.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(source[0].xyzx)).xyz;
    // 41: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 42: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 43: mad o0.xyz, r0.xyzx, cb0[6].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[6].xyzx)+(r3.xyzx)).xyz;
    // 44: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 45: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 46: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 47: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 48: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 49: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 50: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 51: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 52: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 53: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 54: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 55: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 56: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 57: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 58: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 59: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 60: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 61: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 62: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 63: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 64: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 65: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 66: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 67: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 68: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 69: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 70: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 71: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 72: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 73: ret
    return output;
}

// source.character.static-map-native-223.v1 / source program 1bb5c3651fd7f04e961445cd3f48181b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked223(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[9]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[10]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[13]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[15].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))).x;
    source[20]=1.f; source[21]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: dp3 r0.x, cb0[7].xyzx, cb0[7].xyzx
    r0.x = (dot((source[7].xyzx).xyz,(source[7].xyzx).xyz).xxxx).x;
    // 2: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 3: div r0.xyz, cb0[7].xyzx, r0.xxxx
    r0.xyz = ((source[7].xyzx)/(r0.xxxx)).xyz;
    // 4: add r1.xyz, v7.xyzx, cb0[0].xyzx
    r1.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 5: add r2.xyz, -r1.xyzx, cb0[0].xyzx
    r2.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 6: add r1.xyz, r1.xyzx, -cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[1].xyzx))).xyz;
    // 7: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 10: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 11: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 12: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 13: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 14: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 15: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 16: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 17: mul r0.yz, v4.xxyx, cb0[8].xxyx
    r0.yz = ((v4.xxyx)*(source[8].xxyx)).yz;
    // 18: mad r0.yz, r0.yyzy, l(0.000000, -1.000000, 2.000000, 0.000000), cb0[10].xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,-1.000000,2.000000,0.000000))+(source[10].xxyx)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t2.zxyw, s2, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 21: mad r3.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mov r4.xw, l(1.000000,0,0,2.000000)
    r4.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 23: mov r4.yz, cb0[8].yyxy
    r4.yz = (source[8].yyxy).yz;
    // 24: mul r0.yz, r4.xxyx, v4.xxyx
    r0.yz = ((r4.xxyx)*(v4.xxyx)).yz;
    // 25: mad r4.xy, r4.zwzz, r0.yzyy, cb0[9].xyxx
    r4.xy = ((r4.zwzz)*(r0.yzyy)+(source[9].xyxx)).xy;
    // 26: mad r0.yz, r4.zzwz, r0.yyzy, cb0[13].xxyx
    r0.yz = ((r4.zzwz)*(r0.yyzy)+(source[13].xxyx)).yz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 30: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 31: mad r2.yzw, r5.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r2.xxyz
    r2.yzw = ((r5.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r2.xxyz)).yzw;
    // 32: mul r2.yzw, r2.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r2.yzw = ((r2.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 33: mad_sat r0.yzw, r0.yyzw, r2.yyzw, r2.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r2.yyzw)+(r2.yyzw))).yzw;
    // 34: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 40: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 43: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: add r1.w, r3.z, -r4.z
    r1.w = ((r3.zzzz)+(-(r4.zzzz))).w;
    // 45: add r2.yzw, r3.xxyz, r4.xxyz
    r2.yzw = ((r3.xxyz)+(r4.xxyz)).yzw;
    // 46: mad r1.w, r2.x, r1.w, r4.z
    r1.w = ((r2.xxxx)*(r1.wwww)+(r4.zzzz)).w;
    // 47: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 49: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 50: mul r3.xyz, r0.yzwy, r1.wwww
    r3.xyz = ((r0.yzwy)*(r1.wwww)).xyz;
    // 51: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 52: add r0.z, cb0[12].w, l(-0.030000)
    r0.z = ((source[12].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 53: mad r3.xyz, r3.xyzx, r0.zzzz, l(0.030000, 0.030000, 0.030000, 0.000000)
    r3.xyz = ((r3.xyzx)*(r0.zzzz)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 54: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 55: dp3 r0.z, r2.yzwy, r2.yzwy
    r0.z = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).z;
    // 56: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 57: div r2.xyz, r2.yzwy, r0.zzzz
    r2.xyz = ((r2.yzwy)/(r0.zzzz)).xyz;
    // 58: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 59: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 60: mul r4.xyz, r0.zzzz, v5.xyzx
    r4.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 61: dp3 r0.z, r4.xyzx, r2.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 62: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mad r0.z, -r0.z, l(0.500000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 64: mul r2.xyz, r0.zzzz, cb0[11].xyzx
    r2.xyz = ((r0.zzzz)*(source[11].xyzx)).xyz;
    // 65: mul r2.xyz, r2.xyzx, cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(source[11].wwww)).xyz;
    // 66: mad r0.xzw, r0.xxxx, r2.xxyz, r3.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)+(r3.xxyz)).xzw;
    // 67: mul r1.w, cb0[2].w, cb0[14].x
    r1.w = ((source[2].wwww)*(source[14].xxxx)).w;
    // 68: div r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)/(r1.zzzz)).z;
    // 69: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: mad r1.zw, |r1.zzzz|, r1.xxxy, -r1.xxxy
    r1.zw = ((abs(r1.zzzz))*(r1.xxxy)+(-(r1.xxxy))).zw;
    // 71: mad r1.xy, cb0[14].yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((source[14].yyyy)*(r1.zwzz)+(r1.xyxx)).xy;
    // 72: div r1.xy, r1.xyxx, cb0[2].wwww
    r1.xy = ((r1.xyxx)/(source[2].wwww)).xy;
    // 73: mul r1.zw, cb0[5].xxxy, cb0[14].zzzz
    r1.zw = ((source[5].xxxy)*(source[14].zzzz)).zw;
    // 74: mul r1.zw, r1.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 75: mad r1.zw, r1.xxxy, l(0.000000, 0.000000, -0.750000, -0.750000), r1.zzzw
    r1.zw = ((r1.xxxy)*(float4(0.000000,0.000000,-0.750000,-0.750000))+(r1.zzzw)).zw;
    // 76: mad r1.xy, cb0[14].zzzz, cb0[5].xyxx, r1.xyxx
    r1.xy = ((source[14].zzzz)*(source[5].xyxx)+(r1.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 79: add r3.xyzw, -r1.xyzw, r2.xyzw
    r3.xyzw = ((-(r1.xyzw))+(r2.xyzw)).xyzw;
    // 80: mad r1.xyzw, r2.wwww, r3.xyzw, r1.xyzw
    r1.xyzw = ((r2.wwww)*(r3.xyzw)+(r1.xyzw)).xyzw;
    // 81: add r2.x, v4.y, l(-0.500000)
    r2.x = ((v4.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 82: add_sat r2.x, r2.x, r2.x
    r2.x = (saturate((r2.xxxx)+(r2.xxxx))).x;
    // 83: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 84: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 85: mul r2.y, r2.y, cb0[14].w
    r2.y = ((r2.yyyy)*(source[14].wwww)).y;
    // 86: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 87: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 88: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 90: mul r3.xyz, r2.xyzx, cb0[4].xyzx
    r3.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 91: mad r2.xyz, -r2.xyzx, cb0[4].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[4].xyzx)+(r2.xyzx)).xyz;
    // 92: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 93: add r2.w, -r2.w, l(0.200000)
    r2.w = ((-(r2.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 94: mul_sat r2.w, r2.w, l(5.000000)
    r2.w = (saturate((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000)))).w;
    // 95: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 96: mad r1.xyz, r1.xyzx, cb0[6].xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(source[6].xyzx)+(-(r2.xyzx))).xyz;
    // 97: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 98: add r0.xzw, r0.xxzw, -r1.xxyz
    r0.xzw = ((r0.xxzw)+(-(r1.xxyz))).xzw;
    // 99: mad r0.xyz, r0.yyyy, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r1.xyzx)).xyz;
    // 100: mul r0.xyz, r0.xyzx, cb0[16].yyyy
    r0.xyz = ((r0.xyzx)*(source[16].yyyy)).xyz;
    // 101: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 102: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 103: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 104: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 105: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 106: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 107: mul r1.yzw, r1.yyyy, cb0[18].xxyz
    r1.yzw = ((r1.yyyy)*(source[18].xxyz)).yzw;
    // 108: mad r1.xyz, r1.xxxx, cb0[17].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[17].xyzx)+(r1.yzwy)).xyz;
    // 109: mul r1.xyz, r1.xyzx, cb0[19].wwww
    r1.xyz = ((r1.xyzx)*(source[19].wwww)).xyz;
    // 110: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 111: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t5.xyzw, s4
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 112: mul r3.xyz, r3.xyzx, cb0[21].xyzx
    r3.xyz = ((r3.xyzx)*(source[21].xyzx)).xyz;
    // 113: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 114: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s4
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 115: mul r3.xyz, r3.xyzx, cb0[20].xyzx
    r3.xyz = ((r3.xyzx)*(source[20].xyzx)).xyz;
    // 116: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 117: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 118: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 119: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 120: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 121: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 123: add r1.xyz, r2.xyzx, cb0[3].xyzx
    r1.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 124: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 125: mad o0.xyz, r0.xyzx, cb0[19].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)+(r1.xyzx)).xyz;
    // 126: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 127: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 128: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 129: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 130: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 131: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 132: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 133: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 134: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 135: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 136: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 137: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 138: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 139: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 140: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 141: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 142: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 143: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 144: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 145: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 146: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 147: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 148: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 149: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 150: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 151: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 152: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 153: ret
    return output;
}

// source.character.static-map-native-223.v1 / source program 916bf6bbae81c24faa36eda8f4b7663b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase223(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=g_SourceCharacterBaseConstants[8];
    source[9]=g_SourceCharacterBaseConstants[9];
    source[10]=g_SourceCharacterBaseConstants[10];
    source[11]=g_SourceCharacterBaseConstants[11];
    source[12]=g_SourceCharacterBaseConstants[12];
    source[13]=g_SourceCharacterBaseConstants[13];
    source[14].x=g_SourceCharacterBaseConstants[14].x;
    source[14].y=g_SourceCharacterBaseConstants[14].y;
    source[14].z=g_SourceCharacterBaseConstants[14].z;
    source[14].w=g_SourceCharacterBaseConstants[14].w;
    source[15].x=g_SourceCharacterBaseConstants[15].x;
    source[15].y=g_SourceCharacterBaseConstants[15].y;
    source[15].z=g_SourceCharacterBaseConstants[15].z;
    source[15].w=g_SourceCharacterBaseConstants[15].w;
    source[16].x=g_SourceCharacterBaseConstants[16].x;
    source[16].y=g_SourceCharacterBaseConstants[16].y;
    source[9]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[10]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[13]=SourceCharacterAppend(frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))),frac(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))),1u);
    source[14].z=(g_SourceCharacterTime.xxxx).x;
    source[15].z=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15].w=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: dp3 r0.x, cb0[7].xyzx, cb0[7].xyzx
    r0.x = (dot((source[7].xyzx).xyz,(source[7].xyzx).xyz).xxxx).x;
    // 2: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 3: div r0.xyz, cb0[7].xyzx, r0.xxxx
    r0.xyz = ((source[7].xyzx)/(r0.xxxx)).xyz;
    // 4: add r1.xyz, v7.xyzx, cb0[0].xyzx
    r1.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 5: add r2.xyz, -r1.xyzx, cb0[0].xyzx
    r2.xyz = ((-(r1.xyzx))+(source[0].xyzx)).xyz;
    // 6: add r1.xyz, r1.xyzx, -cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[1].xyzx))).xyz;
    // 7: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 10: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 11: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 12: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 13: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 14: mul r0.y, r0.y, cb0[15].x
    r0.y = ((r0.yyyy)*(source[15].xxxx)).y;
    // 15: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 16: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 17: mul r0.yz, v4.xxyx, cb0[8].xxyx
    r0.yz = ((v4.xxyx)*(source[8].xxyx)).yz;
    // 18: mad r0.yz, r0.yyzy, l(0.000000, -1.000000, 2.000000, 0.000000), cb0[10].xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,-1.000000,2.000000,0.000000))+(source[10].xxyx)).yz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t2.zxyw, s2, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 21: mad r3.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mov r4.xw, l(1.000000,0,0,2.000000)
    r4.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 23: mov r4.yz, cb0[8].yyxy
    r4.yz = (source[8].yyxy).yz;
    // 24: mul r0.yz, r4.xxyx, v4.xxyx
    r0.yz = ((r4.xxyx)*(v4.xxyx)).yz;
    // 25: mad r4.xy, r4.zwzz, r0.yzyy, cb0[9].xyxx
    r4.xy = ((r4.zwzz)*(r0.yzyy)+(source[9].xyxx)).xy;
    // 26: mad r0.yz, r4.zzwz, r0.yyzy, cb0[13].xxyx
    r0.yz = ((r4.zzwz)*(r0.yyzy)+(source[13].xxyx)).yz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t3.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 30: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 31: mad r2.yzw, r5.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r2.xxyz
    r2.yzw = ((r5.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r2.xxyz)).yzw;
    // 32: mul r2.yzw, r2.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r2.yzw = ((r2.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 33: mad_sat r0.yzw, r0.yyzw, r2.yyzw, r2.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r2.yyzw)+(r2.yyzw))).yzw;
    // 34: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 40: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 43: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 44: add r1.w, r3.z, -r4.z
    r1.w = ((r3.zzzz)+(-(r4.zzzz))).w;
    // 45: add r2.yzw, r3.xxyz, r4.xxyz
    r2.yzw = ((r3.xxyz)+(r4.xxyz)).yzw;
    // 46: mad r1.w, r2.x, r1.w, r4.z
    r1.w = ((r2.xxxx)*(r1.wwww)+(r4.zzzz)).w;
    // 47: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 49: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 50: mul r3.xyz, r0.yzwy, r1.wwww
    r3.xyz = ((r0.yzwy)*(r1.wwww)).xyz;
    // 51: mul r0.y, r0.y, cb0[16].x
    r0.y = ((r0.yyyy)*(source[16].xxxx)).y;
    // 52: add r0.z, cb0[12].w, l(-0.030000)
    r0.z = ((source[12].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 53: mad r3.xyz, r3.xyzx, r0.zzzz, l(0.030000, 0.030000, 0.030000, 0.000000)
    r3.xyz = ((r3.xyzx)*(r0.zzzz)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 54: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 55: dp3 r0.z, r2.yzwy, r2.yzwy
    r0.z = (dot((r2.yzwy).xyz,(r2.yzwy).xyz).xxxx).z;
    // 56: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 57: div r2.xyz, r2.yzwy, r0.zzzz
    r2.xyz = ((r2.yzwy)/(r0.zzzz)).xyz;
    // 58: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 59: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 60: mul r4.xyz, r0.zzzz, v5.xyzx
    r4.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 61: dp3 r0.z, r4.xyzx, r2.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 62: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mad r0.z, -r0.z, l(0.500000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 64: mul r2.xyz, r0.zzzz, cb0[11].xyzx
    r2.xyz = ((r0.zzzz)*(source[11].xyzx)).xyz;
    // 65: mul r2.xyz, r2.xyzx, cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(source[11].wwww)).xyz;
    // 66: mad r0.xzw, r0.xxxx, r2.xxyz, r3.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)+(r3.xxyz)).xzw;
    // 67: mul r1.w, cb0[2].w, cb0[14].x
    r1.w = ((source[2].wwww)*(source[14].xxxx)).w;
    // 68: div r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)/(r1.zzzz)).z;
    // 69: add r1.z, r1.z, l(1.000000)
    r1.z = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: mad r1.zw, |r1.zzzz|, r1.xxxy, -r1.xxxy
    r1.zw = ((abs(r1.zzzz))*(r1.xxxy)+(-(r1.xxxy))).zw;
    // 71: mad r1.xy, cb0[14].yyyy, r1.zwzz, r1.xyxx
    r1.xy = ((source[14].yyyy)*(r1.zwzz)+(r1.xyxx)).xy;
    // 72: div r1.xy, r1.xyxx, cb0[2].wwww
    r1.xy = ((r1.xyxx)/(source[2].wwww)).xy;
    // 73: mul r1.zw, cb0[5].xxxy, cb0[14].zzzz
    r1.zw = ((source[5].xxxy)*(source[14].zzzz)).zw;
    // 74: mul r1.zw, r1.zzzw, l(0.000000, 0.000000, -0.500000, -0.500000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 75: mad r1.zw, r1.xxxy, l(0.000000, 0.000000, -0.750000, -0.750000), r1.zzzw
    r1.zw = ((r1.xxxy)*(float4(0.000000,0.000000,-0.750000,-0.750000))+(r1.zzzw)).zw;
    // 76: mad r1.xy, cb0[14].zzzz, cb0[5].xyxx, r1.xyxx
    r1.xy = ((source[14].zzzz)*(source[5].xyxx)+(r1.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r1.zwzz, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 79: add r3.xyzw, -r1.xyzw, r2.xyzw
    r3.xyzw = ((-(r1.xyzw))+(r2.xyzw)).xyzw;
    // 80: mad r1.xyzw, r2.wwww, r3.xyzw, r1.xyzw
    r1.xyzw = ((r2.wwww)*(r3.xyzw)+(r1.xyzw)).xyzw;
    // 81: add r2.x, v4.y, l(-0.500000)
    r2.x = ((v4.yyyy)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).x;
    // 82: add_sat r2.x, r2.x, r2.x
    r2.x = (saturate((r2.xxxx)+(r2.xxxx))).x;
    // 83: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 84: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 85: mul r2.y, r2.y, cb0[14].w
    r2.y = ((r2.yyyy)*(source[14].wwww)).y;
    // 86: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 87: movc r2.x, r2.x, l(0), r2.y
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).x;
    // 88: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 90: mul r3.xyz, r2.xyzx, cb0[4].xyzx
    r3.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 91: mad r2.xyz, -r2.xyzx, cb0[4].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[4].xyzx)+(r2.xyzx)).xyz;
    // 92: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 93: add r2.w, -r2.w, l(0.200000)
    r2.w = ((-(r2.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 94: mul_sat r2.w, r2.w, l(5.000000)
    r2.w = (saturate((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000)))).w;
    // 95: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 96: mad r1.xyz, r1.xyzx, cb0[6].xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(source[6].xyzx)+(-(r2.xyzx))).xyz;
    // 97: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 98: add r0.xzw, r0.xxzw, -r1.xxyz
    r0.xzw = ((r0.xxzw)+(-(r1.xxyz))).xzw;
    // 99: mad r0.xyz, r0.yyyy, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r1.xyzx)).xyz;
    // 100: mul r0.xyz, r0.xyzx, cb0[16].yyyy
    r0.xyz = ((r0.xyzx)*(source[16].yyyy)).xyz;
    // 101: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 102: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 103: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 104: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 105: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 106: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 107: mul r1.yzw, r1.yyyy, cb0[18].xxyz
    r1.yzw = ((r1.yyyy)*(source[18].xxyz)).yzw;
    // 108: mad r1.xyz, r1.xxxx, cb0[17].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[17].xyzx)+(r1.yzwy)).xyz;
    // 109: mul r1.xyz, r1.xyzx, cb0[19].wwww
    r1.xyz = ((r1.xyzx)*(source[19].wwww)).xyz;
    // 110: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[3].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[3].xyzx)).xyz;
    // 111: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 112: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 113: mad o0.xyz, r0.xyzx, cb0[19].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[19].xyzx)+(r2.xyzx)).xyz;
    // 114: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 115: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 116: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 117: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 118: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 119: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 120: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 121: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 122: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 123: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 124: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 125: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 126: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 127: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 128: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 129: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 130: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 131: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 132: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 133: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 134: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 135: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 136: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 137: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 138: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 139: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 140: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 141: ret
    return output;
}

// source.character.static-map-native-224.v1 / source program 87c053efe3f0dc4e924d49533f0f4c74
