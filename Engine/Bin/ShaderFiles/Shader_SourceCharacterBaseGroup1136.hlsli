#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1136(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    source[25]=1.f;
    source[26]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 6: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 7: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 15: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 18: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 19: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 22: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 23: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 24: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 25: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 27: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 28: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 30: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 33: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.zw, v4.xxxy, cb0[6].yyyy
    r0.zw = ((v4.xxxy)*(source[6].yyyy)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 37: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: mul r1.w, r6.w, r6.w
    r1.w = ((r6.wwww)*(r6.wwww)).w;
    // 39: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 40: max r1.w, cb0[6].w, l(0.000000)
    r1.w = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 43: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 44: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 45: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 47: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 48: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 49: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 50: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r7.xyz, -r3.xyzx, r0.yyyy
    r7.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 52: mad r3.xyz, cb0[8].xxxx, r7.xyzx, r3.xyzx
    r3.xyz = ((source[8].xxxx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 53: mul r7.xyz, cb0[5].xyzx, cb0[8].zzzz
    r7.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 54: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 55: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 57: mad r6.xyz, cb0[9].xxxx, r6.xyzx, r8.xyzx
    r6.xyz = ((source[9].xxxx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[4].xyzx, cb0[8].yyyy
    r7.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 59: mad r6.xyz, -r3.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 60: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 61: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mul r6.xyz, r3.xyzx, cb0[9].yyyy
    r6.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 64: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: mul r0.y, r7.z, cb0[9].w
    r0.y = ((r7.zzzz)*(source[9].wwww)).y;
    // 67: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 68: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 69: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 70: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 71: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 72: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 74: mad r3.xyz, r1.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 75: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 77: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 78: mad r6.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 79: mad r8.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 80: mad r9.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 81: mul r0.y, r7.x, cb0[11].y
    r0.y = ((r7.xxxx)*(source[11].yyyy)).y;
    // 82: mul r1.w, r7.y, cb0[10].w
    r1.w = ((r7.yyyy)*(source[10].wwww)).w;
    // 83: log r2.w, |r0.y|
    r2.w = (log2(abs(r0.yyyy))).w;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r2.w, r2.w, cb0[11].z
    r2.w = ((r2.wwww)*(source[11].zzzz)).w;
    // 86: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 87: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: movc r0.y, r0.y, l(0), r2.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 89: mad r8.xyz, r0.yyyy, r8.xyzx, r9.xyzx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 90: mad r6.xyz, r8.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 91: mul r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = ((r0.yyyy)*(r6.xyzx)).xyz;
    // 92: max r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = (max(r0.yyyy,r6.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r8.xy, r0.zwzz, cb0[6].zzzz
    r8.xy = ((r0.zwzz)*(source[6].zzzz)).xy;
    // 95: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 97: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 98: add r8.z, r0.z, l(0.000010)
    r8.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 99: add r8.xyz, -r1.xyzx, r8.xyzx
    r8.xyz = ((-(r1.xyzx))+(r8.xyzx)).xyz;
    // 100: mad r0.xzw, r0.xxxx, r8.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r8.xxyz)+(r1.xxyz)).xzw;
    // 101: dp3 r1.x, r0.xzwx, r0.xzwx
    r1.x = (dot((r0.xzwx).xyz,(r0.xzwx).xyz).xxxx).x;
    // 102: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 103: mul r1.xyz, r0.xzwx, r1.xxxx
    r1.xyz = ((r0.xzwx)*(r1.xxxx)).xyz;
    // 104: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 105: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 106: mul r8.xyz, r2.wwww, v6.xyzx
    r8.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 107: dp3 r2.w, r8.xyzx, r1.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 108: mad r7.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 110: mul r8.xyz, r7.yyyy, cb0[23].xyzx
    r8.xyz = ((r7.yyyy)*(source[23].xyzx)).xyz;
    // 111: mad r8.xyz, r7.xxxx, cb0[22].xyzx, r8.xyzx
    r8.xyz = ((r7.xxxx)*(source[22].xyzx)+(r8.xyzx)).xyz;
    // 112: mul r8.xyz, r8.xyzx, cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(source[24].wwww)).xyz;
    // 113: mul r9.xyz, r3.xyzx, r8.xyzx
    r9.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 114: dp2_sat r10.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 115: dp3_sat r10.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 116: dp3_sat r10.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 117: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 118: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 119: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 120: dp3 r2.w, r11.xyzx, r10.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 121: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[25].xyzx
    r10.xyz = ((r10.xyzx)*(source[25].xyzx)).xyz;
    // 123: mul r12.xyz, r2.wwww, r10.xyzx
    r12.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 124: mad r9.xyz, r3.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 125: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 126: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 127: dp3 r9.y, r5.xyzx, r1.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 128: dp2 r12.z, r9.xyxx, cb0[13].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 129: dp3 r12.y, r4.xyzx, r1.xyzx
    r12.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 130: mul r7.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r12.x, r9.xyxx, r7.xyxx
    r12.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 132: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 133: dp4 r13.x, cb0[14].xyzw, r12.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[15].xyzw, r12.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[16].xyzw, r12.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 137: dp4 r15.x, cb0[17].xyzw, r14.xyzw
    r15.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 138: dp4 r15.y, cb0[18].xyzw, r14.xyzw
    r15.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 139: dp4 r15.z, cb0[19].xyzw, r14.xyzw
    r15.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 140: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 141: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 142: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 143: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 144: mad r12.xyz, cb0[20].xyzx, r4.wwww, r13.xyzx
    r12.xyz = ((source[20].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 145: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 146: mul r12.xyz, r12.xyzx, cb0[12].xyzx
    r12.xyz = ((r12.xyzx)*(source[12].xyzx)).xyz;
    // 147: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 148: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 149: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 150: mul r4.w, r3.w, l(0.080000)
    r4.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 151: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 152: mad r13.xyz, r7.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 153: mul_sat r3.w, r13.y, l(50.000000)
    r3.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 155: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 156: mul r4.w, r4.w, cb0[11].x
    r4.w = ((r4.wwww)*(source[11].xxxx)).w;
    // 157: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 158: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 159: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 160: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 161: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 162: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 163: mul r14.xyz, r1.wwww, v5.xyzx
    r14.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 164: dp3 r1.w, r1.xyzx, r14.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 165: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 166: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 167: deriv_rtx_coarse r15.x, r1.w
    r15.x = (ddx_coarse(r1.wwww)).x;
    // 168: deriv_rty_coarse r15.y, r1.w
    r15.y = (ddy_coarse(r1.wwww)).y;
    // 169: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 171: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 172: mad_sat r15.y, r4.w, l(0.300000), r7.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 173: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 175: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 176: mul r16.xyz, r3.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r16.xyzx)).xyz;
    // 177: add r3.w, r1.z, l(1.000000)
    r3.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add_sat r15.x, r1.w, -r3.w
    r15.x = (saturate((r1.wwww)+(-(r3.wwww)))).x;
    // 180: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 181: add r1.w, r0.y, r15.x
    r1.w = ((r0.yyyy)+(r15.xxxx)).w;
    // 182: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 183: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 184: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 185: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 186: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 187: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 188: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 190: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 192: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 193: mul r6.xyz, r6.xyzx, r12.xyzx
    r6.xyz = ((r6.xyzx)*(r12.xyzx)).xyz;
    // 194: mad r6.xyz, -r6.xyzx, r7.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r7.wwww)+(r6.xyzx)).xyz;
    // 195: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 196: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 197: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 198: dp2 r5.x, r2.xyxx, r7.xyxx
    r5.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 199: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 200: mul r2.x, r15.y, l(5.000000)
    r2.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 201: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 202: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 203: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 204: add r1.w, r0.y, r1.w
    r1.w = ((r0.yyyy)+(r1.wwww)).w;
    // 205: mov o5.y, r0.y
    output.targets[5].y = (r0.yyyy).y;
    // 206: add_sat r0.y, r1.w, l(-1.000000)
    r0.y = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 207: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r5.xyzx, t6.xyzw, s6, r2.x
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r2.xyz, r4.xyzx, r4.wwww
    r2.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 211: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 212: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 213: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 214: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 215: mul r1.xyz, r4.xyzx, r4.xyzx
    r1.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 216: dp3 r1.x, r11.xyzx, r1.xyzx
    r1.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 217: add r1.y, -r1.x, r2.w
    r1.y = ((-(r1.xxxx))+(r2.wwww)).y;
    // 218: mad r1.x, r7.z, r1.y, r1.x
    r1.x = ((r7.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 219: mad r1.yzw, r10.xxyz, r1.xxxx, r8.xxyz
    r1.yzw = ((r10.xxyz)*(r1.xxxx)+(r8.xxyz)).yzw;
    // 220: mul r4.xyz, r1.xxxx, r10.xyzx
    r4.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 221: mad r1.x, r0.y, r13.x, r13.y
    r1.x = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).x;
    // 222: mad r1.x, r1.x, r0.y, r13.z
    r1.x = ((r1.xxxx)*(r0.yyyy)+(r13.zzzz)).x;
    // 223: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 224: max r0.y, r0.y, r1.x
    r0.y = (max(r0.yyyy,r1.xxxx)).y;
    // 225: mul r5.xyz, r0.yyyy, r1.yzwy
    r5.xyz = ((r0.yyyy)*(r1.yzwy)).xyz;
    // 226: add r1.xyz, r1.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 227: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 228: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 229: mul r1.xyz, r2.xyzx, r5.xyzx
    r1.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 230: mad r2.xyz, r1.xyzx, r15.xzwx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r6.xyzx)).xyz;
    // 231: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 232: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 233: dp3 r0.x, r0.xzwx, r14.xyzx
    r0.x = (dot((r0.xzwx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 234: add r0.z, -|r14.z|, l(1.000000)
    r0.z = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 235: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 236: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 237: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 238: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 239: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 240: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 241: mul r1.xyz, r0.xxxx, cb0[3].xyzx
    r1.xyz = ((r0.xxxx)*(source[3].xyzx)).xyz;
    // 242: movc r0.xzw, r0.zzzz, l(0,0,0,0), r1.xxyz
    r0.xzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyz)).xzw;
    // 243: add r0.xzw, r0.xxzw, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)+(source[1].xxyz)).xzw;
    // 244: add r0.xzw, r2.xxyz, r0.xxzw
    r0.xzw = ((r2.xxyz)+(r0.xxzw)).xzw;
    // 245: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r0.xzwx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r0.xzwx)).xyz;
    // 246: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 247: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 248: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 249: mul r0.xzw, r0.xxxx, r9.xxyz
    r0.xzw = ((r0.xxxx)*(r9.xxyz)).xzw;
    // 250: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 251: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 252: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 253: ge r1.yz, r0.xxzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxzx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 254: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 255: mad r1.yz, -|r0.zzxz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzxz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 256: movc r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.xxzx)).xz;
    // 257: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 258: mul o4.z, r0.y, r2.x
    output.targets[4].z = ((r0.yyyy)*(r2.xxxx)).z;
    // 259: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 260: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 261: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 262: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 263: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 264: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 265: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 266: ret
    return output;
}

// source.character.static-map-native-1136.v1 / source program bae1fdb7b968b448892353ec7ef6150d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1136(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1136(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 6: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 7: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 8: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 10: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 11: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 12: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 15: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 16: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 17: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 18: dp3 r3.x, r2.xyzx, r1.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 19: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r4.xyz, r0.wwww, v1.xyzx
    r4.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 22: dp3 r3.z, r4.xyzx, r1.xyzx
    r3.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 23: mul r5.xyz, r2.yzxy, r4.zxyz
    r5.xyz = ((r2.yzxy)*(r4.zxyz)).xyz;
    // 24: mad r5.xyz, r4.yzxy, r2.zxyz, -r5.xyzx
    r5.xyz = ((r4.yzxy)*(r2.zxyz)+(-(r5.xyzx))).xyz;
    // 25: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 27: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 28: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 29: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 30: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 33: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.zw, v4.xxxy, cb0[6].yyyy
    r0.zw = ((v4.xxxy)*(source[6].yyyy)).zw;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 37: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: mul r1.w, r6.w, r6.w
    r1.w = ((r6.wwww)*(r6.wwww)).w;
    // 39: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 40: max r1.w, cb0[6].w, l(0.000000)
    r1.w = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 43: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 44: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 45: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 47: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 48: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 49: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 50: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r7.xyz, -r3.xyzx, r0.yyyy
    r7.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 52: mad r3.xyz, cb0[8].xxxx, r7.xyzx, r3.xyzx
    r3.xyz = ((source[8].xxxx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 53: mul r7.xyz, cb0[5].xyzx, cb0[8].zzzz
    r7.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 54: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 55: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 57: mad r6.xyz, cb0[9].xxxx, r6.xyzx, r8.xyzx
    r6.xyz = ((source[9].xxxx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[4].xyzx, cb0[8].yyyy
    r7.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 59: mad r6.xyz, -r3.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 60: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 61: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mul r6.xyz, r3.xyzx, cb0[9].yyyy
    r6.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 64: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: mul r0.y, r7.z, cb0[9].w
    r0.y = ((r7.zzzz)*(source[9].wwww)).y;
    // 67: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 68: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 69: mul r1.w, r1.w, cb0[10].x
    r1.w = ((r1.wwww)*(source[10].xxxx)).w;
    // 70: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 71: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 72: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 74: mad r3.xyz, r1.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 75: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 77: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 78: mad r6.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 79: mad r8.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 80: mad r9.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 81: mul r0.y, r7.x, cb0[11].y
    r0.y = ((r7.xxxx)*(source[11].yyyy)).y;
    // 82: mul r1.w, r7.y, cb0[10].w
    r1.w = ((r7.yyyy)*(source[10].wwww)).w;
    // 83: log r2.w, |r0.y|
    r2.w = (log2(abs(r0.yyyy))).w;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r2.w, r2.w, cb0[11].z
    r2.w = ((r2.wwww)*(source[11].zzzz)).w;
    // 86: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 87: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: movc r0.y, r0.y, l(0), r2.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 89: mad r8.xyz, r0.yyyy, r8.xyzx, r9.xyzx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 90: mad r6.xyz, r8.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 91: mul r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = ((r0.yyyy)*(r6.xyzx)).xyz;
    // 92: max r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = (max(r0.yyyy,r6.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r8.xy, r0.zwzz, cb0[6].zzzz
    r8.xy = ((r0.zwzz)*(source[6].zzzz)).xy;
    // 95: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 97: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 98: add r8.z, r0.z, l(0.000010)
    r8.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 99: add r8.xyz, -r1.xyzx, r8.xyzx
    r8.xyz = ((-(r1.xyzx))+(r8.xyzx)).xyz;
    // 100: mad r0.xzw, r0.xxxx, r8.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r8.xxyz)+(r1.xxyz)).xzw;
    // 101: dp3 r1.x, r0.xzwx, r0.xzwx
    r1.x = (dot((r0.xzwx).xyz,(r0.xzwx).xyz).xxxx).x;
    // 102: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 103: mul r1.xyz, r0.xzwx, r1.xxxx
    r1.xyz = ((r0.xzwx)*(r1.xxxx)).xyz;
    // 104: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 105: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 106: mul r8.xyz, r2.wwww, v6.xyzx
    r8.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 107: dp3 r2.w, r8.xyzx, r1.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 108: mad r7.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 110: mul r9.xyz, r7.yyyy, cb0[23].xyzx
    r9.xyz = ((r7.yyyy)*(source[23].xyzx)).xyz;
    // 111: mad r9.xyz, r7.xxxx, cb0[22].xyzx, r9.xyzx
    r9.xyz = ((r7.xxxx)*(source[22].xyzx)+(r9.xyzx)).xyz;
    // 112: mul r9.xyz, r9.xyzx, cb0[24].wwww
    r9.xyz = ((r9.xyzx)*(source[24].wwww)).xyz;
    // 113: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 114: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 115: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 116: dp3 r9.y, r5.xyzx, r1.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 117: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 118: dp3 r10.y, r4.xyzx, r1.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 119: mul r7.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 120: dp2 r10.x, r9.xyxx, r7.xyxx
    r10.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 121: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 122: dp4 r11.x, cb0[14].xyzw, r10.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 123: dp4 r11.y, cb0[15].xyzw, r10.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 124: dp4 r11.z, cb0[16].xyzw, r10.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 125: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 126: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 127: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 128: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 129: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 130: mul r2.w, r10.y, r10.y
    r2.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 131: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 132: mad r2.w, r10.x, r10.x, -r2.w
    r2.w = ((r10.xxxx)*(r10.xxxx)+(-(r2.wwww))).w;
    // 133: mad r10.xyz, cb0[20].xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((source[20].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 134: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r10.xyz, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((r10.xyzx)*(source[12].xyzx)).xyz;
    // 136: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 137: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 138: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 139: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 140: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 141: mad r11.xyz, r7.wwww, r11.xyzx, r2.wwww
    r11.xyz = ((r7.wwww)*(r11.xyzx)+(r2.wwww)).xyz;
    // 142: mul_sat r2.w, r11.y, l(50.000000)
    r2.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 143: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 144: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 145: mul r3.w, r3.w, cb0[11].x
    r3.w = ((r3.wwww)*(source[11].xxxx)).w;
    // 146: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 147: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 148: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 149: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 151: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 152: mul r12.xyz, r1.wwww, v5.xyzx
    r12.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 153: dp3 r1.w, r1.xyzx, r12.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 154: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 155: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 156: deriv_rtx_coarse r13.x, r1.w
    r13.x = (ddx_coarse(r1.wwww)).x;
    // 157: deriv_rty_coarse r13.y, r1.w
    r13.y = (ddy_coarse(r1.wwww)).y;
    // 158: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 160: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 161: mad_sat r13.y, r3.w, l(0.300000), r7.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 162: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 163: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 165: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 166: mul r14.xyz, r2.wwww, r14.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 167: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add_sat r13.x, r1.w, -r2.w
    r13.x = (saturate((r1.wwww)+(-(r2.wwww)))).x;
    // 170: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 171: add r1.w, r0.y, r13.x
    r1.w = ((r0.yyyy)+(r13.xxxx)).w;
    // 172: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 173: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 174: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 175: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 176: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 177: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 178: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 179: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 180: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 182: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 183: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 184: mad r6.xyz, -r6.xyzx, r7.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r7.wwww)+(r6.xyzx)).xyz;
    // 185: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 186: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 187: dp2 r5.x, r2.xyxx, r7.xyxx
    r5.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 188: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 189: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 190: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 191: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 192: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 193: add r1.w, r0.y, r1.w
    r1.w = ((r0.yyyy)+(r1.wwww)).w;
    // 194: mov o5.y, r0.y
    output.targets[5].y = (r0.yyyy).y;
    // 195: add_sat r0.y, r1.w, l(-1.000000)
    r0.y = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 196: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 197: dp3 r1.x, r8.xyzx, r1.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 198: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s5, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 202: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 203: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 204: mad r1.z, r0.y, r11.x, r11.y
    r1.z = ((r0.yyyy)*(r11.xxxx)+(r11.yyyy)).z;
    // 205: mad r1.z, r1.z, r0.y, r11.z
    r1.z = ((r1.zzzz)*(r0.yyyy)+(r11.zzzz)).z;
    // 206: mul r1.z, r0.y, r1.z
    r1.z = ((r0.yyyy)*(r1.zzzz)).z;
    // 207: max r0.y, r0.y, r1.z
    r0.y = (max(r0.yyyy,r1.zzzz)).y;
    // 208: mul r1.yzw, r1.yyyy, cb0[23].xxyz
    r1.yzw = ((r1.yyyy)*(source[23].xxyz)).yzw;
    // 209: mad r1.xyz, cb0[22].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[22].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 210: mul r1.xyz, r1.xyzx, cb0[24].wwww
    r1.xyz = ((r1.xyzx)*(source[24].wwww)).xyz;
    // 211: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 212: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 213: mad r2.xyz, r1.xyzx, r13.xzwx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r13.xzwx)+(r6.xyzx)).xyz;
    // 214: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 215: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: dp3 r0.x, r0.xzwx, r12.xyzx
    r0.x = (dot((r0.xzwx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 217: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 218: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 219: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 220: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 221: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 222: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 223: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 224: mul r0.xzw, r0.xxxx, cb0[3].xxyz
    r0.xzw = ((r0.xxxx)*(source[3].xxyz)).xzw;
    // 225: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 226: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 227: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 228: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 229: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 230: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 231: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 232: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 233: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 234: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 235: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 236: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 237: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 238: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 239: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 240: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 241: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 242: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 243: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 244: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 245: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 246: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 247: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 248: ret
    return output;
}

// source.character.static-map-native-1137.v1 / source program 7ab5b38163421e4e982de6ccce5fd5c6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1137(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[8]=g_SourceCharacterEnvironmentColor;source[9]=g_SourceCharacterEnvironmentRotation;}
    source[21]=1.f;
    source[22]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
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
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 8: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 9: dp2 r0.x, r1.xyxx, r1.xyxx
    r0.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 10: mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 11: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 14: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 16: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 17: div r1.xyz, r1.xyzx, r0.xxxx
    r1.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 18: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 19: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 20: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 21: dp2_sat r2.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 22: dp3_sat r2.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 23: dp3_sat r2.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 24: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 25: mul r3.xyz, cb0[3].xyzx, cb0[5].zzzz
    r3.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 26: mul r3.xyz, r0.yzwy, r3.xyzx
    r3.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 27: mul r4.xyz, cb0[4].xyzx, cb0[5].wwww
    r4.xyz = ((source[4].xyzx)*(source[5].wwww)).xyz;
    // 28: mad r0.xyz, r4.xyzx, r0.yzwy, -r3.xyzx
    r0.xyz = ((r4.xyzx)*(r0.yzwy)+(-(r3.xyzx))).xyz;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 30: max r0.w, r4.z, l(0.000000)
    r0.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 31: mul r4.xy, r4.yxyy, cb0[7].xzxx
    r4.xy = ((r4.yxyy)*(source[7].xzxx)).xy;
    // 32: min r0.w, r0.w, l(100.000000)
    r0.w = (min(r0.wwww,float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 33: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 34: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 35: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 36: mul r1.w, r1.w, cb0[6].y
    r1.w = ((r1.wwww)*(source[6].yyyy)).w;
    // 37: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 38: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 40: mad r0.xyz, r0.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 41: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 42: add r5.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: mad r2.xyz, r2.xyzx, r5.xyzx, r0.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 44: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t7.xyzw, s4
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 45: mul r5.xyz, r5.xyzx, cb0[22].xyzx
    r5.xyz = ((r5.xyzx)*(source[22].xyzx)).xyz;
    // 46: dp3 r0.w, r5.xyzx, r2.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 47: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s4
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 48: mul r2.xyz, r2.xyzx, cb0[21].xyzx
    r2.xyz = ((r2.xyzx)*(source[21].xyzx)).xyz;
    // 49: mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 50: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 51: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 52: mul r7.xyz, r1.wwww, v6.xyzx
    r7.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 53: dp3 r1.w, r7.xyzx, r1.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 54: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 55: mad r7.xyz, -r3.yyyy, r3.yyyy, r0.xyzx
    r7.xyz = ((-(r3.yyyy))*(r3.yyyy)+(r0.xyzx)).xyz;
    // 56: mul r4.zw, r3.xxxy, r3.xxxy
    r4.zw = ((r3.xxxy)*(r3.xxxy)).zw;
    // 57: mad r8.xyz, -r3.xxxx, r3.xxxx, r0.xyzx
    r8.xyz = ((-(r3.xxxx))*(r3.xxxx)+(r0.xyzx)).xyz;
    // 58: mad r8.xyz, r0.xyzx, r8.xyzx, r4.zzzz
    r8.xyz = ((r0.xyzx)*(r8.xyzx)+(r4.zzzz)).xyz;
    // 59: mad r7.xyz, r0.xyzx, r7.xyzx, r4.wwww
    r7.xyz = ((r0.xyzx)*(r7.xyzx)+(r4.wwww)).xyz;
    // 60: mul r7.xyz, r7.xyzx, cb0[19].xyzx
    r7.xyz = ((r7.xyzx)*(source[19].xyzx)).xyz;
    // 61: mad r7.xyz, r8.xyzx, cb0[18].xyzx, r7.xyzx
    r7.xyz = ((r8.xyzx)*(source[18].xyzx)+(r7.xyzx)).xyz;
    // 62: mul r7.xyz, r7.xyzx, cb0[20].wwww
    r7.xyz = ((r7.xyzx)*(source[20].wwww)).xyz;
    // 63: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 64: mul r8.xyz, r0.xyzx, r8.xyzx
    r8.xyz = ((r0.xyzx)*(r8.xyzx)).xyz;
    // 65: mul r0.xyz, r0.xyzx, l(0.101321, 0.101321, 0.101321, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.101321,0.101321,0.101321,0.000000))).xyz;
    // 66: dp3_sat o5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 67: mad_sat r8.xyz, r8.xyzx, cb2[3].wwww, cb2[3].xyzx
    r8.xyz = (saturate((r8.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 68: mul r0.xyz, r7.xyzx, r8.xyzx
    r0.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 69: mad r0.xyz, r8.xyzx, r6.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 70: mad r6.xyz, r8.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r8.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 71: mad r9.xyz, r8.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r9.xyz = ((r8.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 72: mad r10.xyz, r8.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r8.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 73: log r3.xy, |r4.xyxx|
    r3.xy = (log2(abs(r4.xyxx))).xy;
    // 74: lt r4.xy, |r4.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r4.xy = (asfloat((uint4)((abs(r4.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 75: mul r3.xy, r3.xyxx, cb0[7].ywyy
    r3.xy = ((r3.xyxx)*(source[7].ywyy)).xy;
    // 76: exp r3.xy, r3.xyxx
    r3.xy = (exp2(r3.xyxx)).xy;
    // 77: min r1.w, r3.y, l(1.000000)
    r1.w = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 78: movc r2.w, r4.x, l(0), r3.x
    r2.w = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 79: movc r1.w, r4.y, l(0), r1.w
    r1.w = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 80: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 81: min r3.z, r2.w, l(1.000000)
    r3.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 82: mad r4.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r4.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 83: mad r4.xyz, r4.xyzx, r1.wwww, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r1.wwww)+(r6.xyzx)).xyz;
    // 84: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 85: max r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = (max(r1.wwww,r4.xyzx)).xyz;
    // 86: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 87: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 88: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 89: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 90: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 91: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 92: mul r6.xyz, r2.wwww, v0.xyzx
    r6.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 93: mul r9.xyz, r4.zxyz, r6.yzxy
    r9.xyz = ((r4.zxyz)*(r6.yzxy)).xyz;
    // 94: mad r9.xyz, r4.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r4.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 95: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 96: dp3 r10.y, r9.xyzx, r1.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 97: dp3 r10.x, r6.xyzx, r1.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 98: dp2 r11.z, r10.xyxx, cb0[9].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[9].xyxx).xy).xxxx).z;
    // 99: mul r3.xy, cb0[9].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((source[9].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 100: dp2 r11.x, r10.xyxx, r3.xyxx
    r11.x = (dot((r10.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 101: dp3 r11.y, r4.xyzx, r1.xyzx
    r11.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 102: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 103: dp4 r12.x, cb0[10].xyzw, r11.xyzw
    r12.x = (dot((source[10].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 104: dp4 r12.y, cb0[11].xyzw, r11.xyzw
    r12.y = (dot((source[11].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 105: dp4 r12.z, cb0[12].xyzw, r11.xyzw
    r12.z = (dot((source[12].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 106: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 107: dp4 r14.x, cb0[13].xyzw, r13.xyzw
    r14.x = (dot((source[13].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 108: dp4 r14.y, cb0[14].xyzw, r13.xyzw
    r14.y = (dot((source[14].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 109: dp4 r14.z, cb0[15].xyzw, r13.xyzw
    r14.z = (dot((source[15].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 110: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 111: mul r2.w, r11.y, r11.y
    r2.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 112: mov r10.z, r11.y
    r10.z = (r11.yyyy).z;
    // 113: mad r2.w, r11.x, r11.x, -r2.w
    r2.w = ((r11.xxxx)*(r11.xxxx)+(-(r2.wwww))).w;
    // 114: mad r11.xyz, cb0[16].xyzx, r2.wwww, r12.xyzx
    r11.xyz = ((source[16].xyzx)*(r2.wwww)+(r12.xyzx)).xyz;
    // 115: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 116: mul r11.xyz, r11.xyzx, cb0[8].xyzx
    r11.xyz = ((r11.xyzx)*(source[8].xyzx)).xyz;
    // 117: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[8].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[8].wwww)).xyz;
    // 118: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 119: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 120: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 121: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 122: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 123: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 124: deriv_rtx_coarse r12.x, r2.w
    r12.x = (ddx_coarse(r2.wwww)).x;
    // 125: deriv_rty_coarse r12.y, r2.w
    r12.y = (ddy_coarse(r2.wwww)).y;
    // 126: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 128: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 129: mad_sat r12.y, r4.w, l(0.300000), r3.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 130: add r4.w, -r12.y, l(1.000000)
    r4.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: mov_sat r8.w, cb0[6].z
    r8.w = (saturate(source[6].zzzz)).w;
    // 132: mad r13.xyz, -r8.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r8.xyzx
    r13.xyz = ((-(r8.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r8.xyzx)).xyz;
    // 133: mul r5.w, r8.w, l(0.080000)
    r5.w = ((r8.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 134: mov o3.xyzw, r8.xyzw
    output.targets[3].xyzw = (r8.xyzw).xyzw;
    // 135: mad r13.xyz, r3.wwww, r13.xyzx, r5.wwww
    r13.xyz = ((r3.wwww)*(r13.xyzx)+(r5.wwww)).xyz;
    // 136: max r14.xyz, r4.wwww, r13.xyzx
    r14.xyz = (max(r4.wwww,r13.xyzx)).xyz;
    // 137: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 138: mul_sat r4.w, r13.y, l(50.000000)
    r4.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 139: mul r14.xyz, r4.wwww, r14.xyzx
    r14.xyz = ((r4.wwww)*(r14.xyzx)).xyz;
    // 140: add r4.w, r1.z, l(1.000000)
    r4.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: add_sat r12.x, r2.w, -r4.w
    r12.x = (saturate((r2.wwww)+(-(r4.wwww)))).x;
    // 143: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t4.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 144: add r2.w, r1.w, r12.x
    r2.w = ((r1.wwww)+(r12.xxxx)).w;
    // 145: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 146: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 147: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 148: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r4.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 149: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 150: mad r12.xzw, r13.xxyz, r4.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r4.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 151: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 152: mad r13.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 153: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 154: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 155: mul r11.xyz, r11.xyzx, r15.xyzx
    r11.xyz = ((r11.xyzx)*(r15.xyzx)).xyz;
    // 156: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 157: mad r0.xyz, -r0.xyzx, r3.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r3.wwww)+(r0.xyzx)).xyz;
    // 158: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 159: mul r3.w, r12.y, l(5.000000)
    r3.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 160: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 161: mul r2.w, r2.w, r4.w
    r2.w = ((r2.wwww)*(r4.wwww)).w;
    // 162: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 163: add r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)+(r2.wwww)).w;
    // 164: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 165: add_sat r1.w, r2.w, l(-1.000000)
    r1.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 166: dp3 r6.x, r6.xyzx, r1.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 167: dp3 r6.y, r9.xyzx, r1.xyzx
    r6.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 168: dp2 r9.x, r6.xyxx, r3.xyxx
    r9.x = (dot((r6.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 169: dp2 r9.z, r6.xyxx, cb0[9].xyxx
    r9.z = (dot((r6.xyxx).xy,(source[9].xyxx).xy).xxxx).z;
    // 170: dp3 r9.y, r4.xyzx, r1.xyzx
    r9.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 171: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r9.xyzx, t5.xyzw, s5, r3.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 172: mul r3.xyw, r4.xyxz, r4.wwww
    r3.xyw = ((r4.xyxz)*(r4.wwww)).xyw;
    // 173: mul r3.xyw, r3.xyxw, cb0[8].xyxz
    r3.xyw = ((r3.xyxw)*(source[8].xyxz)).xyw;
    // 174: mad r3.xyw, r3.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[8].wwww
    r3.xyw = ((r3.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[8].wwww)).xyw;
    // 175: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 176: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 177: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 178: mul r1.xyz, r4.xyzx, r4.xyzx
    r1.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 179: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 180: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 181: mad r0.w, r3.z, r0.w, r1.x
    r0.w = ((r3.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 182: mad r1.xyz, r2.xyzx, r0.wwww, r7.xyzx
    r1.xyz = ((r2.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 183: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 184: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 185: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 186: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 187: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 188: mul r4.xyz, r0.wwww, r1.xyzx
    r4.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 189: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 190: div r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)/(r1.xyzx)).xyz;
    // 191: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: mul r1.xyz, r3.xywx, r4.xyzx
    r1.xyz = ((r3.xywx)*(r4.xyzx)).xyz;
    // 193: mad r0.xyz, r1.xyzx, r12.xzwx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r12.xzwx)+(r0.xyzx)).xyz;
    // 194: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 195: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 196: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 197: mul r2.xyz, cb0[2].xyzx, cb0[5].yyyy
    r2.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // 198: mad r1.xyz, r1.xyzx, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)+(source[1].xyzx)).xyz;
    // 199: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 200: mad o0.xyz, r8.xyzx, cb0[20].xyzx, r1.xyzx
    output.targets[0].xyz = ((r8.xyzx)*(source[20].xyzx)+(r1.xyzx)).xyz;
    // 201: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 202: dp3 r1.x, r10.xyzx, r10.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 203: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 204: mul r1.xyz, r1.xxxx, r10.xyzx
    r1.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 205: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 206: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 207: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 208: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 209: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 210: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 211: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 212: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 213: mul o4.z, r0.w, r0.x
    output.targets[4].z = ((r0.wwww)*(r0.xxxx)).z;
    // 214: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 215: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 216: ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // 217: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 218: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 219: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 220: mov o5.z, l(1.000000)
    output.targets[5].z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 221: ret
    return output;
}

// source.character.static-map-native-1137.v1 / source program 9676727653fce642af5f52aca7a079d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1137(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1137(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[8]=g_SourceCharacterEnvironmentColor;source[9]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
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
    // 7: mul r1.xyz, cb0[3].xyzx, cb0[5].zzzz
    r1.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 8: mul r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 9: mul r2.xyz, cb0[4].xyzx, cb0[5].wwww
    r2.xyz = ((source[4].xyzx)*(source[5].wwww)).xyz;
    // 10: mad r0.xyz, r2.xyzx, r0.yzwy, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.yzwy)+(-(r1.xyzx))).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 12: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: mul r2.xy, r2.yxyy, cb0[7].xzxx
    r2.xy = ((r2.yxyy)*(source[7].xzxx)).xy;
    // 14: min r0.w, r0.w, l(100.000000)
    r0.w = (min(r0.wwww,float4(100.000000,100.000000,100.000000,100.000000))).w;
    // 15: mul r0.w, r0.w, cb0[6].x
    r0.w = ((r0.wwww)*(source[6].xxxx)).w;
    // 16: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 17: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r1.w, r1.w, cb0[6].y
    r1.w = ((r1.wwww)*(source[6].yyyy)).w;
    // 19: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 20: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 22: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul_sat r1.w, r0.w, cb2[3].w
    r1.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 27: mul r3.xy, r1.xyxx, cb0[5].xxxx
    r3.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 28: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 31: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 32: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 33: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 34: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 35: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 36: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 37: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 38: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 39: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 40: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 41: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 42: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: mad r5.xyz, -r1.yyyy, r1.yyyy, r0.xyzx
    r5.xyz = ((-(r1.yyyy))*(r1.yyyy)+(r0.xyzx)).xyz;
    // 44: mul r2.zw, r1.xxxy, r1.xxxy
    r2.zw = ((r1.xxxy)*(r1.xxxy)).zw;
    // 45: mad r6.xyz, -r1.xxxx, r1.xxxx, r0.xyzx
    r6.xyz = ((-(r1.xxxx))*(r1.xxxx)+(r0.xyzx)).xyz;
    // 46: mad r6.xyz, r0.xyzx, r6.xyzx, r2.zzzz
    r6.xyz = ((r0.xyzx)*(r6.xyzx)+(r2.zzzz)).xyz;
    // 47: mad r5.xyz, r0.xyzx, r5.xyzx, r2.wwww
    r5.xyz = ((r0.xyzx)*(r5.xyzx)+(r2.wwww)).xyz;
    // 48: mul r5.xyz, r5.xyzx, cb0[19].xyzx
    r5.xyz = ((r5.xyzx)*(source[19].xyzx)).xyz;
    // 49: mad r5.xyz, r6.xyzx, cb0[18].xyzx, r5.xyzx
    r5.xyz = ((r6.xyzx)*(source[18].xyzx)+(r5.xyzx)).xyz;
    // 50: mul r5.xyz, r5.xyzx, cb0[20].wwww
    r5.xyz = ((r5.xyzx)*(source[20].wwww)).xyz;
    // 51: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 52: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 53: mul r0.xyz, r0.xyzx, l(0.101321, 0.101321, 0.101321, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.101321,0.101321,0.101321,0.000000))).xyz;
    // 54: dp3_sat o5.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[5].x = (saturate(dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx)).x;
    // 55: mad_sat r0.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 56: mul r5.xyz, r0.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 57: mad r6.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 58: mad r7.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 59: mad r8.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 60: log r1.xy, |r2.xyxx|
    r1.xy = (log2(abs(r2.xyxx))).xy;
    // 61: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 62: mul r1.xy, r1.xyxx, cb0[7].ywyy
    r1.xy = ((r1.xyxx)*(source[7].ywyy)).xy;
    // 63: exp r1.xy, r1.xyxx
    r1.xy = (exp2(r1.xyxx)).xy;
    // 64: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 65: movc r1.xy, r2.xyxx, l(0,0,0,0), r1.xyxx
    r1.xy = ((asuint(r2.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyxx)).xy;
    // 66: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 67: min r1.z, r1.x, l(1.000000)
    r1.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 68: mad r2.xyz, r1.yyyy, r7.xyzx, r8.xyzx
    r2.xyz = ((r1.yyyy)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 69: mad r2.xyz, r2.xyzx, r1.yyyy, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r1.yyyy)+(r6.xyzx)).xyz;
    // 70: mul r2.xyz, r1.yyyy, r2.xyzx
    r2.xyz = ((r1.yyyy)*(r2.xyzx)).xyz;
    // 71: max r2.xyz, r1.yyyy, r2.xyzx
    r2.xyz = (max(r1.yyyy,r2.xyzx)).xyz;
    // 72: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 73: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 74: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 75: mul r5.xyz, r1.xxxx, v1.xyzx
    r5.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 76: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 77: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 78: mul r6.xyz, r1.xxxx, v0.xyzx
    r6.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 79: mul r7.xyz, r5.zxyz, r6.yzxy
    r7.xyz = ((r5.zxyz)*(r6.yzxy)).xyz;
    // 80: mad r7.xyz, r5.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r5.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 81: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 82: dp3 r8.y, r7.xyzx, r3.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 83: dp3 r8.x, r6.xyzx, r3.xyzx
    r8.x = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 84: dp2 r9.z, r8.xyxx, cb0[9].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[9].xyxx).xy).xxxx).z;
    // 85: mul r10.xy, cb0[9].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((source[9].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 86: dp2 r9.x, r8.xyxx, r10.xyxx
    r9.x = (dot((r8.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 87: dp3 r9.y, r5.xyzx, r3.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 88: mov r9.w, l(1.000000)
    r9.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 89: dp4 r11.x, cb0[10].xyzw, r9.xyzw
    r11.x = (dot((source[10].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 90: dp4 r11.y, cb0[11].xyzw, r9.xyzw
    r11.y = (dot((source[11].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 91: dp4 r11.z, cb0[12].xyzw, r9.xyzw
    r11.z = (dot((source[12].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 92: mul r12.xyzw, r9.yzzx, r9.xyzz
    r12.xyzw = ((r9.yzzx)*(r9.xyzz)).xyzw;
    // 93: dp4 r13.x, cb0[13].xyzw, r12.xyzw
    r13.x = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 94: dp4 r13.y, cb0[14].xyzw, r12.xyzw
    r13.y = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 95: dp4 r13.z, cb0[15].xyzw, r12.xyzw
    r13.z = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 96: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 97: mul r1.x, r9.y, r9.y
    r1.x = ((r9.yyyy)*(r9.yyyy)).x;
    // 98: mov r8.z, r9.y
    r8.z = (r9.yyyy).z;
    // 99: mad r1.x, r9.x, r9.x, -r1.x
    r1.x = ((r9.xxxx)*(r9.xxxx)+(-(r1.xxxx))).x;
    // 100: mad r9.xyz, cb0[16].xyzx, r1.xxxx, r11.xyzx
    r9.xyz = ((source[16].xyzx)*(r1.xxxx)+(r11.xyzx)).xyz;
    // 101: max r9.xyz, r9.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 102: mul r9.xyz, r9.xyzx, cb0[8].xyzx
    r9.xyz = ((r9.xyzx)*(source[8].xyzx)).xyz;
    // 103: mad r9.xyz, r9.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[8].wwww)).xyz;
    // 104: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 105: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 106: mul r11.xyz, r1.xxxx, v5.xyzx
    r11.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 107: dp3 r1.x, r3.xyzx, r11.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 108: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 109: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 110: deriv_rtx_coarse r11.x, r1.x
    r11.x = (ddx_coarse(r1.xxxx)).x;
    // 111: deriv_rty_coarse r11.y, r1.x
    r11.y = (ddy_coarse(r1.xxxx)).y;
    // 112: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 113: dp2 r2.w, r11.xyxx, r11.xyxx
    r2.w = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).w;
    // 114: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 115: mad_sat r11.y, r2.w, l(0.300000), r1.z
    r11.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 116: mov o2.zw, r1.zzzw
    output.targets[2].zw = (r1.zzzw).zw;
    // 117: add r1.z, -r11.y, l(1.000000)
    r1.z = ((-(r11.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 118: mov_sat r0.w, cb0[6].z
    r0.w = (saturate(source[6].zzzz)).w;
    // 119: mad r12.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r12.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 120: mul r2.w, r0.w, l(0.080000)
    r2.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 121: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 122: mad r12.xyz, r1.wwww, r12.xyzx, r2.wwww
    r12.xyz = ((r1.wwww)*(r12.xyzx)+(r2.wwww)).xyz;
    // 123: max r13.xyz, r1.zzzz, r12.xyzx
    r13.xyz = (max(r1.zzzz,r12.xyzx)).xyz;
    // 124: add r13.xyz, -r12.xyzx, r13.xyzx
    r13.xyz = ((-(r12.xyzx))+(r13.xyzx)).xyz;
    // 125: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 126: mul r13.xyz, r0.wwww, r13.xyzx
    r13.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 127: add r0.w, r3.z, l(1.000000)
    r0.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: add_sat r11.x, -r0.w, r1.x
    r11.x = (saturate((-(r0.wwww))+(r1.xxxx))).x;
    // 130: sample_indexable(texture2d)(float,float,float,float) r1.xz, r11.xyxx, t4.xzyw, s5
    r1.xz = ((float4(0.0,0.0,0.0,0.0)).xzyw).xz;
    // 131: add r0.w, r1.y, r11.x
    r0.w = ((r1.yyyy)+(r11.xxxx)).w;
    // 132: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 133: mul r11.xzw, r1.zzzz, r12.xxyz
    r11.xzw = ((r1.zzzz)*(r12.xxyz)).xzw;
    // 134: mad r11.xzw, r13.xxyz, r1.xxxx, r11.xxzw
    r11.xzw = ((r13.xxyz)*(r1.xxxx)+(r11.xxzw)).xzw;
    // 135: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.z
    r1.x = r1.z != 0.f ? 1.f / r1.z : 0.f;
    // 136: add r1.x, r1.x, l(-1.000000)
    r1.x = ((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 137: mad r13.xyz, r12.xyzx, r1.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r12.xyzx)*(r1.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 138: dp3 r1.x, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 139: mad r12.xyz, r1.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r1.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 140: mad r14.xyz, -r11.xzwx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r11.xzwx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 141: mul r11.xzw, r11.xxzw, r13.xxyz
    r11.xzw = ((r11.xxzw)*(r13.xxyz)).xzw;
    // 142: mul r9.xyz, r9.xyzx, r14.xyzx
    r9.xyz = ((r9.xyzx)*(r14.xyzx)).xyz;
    // 143: mul r2.xyz, r2.xyzx, r9.xyzx
    r2.xyz = ((r2.xyzx)*(r9.xyzx)).xyz;
    // 144: mad r1.xzw, -r2.xxyz, r1.wwww, r2.xxyz
    r1.xzw = ((-(r2.xxyz))*(r1.wwww)+(r2.xxyz)).xzw;
    // 145: mul r2.x, r11.y, l(5.000000)
    r2.x = ((r11.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 146: mul r2.y, r11.y, r11.y
    r2.y = ((r11.yyyy)*(r11.yyyy)).y;
    // 147: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 148: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 149: add r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)+(r0.wwww)).w;
    // 150: mov o5.y, r1.y
    output.targets[5].y = (r1.yyyy).y;
    // 151: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 152: dp3 r6.x, r6.xyzx, r3.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 153: dp3 r6.y, r7.xyzx, r3.xyzx
    r6.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 154: dp2 r7.x, r6.xyxx, r10.xyxx
    r7.x = (dot((r6.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 155: dp2 r7.z, r6.xyxx, cb0[9].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[9].xyxx).xy).xxxx).z;
    // 156: dp3 r7.y, r5.xyzx, r3.xyzx
    r7.y = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 157: dp3 r1.y, r4.xyzx, r3.xyzx
    r1.y = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 158: mad r2.yz, r1.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r1.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 159: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 160: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r7.xyzx, t5.xyzw, s4, r2.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r7.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 161: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 162: mul r3.xyz, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((r3.xyzx)*(source[8].xyzx)).xyz;
    // 163: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[8].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[8].wwww)).xyz;
    // 164: mad r1.y, r0.w, r12.x, r12.y
    r1.y = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).y;
    // 165: mad r1.y, r1.y, r0.w, r12.z
    r1.y = ((r1.yyyy)*(r0.wwww)+(r12.zzzz)).y;
    // 166: mul r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)*(r1.yyyy)).y;
    // 167: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 168: mul r2.xzw, r2.zzzz, cb0[19].xxyz
    r2.xzw = ((r2.zzzz)*(source[19].xxyz)).xzw;
    // 169: mad r2.xyz, cb0[18].xyzx, r2.yyyy, r2.xzwx
    r2.xyz = ((source[18].xyzx)*(r2.yyyy)+(r2.xzwx)).xyz;
    // 170: mul r2.xyz, r2.xyzx, cb0[20].wwww
    r2.xyz = ((r2.xyzx)*(source[20].wwww)).xyz;
    // 171: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 172: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 173: mad r1.xyz, r2.xyzx, r11.xzwx, r1.xzwx
    r1.xyz = ((r2.xyzx)*(r11.xzwx)+(r1.xzwx)).xyz;
    // 174: mul r2.xyz, r11.xzwx, r2.xyzx
    r2.xyz = ((r11.xzwx)*(r2.xyzx)).xyz;
    // 175: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 176: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 177: mul r3.xyz, cb0[2].xyzx, cb0[5].yyyy
    r3.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // 178: mad r2.xyz, r2.xyzx, r3.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // 179: add r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 180: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 181: mad o0.xyz, r0.xyzx, cb0[20].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)+(r2.xyzx)).xyz;
    // 182: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 183: dp3 r0.x, r8.xyzx, r8.xyzx
    r0.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 184: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 185: mul r0.xyz, r0.xxxx, r8.xyzx
    r0.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 186: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 187: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 188: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 189: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 190: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 191: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 192: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 193: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 194: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 195: ftou r0.x, cb0[17].z
    r0.x = (asfloat((uint4)(source[17].zzzz))).x;
    // 196: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 197: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 198: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 199: mov o5.z, l(1.000000)
    output.targets[5].z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 200: ret
    return output;
}

// source.character.static-map-native-1138.v1 / source program 4fc9bcd8574fdb41b182d94e2f7955bb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1138(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[10]=g_SourceCharacterEnvironmentColor;source[11]=g_SourceCharacterEnvironmentRotation;}
    source[23]=1.f;
    source[24]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: mul r0.w, r1.z, cb0[6].z
    r0.w = ((r1.zzzz)*(source[6].zzzz)).w;
    // 7: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 8: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 9: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 10: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 11: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 12: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 13: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 15: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 16: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 17: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 18: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 19: mul r2.xyz, r1.wwww, r1.xyzx
    r2.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 20: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 21: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 22: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 23: mul r3.xyz, r0.yyyy, cb0[21].xyzx
    r3.xyz = ((r0.yyyy)*(source[21].xyzx)).xyz;
    // 24: mad r0.xyz, r0.xxxx, cb0[20].xyzx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(source[20].xyzx)+(r3.xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, cb0[22].wwww
    r0.xyz = ((r0.xyzx)*(source[22].wwww)).xyz;
    // 26: mul r3.xyz, cb0[5].xyzx, cb0[7].xxxx
    r3.xyz = ((source[5].xyzx)*(source[7].xxxx)).xyz;
    // 27: mul r4.xyz, cb0[4].xyzx, cb0[6].wwww
    r4.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 29: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 30: mad r3.xyz, r3.xyzx, r5.xyzx, -r4.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)+(-(r4.xyzx))).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: mul r1.w, r5.z, cb0[7].y
    r1.w = ((r5.zzzz)*(source[7].yyyy)).w;
    // 33: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 34: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: mul r2.w, r2.w, cb0[7].z
    r2.w = ((r2.wwww)*(source[7].zzzz)).w;
    // 36: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 37: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 38: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mul_sat r5.w, r1.w, cb2[3].w
    r5.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 40: mad r3.xyz, r2.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 41: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 42: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 43: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 44: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 45: mul r6.xyz, r1.wwww, r2.xyzx
    r6.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 46: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 47: add r7.xyz, r6.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r7.xyz = ((r6.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 48: mad r7.xy, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 49: min r3.w, r7.z, l(1.000000)
    r3.w = (min(r7.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad r7.xy, r7.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 52: mul r7.xyz, r7.xyzx, cb0[3].xyzx
    r7.xyz = ((r7.xyzx)*(source[3].xyzx)).xyz;
    // 53: mad r7.xyz, cb0[6].yyyy, r7.xyzx, r7.xyzx
    r7.xyz = ((source[6].yyyy)*(r7.xyzx)+(r7.xyzx)).xyz;
    // 54: add r7.xyz, r7.xyzx, -cb0[6].yyyy
    r7.xyz = ((r7.xyzx)+(-(source[6].yyyy))).xyz;
    // 55: mov_sat r8.xyz, r7.xyzx
    r8.xyz = (saturate(r7.xyzx)).xyz;
    // 56: mov_sat r7.xyz, -r7.xyzx
    r7.xyz = (saturate(-(r7.xyzx))).xyz;
    // 57: mad r7.xyz, -r0.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r0.wwww))*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 58: mad r3.xyz, r0.wwww, r8.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 59: mul r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 60: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 61: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 62: mul r7.xyz, r3.xyzx, cb0[7].wwww
    r7.xyz = ((r3.xyzx)*(source[7].wwww)).xyz;
    // 63: mad r3.xyz, cb0[8].xxxx, r3.xyzx, -r7.xyzx
    r3.xyz = ((source[8].xxxx)*(r3.xyzx)+(-(r7.xyzx))).xyz;
    // 64: mad r3.xyz, r2.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 65: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 66: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 67: mad_sat r7.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 68: mul r3.xyz, r0.xyzx, r7.xyzx
    r3.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // 69: dp2_sat r8.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 70: dp3_sat r8.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 71: dp3_sat r8.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 72: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 73: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 74: mul r9.xyz, r9.xyzx, cb0[24].xyzx
    r9.xyz = ((r9.xyzx)*(source[24].xyzx)).xyz;
    // 75: dp3 r0.w, r9.xyzx, r8.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 76: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 77: mul r8.xyz, r8.xyzx, cb0[23].xyzx
    r8.xyz = ((r8.xyzx)*(source[23].xyzx)).xyz;
    // 78: mul r10.xyz, r0.wwww, r8.xyzx
    r10.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 79: mad r3.xyz, r7.xyzx, r10.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 80: mad r10.xyz, r7.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r7.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 81: mad r11.xyz, r7.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r7.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 82: mul r2.w, r5.x, cb0[9].y
    r2.w = ((r5.xxxx)*(source[9].yyyy)).w;
    // 83: mul r4.w, r5.y, cb0[8].w
    r4.w = ((r5.yyyy)*(source[8].wwww)).w;
    // 84: log r5.x, |r2.w|
    r5.x = (log2(abs(r2.wwww))).x;
    // 85: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: mul r5.x, r5.x, cb0[9].z
    r5.x = ((r5.xxxx)*(source[9].zzzz)).x;
    // 87: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 88: min r5.x, r5.x, l(1.000000)
    r5.x = (min(r5.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: movc r2.w, r2.w, l(0), r5.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).w;
    // 90: mad r10.xyz, r2.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 91: mad r11.xyz, r7.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r7.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 92: mad r10.xyz, r10.xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 93: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 94: max r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = (max(r2.wwww,r10.xyzx)).xyz;
    // 95: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 96: dp3 r5.x, v1.xyzx, v1.xyzx
    r5.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 97: rsq r5.x, r5.x
    r5.x = (rsqrt(r5.xxxx)).x;
    // 98: mul r10.xyz, r5.xxxx, v1.xyzx
    r10.xyz = ((r5.xxxx)*(v1.xyzx)).xyz;
    // 99: dp3 r5.x, v0.xyzx, v0.xyzx
    r5.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 100: rsq r5.x, r5.x
    r5.x = (rsqrt(r5.xxxx)).x;
    // 101: mul r11.xyz, r5.xxxx, v0.xyzx
    r11.xyz = ((r5.xxxx)*(v0.xyzx)).xyz;
    // 102: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 103: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 104: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 105: dp3 r13.y, r12.xyzx, r2.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 106: dp3 r5.y, r12.xyzx, r6.xyzx
    r5.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 107: dp3 r13.x, r11.xyzx, r2.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 108: dp3 r12.y, r10.xyzx, r2.xyzx
    r12.y = (dot((r10.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 109: dp3 r2.y, r10.xyzx, r6.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 110: dp3 r5.x, r11.xyzx, r6.xyzx
    r5.x = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 111: dp2 r12.z, r13.xyxx, cb0[11].xyxx
    r12.z = (dot((r13.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 112: mul r10.xy, cb0[11].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((source[11].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 113: dp2 r12.x, r13.xyxx, r10.xyxx
    r12.x = (dot((r13.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 114: dp2 r2.x, r5.xyxx, r10.xyxx
    r2.x = (dot((r5.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 115: dp2 r2.z, r5.xyxx, cb0[11].xyxx
    r2.z = (dot((r5.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 116: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 117: dp4 r10.x, cb0[12].xyzw, r12.xyzw
    r10.x = (dot((source[12].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 118: dp4 r10.y, cb0[13].xyzw, r12.xyzw
    r10.y = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 119: dp4 r10.z, cb0[14].xyzw, r12.xyzw
    r10.z = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 120: mul r11.xyzw, r12.yzzx, r12.xyzz
    r11.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 121: dp4 r14.x, cb0[15].xyzw, r11.xyzw
    r14.x = (dot((source[15].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 122: dp4 r14.y, cb0[16].xyzw, r11.xyzw
    r14.y = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 123: dp4 r14.z, cb0[17].xyzw, r11.xyzw
    r14.z = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 124: add r10.xyz, r10.xyzx, r14.xyzx
    r10.xyz = ((r10.xyzx)+(r14.xyzx)).xyz;
    // 125: mul r5.x, r12.y, r12.y
    r5.x = ((r12.yyyy)*(r12.yyyy)).x;
    // 126: mov r13.z, r12.y
    r13.z = (r12.yyyy).z;
    // 127: mad r5.x, r12.x, r12.x, -r5.x
    r5.x = ((r12.xxxx)*(r12.xxxx)+(-(r5.xxxx))).x;
    // 128: mad r10.xyz, cb0[18].xyzx, r5.xxxx, r10.xyzx
    r10.xyz = ((source[18].xyzx)*(r5.xxxx)+(r10.xyzx)).xyz;
    // 129: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 130: mul r10.xyz, r10.xyzx, cb0[10].xyzx
    r10.xyz = ((r10.xyzx)*(source[10].xyzx)).xyz;
    // 131: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 132: log r5.x, |r4.w|
    r5.x = (log2(abs(r4.wwww))).x;
    // 133: lt r4.w, |r4.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r4.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 134: mul r5.x, r5.x, cb0[9].x
    r5.x = ((r5.xxxx)*(source[9].xxxx)).x;
    // 135: exp r5.x, r5.x
    r5.x = (exp2(r5.xxxx)).x;
    // 136: movc r4.w, r4.w, l(0), r5.x
    r4.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).w;
    // 137: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 138: min r5.z, r4.w, l(1.000000)
    r5.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 139: deriv_rtx_coarse r5.x, r1.w
    r5.x = (ddx_coarse(r1.wwww)).x;
    // 140: deriv_rty_coarse r5.y, r1.w
    r5.y = (ddy_coarse(r1.wwww)).y;
    // 141: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: add_sat r11.x, -r3.w, r1.w
    r11.x = (saturate((-(r3.wwww))+(r1.wwww))).x;
    // 143: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 144: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 145: mad_sat r11.y, r1.w, l(0.300000), r5.z
    r11.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r5.zzzz))).y;
    // 146: add r1.w, -r11.y, l(1.000000)
    r1.w = ((-(r11.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mov_sat r7.w, cb0[8].y
    r7.w = (saturate(source[8].yyyy)).w;
    // 148: mad r12.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r7.xyzx
    r12.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r7.xyzx)).xyz;
    // 149: mul r3.w, r7.w, l(0.080000)
    r3.w = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 150: mov o3.xyzw, r7.xyzw
    output.targets[3].xyzw = (r7.xyzw).xyzw;
    // 151: mad r12.xyz, r5.wwww, r12.xyzx, r3.wwww
    r12.xyz = ((r5.wwww)*(r12.xyzx)+(r3.wwww)).xyz;
    // 152: max r14.xyz, r1.wwww, r12.xyzx
    r14.xyz = (max(r1.wwww,r12.xyzx)).xyz;
    // 153: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 154: mul_sat r1.w, r12.y, l(50.000000)
    r1.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 155: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 156: sample_indexable(texture2d)(float,float,float,float) r5.xy, r11.xyxx, t4.xyzw, s6
    r5.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 157: add r1.w, r2.w, r11.x
    r1.w = ((r2.wwww)+(r11.xxxx)).w;
    // 158: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 159: mul r11.xzw, r5.yyyy, r12.xxyz
    r11.xzw = ((r5.yyyy)*(r12.xxyz)).xzw;
    // 160: mad r11.xzw, r14.xxyz, r5.xxxx, r11.xxzw
    r11.xzw = ((r14.xxyz)*(r5.xxxx)+(r11.xxzw)).xzw;
    // 161: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r5.y
    r3.w = r5.y != 0.f ? 1.f / r5.y : 0.f;
    // 162: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 163: mad r14.xyz, r12.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r12.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: dp3 r3.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 165: mad r12.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 166: mad r15.xyz, -r11.xzwx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r11.xzwx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: mul r11.xzw, r11.xxzw, r14.xxyz
    r11.xzw = ((r11.xxzw)*(r14.xxyz)).xzw;
    // 168: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 169: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 170: mad r3.xyz, -r3.xyzx, r5.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r5.wwww)+(r3.xyzx)).xyz;
    // 171: mov o2.zw, r5.zzzw
    output.targets[2].zw = (r5.zzzw).zw;
    // 172: dp2_sat r10.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 173: dp3_sat r10.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 174: dp3_sat r10.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 175: mul r5.xyw, r10.xyxz, r10.xyxz
    r5.xyw = ((r10.xyxz)*(r10.xyxz)).xyw;
    // 176: dp3 r3.w, r9.xyzx, r5.xywx
    r3.w = (dot((r9.xyzx).xyz,(r5.xywx).xyz).xxxx).w;
    // 177: add r0.w, r0.w, -r3.w
    r0.w = ((r0.wwww)+(-(r3.wwww))).w;
    // 178: mad r0.w, r5.z, r0.w, r3.w
    r0.w = ((r5.zzzz)*(r0.wwww)+(r3.wwww)).w;
    // 179: mad r0.xyz, r8.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r8.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 180: mul r5.xyz, r0.wwww, r8.xyzx
    r5.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 181: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 182: mul r3.w, r11.y, l(5.000000)
    r3.w = ((r11.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 183: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r2.xyzx, t5.xyzw, s5, r3.w
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 184: mul r2.xyz, r6.xyzx, r6.wwww
    r2.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 185: mul r2.xyz, r2.xyzx, cb0[10].xyzx
    r2.xyz = ((r2.xyzx)*(source[10].xyzx)).xyz;
    // 186: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 187: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 188: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 189: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 190: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 191: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 192: mad r1.w, r0.w, r12.x, r12.y
    r1.w = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).w;
    // 193: mad r1.w, r1.w, r0.w, r12.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r12.zzzz)).w;
    // 194: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 195: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 196: mul r6.xyz, r0.wwww, r0.xyzx
    r6.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 197: add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 198: div r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)/(r0.xyzx)).xyz;
    // 199: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 200: mul r0.yzw, r2.xxyz, r6.xxyz
    r0.yzw = ((r2.xxyz)*(r6.xxyz)).yzw;
    // 201: mad r2.xyz, r0.yzwy, r11.xzwx, r3.xyzx
    r2.xyz = ((r0.yzwy)*(r11.xzwx)+(r3.xyzx)).xyz;
    // 202: mul r0.yzw, r11.xxzw, r0.yyzw
    r0.yzw = ((r11.xxzw)*(r0.yyzw)).yzw;
    // 203: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 204: dp3 r0.y, r1.xyzx, r4.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 205: add r0.z, -|r4.z|, l(1.000000)
    r0.z = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 206: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 207: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 208: lt r0.z, |r0.y|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 209: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 210: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 211: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 212: mul r1.xyz, r0.yyyy, cb0[2].xyzx
    r1.xyz = ((r0.yyyy)*(source[2].xyzx)).xyz;
    // 213: movc r0.yzw, r0.zzzz, l(0,0,0,0), r1.xxyz
    r0.yzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxyz)).yzw;
    // 214: add r0.yzw, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)+(source[1].xxyz)).yzw;
    // 215: add r0.yzw, r2.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)+(r0.yyzw)).yzw;
    // 216: mad o0.xyz, r7.xyzx, cb0[22].xyzx, r0.yzwy
    output.targets[0].xyz = ((r7.xyzx)*(source[22].xyzx)+(r0.yzwy)).xyz;
    // 217: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 218: dp3 r0.y, r13.xyzx, r13.xyzx
    r0.y = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 219: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 220: mul r0.yzw, r0.yyyy, r13.xxyz
    r0.yzw = ((r0.yyyy)*(r13.xxyz)).yzw;
    // 221: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 222: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 223: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 224: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 225: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 226: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 227: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 228: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 229: mul o4.z, r0.x, r2.x
    output.targets[4].z = ((r0.xxxx)*(r2.xxxx)).z;
    // 230: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 231: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 232: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 233: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 234: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 235: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 236: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 237: ret
    return output;
}

// source.character.static-map-native-1138.v1 / source program b2668b9d46cd81438c228e2115091233
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1138(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1138(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[10]=g_SourceCharacterEnvironmentColor;source[11]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: mul r0.xyz, cb0[5].xyzx, cb0[7].xxxx
    r0.xyz = ((source[5].xyzx)*(source[7].xxxx)).xyz;
    // 2: mul r1.xyz, cb0[4].xyzx, cb0[6].wwww
    r1.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 4: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 5: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 7: mul r0.w, r2.z, cb0[7].y
    r0.w = ((r2.zzzz)*(source[7].yyyy)).w;
    // 8: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 9: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 10: mul r1.w, r1.w, cb0[7].z
    r1.w = ((r1.wwww)*(source[7].zzzz)).w;
    // 11: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 12: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 13: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 15: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: mul r0.w, r1.z, cb0[6].z
    r0.w = ((r1.zzzz)*(source[6].zzzz)).w;
    // 19: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 20: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 21: mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 22: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 24: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 25: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 27: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 28: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 29: dp3 r3.x, r1.xyzx, r1.xyzx
    r3.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 30: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 31: mul r3.xyz, r1.xyzx, r3.xxxx
    r3.xyz = ((r1.xyzx)*(r3.xxxx)).xyz;
    // 32: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 33: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 34: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 35: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 36: mul r5.xyz, r3.wwww, r3.xyzx
    r5.xyz = ((r3.wwww)*(r3.xyzx)).xyz;
    // 37: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 38: add r6.xyz, r5.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r6.xyz = ((r5.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 39: mad r6.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 40: min r4.w, r6.z, l(1.000000)
    r4.w = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mad r6.xy, r6.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 43: mul r6.xyz, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 44: mad r6.xyz, cb0[6].yyyy, r6.xyzx, r6.xyzx
    r6.xyz = ((source[6].yyyy)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 45: add r6.xyz, r6.xyzx, -cb0[6].yyyy
    r6.xyz = ((r6.xyzx)+(-(source[6].yyyy))).xyz;
    // 46: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 47: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 48: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: mad r0.xyz, r0.wwww, r7.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 50: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 51: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 52: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 53: mul r6.xyz, r0.xyzx, cb0[7].wwww
    r6.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 54: mad r0.xyz, cb0[8].xxxx, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].xxxx)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 55: mad r0.xyz, r1.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 56: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 57: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 58: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 59: mov_sat r0.w, cb0[8].y
    r0.w = (saturate(source[8].yyyy)).w;
    // 60: mad r6.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r6.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 61: mul r1.w, r0.w, l(0.080000)
    r1.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 62: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 63: mad r6.xyz, r2.wwww, r6.xyzx, r1.wwww
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(r1.wwww)).xyz;
    // 64: mul r0.w, r2.y, cb0[8].w
    r0.w = ((r2.yyyy)*(source[8].wwww)).w;
    // 65: mul r1.w, r2.x, cb0[9].y
    r1.w = ((r2.xxxx)*(source[9].yyyy)).w;
    // 66: log r2.x, |r0.w|
    r2.x = (log2(abs(r0.wwww))).x;
    // 67: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 68: mul r2.x, r2.x, cb0[9].x
    r2.x = ((r2.xxxx)*(source[9].xxxx)).x;
    // 69: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 70: movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 71: max r0.w, r0.w, cb0[0].x
    r0.w = (max(r0.wwww,source[0].xxxx)).w;
    // 72: min r2.z, r0.w, l(1.000000)
    r2.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 73: deriv_rtx_coarse r2.x, r3.w
    r2.x = (ddx_coarse(r3.wwww)).x;
    // 74: deriv_rty_coarse r2.y, r3.w
    r2.y = (ddy_coarse(r3.wwww)).y;
    // 75: add r0.w, r3.w, l(1.000000)
    r0.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: add_sat r7.x, -r4.w, r0.w
    r7.x = (saturate((-(r4.wwww))+(r0.wwww))).x;
    // 77: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 78: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 79: mad_sat r7.y, r0.w, l(0.300000), r2.z
    r7.y = (saturate((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 80: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 81: add r0.w, -r7.y, l(1.000000)
    r0.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 82: max r2.xyz, r6.xyzx, r0.wwww
    r2.xyz = (max(r6.xyzx,r0.wwww)).xyz;
    // 83: add r2.xyz, -r6.xyzx, r2.xyzx
    r2.xyz = ((-(r6.xyzx))+(r2.xyzx)).xyz;
    // 84: mul_sat r0.w, r6.y, l(50.000000)
    r0.w = (saturate((r6.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 85: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 86: sample_indexable(texture2d)(float,float,float,float) r7.zw, r7.xyxx, t4.zwxy, s5
    r7.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 87: mul r8.xyz, r6.xyzx, r7.wwww
    r8.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 88: mad r2.xyz, r2.xyzx, r7.zzzz, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r7.zzzz)+(r8.xyzx)).xyz;
    // 89: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.w
    r0.w = r7.w != 0.f ? 1.f / r7.w : 0.f;
    // 90: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 91: mad r8.xyz, r6.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((r6.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: mad r6.xyz, r0.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r6.xyz = ((r0.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 94: mad r9.xyz, -r2.xyzx, r8.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r2.xyzx))*(r8.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 95: mul r2.xyz, r2.xyzx, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r8.xyzx)).xyz;
    // 96: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 97: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 98: mul r8.xyz, r0.wwww, v1.xyzx
    r8.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 99: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 100: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 101: mul r10.xyz, r0.wwww, v0.xyzx
    r10.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 102: mul r11.xyz, r8.zxyz, r10.yzxy
    r11.xyz = ((r8.zxyz)*(r10.yzxy)).xyz;
    // 103: mad r11.xyz, r8.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r8.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 104: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 105: dp3 r12.y, r11.xyzx, r3.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 106: dp3 r11.y, r11.xyzx, r5.xyzx
    r11.y = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 107: dp3 r12.x, r10.xyzx, r3.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 108: dp3 r11.x, r10.xyzx, r5.xyzx
    r11.x = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 109: dp2 r10.z, r12.xyxx, cb0[11].xyxx
    r10.z = (dot((r12.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 110: mul r7.zw, cb0[11].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r7.zw = ((source[11].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 111: dp2 r10.x, r12.xyxx, r7.zwzz
    r10.x = (dot((r12.xyxx).xy,(r7.zwzz).xy).xxxx).x;
    // 112: dp2 r13.x, r11.xyxx, r7.zwzz
    r13.x = (dot((r11.xyxx).xy,(r7.zwzz).xy).xxxx).x;
    // 113: dp2 r13.z, r11.xyxx, cb0[11].xyxx
    r13.z = (dot((r11.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 114: dp3 r10.y, r8.xyzx, r3.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 115: dp3 r13.y, r8.xyzx, r5.xyzx
    r13.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 116: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 117: dp4 r8.x, cb0[12].xyzw, r10.xyzw
    r8.x = (dot((source[12].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 118: dp4 r8.y, cb0[13].xyzw, r10.xyzw
    r8.y = (dot((source[13].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 119: dp4 r8.z, cb0[14].xyzw, r10.xyzw
    r8.z = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 120: mul r11.xyzw, r10.yzzx, r10.xyzz
    r11.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 121: dp4 r14.x, cb0[15].xyzw, r11.xyzw
    r14.x = (dot((source[15].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 122: dp4 r14.y, cb0[16].xyzw, r11.xyzw
    r14.y = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 123: dp4 r14.z, cb0[17].xyzw, r11.xyzw
    r14.z = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 124: add r8.xyz, r8.xyzx, r14.xyzx
    r8.xyz = ((r8.xyzx)+(r14.xyzx)).xyz;
    // 125: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 126: mov r12.z, r10.y
    r12.z = (r10.yyyy).z;
    // 127: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 128: mad r8.xyz, cb0[18].xyzx, r0.wwww, r8.xyzx
    r8.xyz = ((source[18].xyzx)*(r0.wwww)+(r8.xyzx)).xyz;
    // 129: max r8.xyz, r8.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 130: mul r8.xyz, r8.xyzx, cb0[10].xyzx
    r8.xyz = ((r8.xyzx)*(source[10].xyzx)).xyz;
    // 131: mad r8.xyz, r8.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r8.xyz = ((r8.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 132: mul r8.xyz, r9.xyzx, r8.xyzx
    r8.xyz = ((r9.xyzx)*(r8.xyzx)).xyz;
    // 133: mad r9.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 134: mad r10.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 135: mad r11.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 136: log r0.w, |r1.w|
    r0.w = (log2(abs(r1.wwww))).w;
    // 137: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 138: mul r0.w, r0.w, cb0[9].z
    r0.w = ((r0.wwww)*(source[9].zzzz)).w;
    // 139: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 140: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 142: mad r10.xyz, r0.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 143: mad r9.xyz, r10.xyzx, r0.wwww, r9.xyzx
    r9.xyz = ((r10.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 144: mul r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 145: max r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = (max(r0.wwww,r9.xyzx)).xyz;
    // 146: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 147: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 148: mul r10.xyz, r1.wwww, v6.xyzx
    r10.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 149: dp3 r1.w, r10.xyzx, r3.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 150: dp3 r3.x, r10.xyzx, r5.xyzx
    r3.x = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 151: mad r3.xy, r3.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 152: mad r3.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r3.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 153: mul r3.xyzw, r3.xyzw, r3.xyzw
    r3.xyzw = ((r3.xyzw)*(r3.xyzw)).xyzw;
    // 154: mul r5.xyz, r3.wwww, cb0[21].xyzx
    r5.xyz = ((r3.wwww)*(source[21].xyzx)).xyz;
    // 155: mad r5.xyz, r3.zzzz, cb0[20].xyzx, r5.xyzx
    r5.xyz = ((r3.zzzz)*(source[20].xyzx)+(r5.xyzx)).xyz;
    // 156: mul r5.xyz, r5.xyzx, cb0[22].wwww
    r5.xyz = ((r5.xyzx)*(source[22].wwww)).xyz;
    // 157: mul r5.xyz, r0.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 158: mul r5.xyz, r9.xyzx, r5.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)).xyz;
    // 159: mul r5.xyz, r8.xyzx, r5.xyzx
    r5.xyz = ((r8.xyzx)*(r5.xyzx)).xyz;
    // 160: mad r5.xyz, -r5.xyzx, r2.wwww, r5.xyzx
    r5.xyz = ((-(r5.xyzx))*(r2.wwww)+(r5.xyzx)).xyz;
    // 161: add r1.w, r0.w, r7.x
    r1.w = ((r0.wwww)+(r7.xxxx)).w;
    // 162: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 163: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 164: mul r3.z, r7.y, l(5.000000)
    r3.z = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 165: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r13.xyzx, t5.xyzw, s4, r3.z
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r13.xyzx).xyz, (r3.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 166: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 167: mul r7.xyz, r7.xyzx, cb0[10].xyzx
    r7.xyz = ((r7.xyzx)*(source[10].xyzx)).xyz;
    // 168: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 169: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 170: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 171: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 172: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 173: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 174: mad r1.w, r0.w, r6.x, r6.y
    r1.w = ((r0.wwww)*(r6.xxxx)+(r6.yyyy)).w;
    // 175: mad r1.w, r1.w, r0.w, r6.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r6.zzzz)).w;
    // 176: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 177: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 178: mul r3.yzw, r3.yyyy, cb0[21].xxyz
    r3.yzw = ((r3.yyyy)*(source[21].xxyz)).yzw;
    // 179: mad r3.xyz, cb0[20].xyzx, r3.xxxx, r3.yzwy
    r3.xyz = ((source[20].xyzx)*(r3.xxxx)+(r3.yzwy)).xyz;
    // 180: mul r3.xyz, r3.xyzx, cb0[22].wwww
    r3.xyz = ((r3.xyzx)*(source[22].wwww)).xyz;
    // 181: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 182: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 183: mad r5.xyz, r3.xyzx, r2.xyzx, r5.xyzx
    r5.xyz = ((r3.xyzx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 184: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 185: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 186: dp3 r0.w, r1.xyzx, r4.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 187: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 188: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 190: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 191: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 192: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 193: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 194: mul r1.yzw, r0.wwww, cb0[2].xxyz
    r1.yzw = ((r0.wwww)*(source[2].xxyz)).yzw;
    // 195: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 196: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 197: add r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)+(r1.xyzx)).xyz;
    // 198: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 199: mad o0.xyz, r0.xyzx, cb0[22].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[22].xyzx)+(r1.xyzx)).xyz;
    // 200: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 201: dp3 r0.x, r12.xyzx, r12.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 202: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 203: mul r0.xyz, r0.xxxx, r12.xyzx
    r0.xyz = ((r0.xxxx)*(r12.xyzx)).xyz;
    // 204: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 205: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 206: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 207: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 208: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 209: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 210: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 211: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 212: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 213: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 214: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 215: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 216: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 217: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 218: ret
    return output;
}

// source.character.static-map-native-1139.v1 / source program ae8c57346842b047bfd989407627aeae
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1139(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[9]=g_SourceCharacterEnvironmentColor;source[10]=g_SourceCharacterEnvironmentRotation;}
    source[22]=1.f;
    source[23]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 3: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 4: mad r1.xyz, cb0[6].xxxx, r1.xyzx, r0.xyzx
    r1.xyz = ((source[6].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r2.xyz, cb0[4].xyzx, cb0[5].zzzz
    r2.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // 6: mul r3.xyz, cb0[3].xyzx, cb0[5].yyyy
    r3.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 7: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r1.xyz, r2.xyzx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[6].y
    r0.w = ((r2.zzzz)*(source[6].yyyy)).w;
    // 11: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 12: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.w, r1.w, cb0[6].z
    r1.w = ((r1.wwww)*(source[6].zzzz)).w;
    // 14: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 15: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 16: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 18: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 19: mul r1.xyz, r0.xyzx, cb0[6].wwww
    r1.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 20: mad r0.xyz, cb0[7].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[7].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 21: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 25: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 26: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 27: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 28: mul r1.w, r2.x, cb0[8].y
    r1.w = ((r2.xxxx)*(source[8].yyyy)).w;
    // 29: mul r2.x, r2.y, cb0[7].w
    r2.x = ((r2.yyyy)*(source[7].wwww)).x;
    // 30: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 31: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 32: mul r2.y, r2.y, cb0[8].z
    r2.y = ((r2.yyyy)*(source[8].zzzz)).y;
    // 33: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 34: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 36: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 37: mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 38: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 39: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 40: dp3 r2.y, v6.xyzx, v6.xyzx
    r2.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 41: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 42: mul r3.xyz, r2.yyyy, v6.xyzx
    r3.xyz = ((r2.yyyy)*(v6.xyzx)).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 44: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: dp2 r2.y, r4.xyxx, r4.xyxx
    r2.y = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 46: mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // 47: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 48: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 51: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 53: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 54: div r4.xyz, r4.xyzx, r2.yyyy
    r4.xyz = ((r4.xyzx)/(r2.yyyy)).xyz;
    // 55: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 56: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 57: mul r5.xyz, r2.yyyy, r4.xyzx
    r5.xyz = ((r2.yyyy)*(r4.xyzx)).xyz;
    // 58: dp3 r2.y, r3.xyzx, r5.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 59: mad r3.xy, r2.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 61: mul r3.yzw, r3.yyyy, cb0[20].xxyz
    r3.yzw = ((r3.yyyy)*(source[20].xxyz)).yzw;
    // 62: mad r3.xyz, r3.xxxx, cb0[19].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[19].xyzx)+(r3.yzwy)).xyz;
    // 63: mul r3.xyz, r3.xyzx, cb0[21].wwww
    r3.xyz = ((r3.xyzx)*(source[21].wwww)).xyz;
    // 64: mul r6.xyz, r0.xyzx, r3.xyzx
    r6.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 65: dp2_sat r7.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 66: dp3_sat r7.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 67: dp3_sat r7.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 68: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 69: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s3
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 70: mul r8.xyz, r8.xyzx, cb0[23].xyzx
    r8.xyz = ((r8.xyzx)*(source[23].xyzx)).xyz;
    // 71: dp3 r2.y, r8.xyzx, r7.xyzx
    r2.y = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 72: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t5.xyzw, s3
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 73: mul r7.xyz, r7.xyzx, cb0[22].xyzx
    r7.xyz = ((r7.xyzx)*(source[22].xyzx)).xyz;
    // 74: mul r9.xyz, r2.yyyy, r7.xyzx
    r9.xyz = ((r2.yyyy)*(r7.xyzx)).xyz;
    // 75: mad r6.xyz, r0.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 76: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 77: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 78: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 79: mul r6.xyz, r3.wwww, v1.xyzx
    r6.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 80: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 81: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 82: mul r9.xyz, r3.wwww, v0.xyzx
    r9.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 83: mul r10.xyz, r6.zxyz, r9.yzxy
    r10.xyz = ((r6.zxyz)*(r9.yzxy)).xyz;
    // 84: mad r10.xyz, r6.yzxy, r9.zxyz, -r10.xyzx
    r10.xyz = ((r6.yzxy)*(r9.zxyz)+(-(r10.xyzx))).xyz;
    // 85: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 86: dp3 r11.y, r10.xyzx, r5.xyzx
    r11.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 87: dp3 r11.x, r9.xyzx, r5.xyzx
    r11.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 88: dp2 r12.z, r11.xyxx, cb0[10].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[10].xyxx).xy).xxxx).z;
    // 89: mul r13.xy, cb0[10].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r13.xy = ((source[10].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 90: dp2 r12.x, r11.xyxx, r13.xyxx
    r12.x = (dot((r11.xyxx).xy,(r13.xyxx).xy).xxxx).x;
    // 91: dp3 r12.y, r6.xyzx, r5.xyzx
    r12.y = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 92: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 93: dp4 r14.x, cb0[11].xyzw, r12.xyzw
    r14.x = (dot((source[11].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 94: dp4 r14.y, cb0[12].xyzw, r12.xyzw
    r14.y = (dot((source[12].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 95: dp4 r14.z, cb0[13].xyzw, r12.xyzw
    r14.z = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 96: mul r15.xyzw, r12.yzzx, r12.xyzz
    r15.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 97: dp4 r16.x, cb0[14].xyzw, r15.xyzw
    r16.x = (dot((source[14].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 98: dp4 r16.y, cb0[15].xyzw, r15.xyzw
    r16.y = (dot((source[15].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 99: dp4 r16.z, cb0[16].xyzw, r15.xyzw
    r16.z = (dot((source[16].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 100: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 101: mul r3.w, r12.y, r12.y
    r3.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 102: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 103: mad r3.w, r12.x, r12.x, -r3.w
    r3.w = ((r12.xxxx)*(r12.xxxx)+(-(r3.wwww))).w;
    // 104: mad r12.xyz, cb0[17].xyzx, r3.wwww, r14.xyzx
    r12.xyz = ((source[17].xyzx)*(r3.wwww)+(r14.xyzx)).xyz;
    // 105: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 106: mul r12.xyz, r12.xyzx, cb0[9].xyzx
    r12.xyz = ((r12.xyzx)*(source[9].xyzx)).xyz;
    // 107: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[9].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[9].wwww)).xyz;
    // 108: log r3.w, |r2.x|
    r3.w = (log2(abs(r2.xxxx))).w;
    // 109: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 110: mul r3.w, r3.w, cb0[8].x
    r3.w = ((r3.wwww)*(source[8].xxxx)).w;
    // 111: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 112: movc r2.x, r2.x, l(0), r3.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 113: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 114: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 115: dp3 r2.x, v5.xyzx, v5.xyzx
    r2.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 116: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 117: mul r14.xyz, r2.xxxx, v5.xyzx
    r14.xyz = ((r2.xxxx)*(v5.xyzx)).xyz;
    // 118: dp3 r2.x, r5.xyzx, r14.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 119: mul r5.xyz, r2.xxxx, r5.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 120: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 121: deriv_rtx_coarse r15.x, r2.x
    r15.x = (ddx_coarse(r2.xxxx)).x;
    // 122: deriv_rty_coarse r15.y, r2.x
    r15.y = (ddy_coarse(r2.xxxx)).y;
    // 123: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 124: dp2 r3.w, r15.xyxx, r15.xyxx
    r3.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 125: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 126: mad_sat r15.y, r3.w, l(0.300000), r2.z
    r15.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 127: add r3.w, -r15.y, l(1.000000)
    r3.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mov_sat r0.w, cb0[7].y
    r0.w = (saturate(source[7].yyyy)).w;
    // 129: mad r16.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 130: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 131: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 132: mad r16.xyz, r2.wwww, r16.xyzx, r4.wwww
    r16.xyz = ((r2.wwww)*(r16.xyzx)+(r4.wwww)).xyz;
    // 133: max r17.xyz, r3.wwww, r16.xyzx
    r17.xyz = (max(r3.wwww,r16.xyzx)).xyz;
    // 134: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 135: mul_sat r0.w, r16.y, l(50.000000)
    r0.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 136: mul r17.xyz, r0.wwww, r17.xyzx
    r17.xyz = ((r0.wwww)*(r17.xyzx)).xyz;
    // 137: add r0.w, r5.z, l(1.000000)
    r0.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: add_sat r15.x, -r0.w, r2.x
    r15.x = (saturate((-(r0.wwww))+(r2.xxxx))).x;
    // 140: sample_indexable(texture2d)(float,float,float,float) r13.zw, r15.xyxx, t3.zwxy, s5
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 141: add r0.w, r1.w, r15.x
    r0.w = ((r1.wwww)+(r15.xxxx)).w;
    // 142: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 143: mul r15.xzw, r13.wwww, r16.xxyz
    r15.xzw = ((r13.wwww)*(r16.xxyz)).xzw;
    // 144: mad r15.xzw, r17.xxyz, r13.zzzz, r15.xxzw
    r15.xzw = ((r17.xxyz)*(r13.zzzz)+(r15.xxzw)).xzw;
    // 145: div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.x = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 146: add r2.x, r2.x, l(-1.000000)
    r2.x = ((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 147: mad r17.xyz, r16.xyzx, r2.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 148: dp3 r2.x, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 149: mad r16.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 150: mad r18.xyz, -r15.xzwx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xzwx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r15.xzw, r15.xxzw, r17.xxyz
    r15.xzw = ((r15.xxzw)*(r17.xxyz)).xzw;
    // 152: mul r12.xyz, r12.xyzx, r18.xyzx
    r12.xyz = ((r12.xyzx)*(r18.xyzx)).xyz;
    // 153: mul r1.xyz, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.xyzx)*(r12.xyzx)).xyz;
    // 154: mad r1.xyz, -r1.xyzx, r2.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r2.wwww)+(r1.xyzx)).xyz;
    // 155: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 156: dp2_sat r12.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r12.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 157: dp3_sat r12.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r12.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 158: dp3_sat r12.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r12.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 159: mul r12.xyz, r12.xyzx, r12.xyzx
    r12.xyz = ((r12.xyzx)*(r12.xyzx)).xyz;
    // 160: dp3 r2.x, r8.xyzx, r12.xyzx
    r2.x = (dot((r8.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 161: add r2.y, -r2.x, r2.y
    r2.y = ((-(r2.xxxx))+(r2.yyyy)).y;
    // 162: mad r2.x, r2.z, r2.y, r2.x
    r2.x = ((r2.zzzz)*(r2.yyyy)+(r2.xxxx)).x;
    // 163: mad r2.yzw, r7.xxyz, r2.xxxx, r3.xxyz
    r2.yzw = ((r7.xxyz)*(r2.xxxx)+(r3.xxyz)).yzw;
    // 164: mul r3.xyz, r2.xxxx, r7.xyzx
    r3.xyz = ((r2.xxxx)*(r7.xyzx)).xyz;
    // 165: mul r2.x, r15.y, r15.y
    r2.x = ((r15.yyyy)*(r15.yyyy)).x;
    // 166: mul r3.w, r15.y, l(5.000000)
    r3.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 167: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 168: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 169: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 170: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 171: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 172: mad r1.w, r0.w, r16.x, r16.y
    r1.w = ((r0.wwww)*(r16.xxxx)+(r16.yyyy)).w;
    // 173: mad r1.w, r1.w, r0.w, r16.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r16.zzzz)).w;
    // 174: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 175: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 176: mul r7.xyz, r0.wwww, r2.yzwy
    r7.xyz = ((r0.wwww)*(r2.yzwy)).xyz;
    // 177: add r2.xyz, r2.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 178: div r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)/(r2.xyzx)).xyz;
    // 179: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 180: dp3 r2.x, r9.xyzx, r5.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 181: dp3 r2.y, r10.xyzx, r5.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 182: dp3 r3.y, r6.xyzx, r5.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 183: dp2 r3.x, r2.xyxx, r13.xyxx
    r3.x = (dot((r2.xyxx).xy,(r13.xyxx).xy).xxxx).x;
    // 184: dp2 r3.z, r2.xyxx, cb0[10].xyxx
    r3.z = (dot((r2.xyxx).xy,(source[10].xyxx).xy).xxxx).z;
    // 185: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r3.xyzx, t4.xyzw, s4, r3.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 186: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 187: mul r2.xyz, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((r2.xyzx)*(source[9].xyzx)).xyz;
    // 188: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[9].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[9].wwww)).xyz;
    // 189: mul r2.xyz, r7.xyzx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 190: mad r1.xyz, r2.xyzx, r15.xzwx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r15.xzwx)+(r1.xyzx)).xyz;
    // 191: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 192: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 193: dp3 r1.w, r4.xyzx, r14.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 194: add r2.x, -|r14.z|, l(1.000000)
    r2.x = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 195: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 196: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 197: lt r2.x, |r1.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 198: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 199: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 200: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 201: mul r2.yzw, r1.wwww, cb0[2].xxyz
    r2.yzw = ((r1.wwww)*(source[2].xxyz)).yzw;
    // 202: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 203: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 204: add r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 205: mad o0.xyz, r0.xyzx, cb0[21].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)+(r2.xyzx)).xyz;
    // 206: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 207: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 208: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 209: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 210: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 211: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 212: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 213: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 214: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 215: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 216: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 217: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 218: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 219: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 220: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 221: ftou r0.x, cb0[18].z
    r0.x = (asfloat((uint4)(source[18].zzzz))).x;
    // 222: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 223: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 224: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 225: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 226: ret
    return output;
}

// source.character.static-map-native-1139.v1 / source program 8ee5cf7464edfa42906a9ec2e1560a3b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1139(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1139(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[9]=g_SourceCharacterEnvironmentColor;source[10]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 3: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 4: mad r1.xyz, cb0[6].xxxx, r1.xyzx, r0.xyzx
    r1.xyz = ((source[6].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r2.xyz, cb0[4].xyzx, cb0[5].zzzz
    r2.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // 6: mul r3.xyz, cb0[3].xyzx, cb0[5].yyyy
    r3.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 7: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r1.xyz, r2.xyzx, r1.xyzx, -r0.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r0.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[6].y
    r0.w = ((r2.zzzz)*(source[6].yyyy)).w;
    // 11: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 12: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 13: mul r1.w, r1.w, cb0[6].z
    r1.w = ((r1.wwww)*(source[6].zzzz)).w;
    // 14: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 15: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 16: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 18: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 19: mul r1.xyz, r0.xyzx, cb0[6].wwww
    r1.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 20: mad r0.xyz, cb0[7].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[7].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 21: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 25: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 26: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 27: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 28: mul r1.w, r2.x, cb0[8].y
    r1.w = ((r2.xxxx)*(source[8].yyyy)).w;
    // 29: mul r2.x, r2.y, cb0[7].w
    r2.x = ((r2.yyyy)*(source[7].wwww)).x;
    // 30: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 31: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 32: mul r2.y, r2.y, cb0[8].z
    r2.y = ((r2.yyyy)*(source[8].zzzz)).y;
    // 33: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 34: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 36: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 37: mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 38: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 39: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 41: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 42: dp2 r2.y, r3.xyxx, r3.xyxx
    r2.y = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 43: mul r3.xy, r3.xyxx, cb0[5].xxxx
    r3.xy = ((r3.xyxx)*(source[5].xxxx)).xy;
    // 44: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 45: add r2.y, -r2.y, l(1.000000)
    r2.y = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 47: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 48: add r3.z, r2.y, l(0.000010)
    r3.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 49: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 50: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 51: div r3.xyz, r3.xyzx, r2.yyyy
    r3.xyz = ((r3.xyzx)/(r2.yyyy)).xyz;
    // 52: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 53: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 54: mul r4.xyz, r2.yyyy, r3.xyzx
    r4.xyz = ((r2.yyyy)*(r3.xyzx)).xyz;
    // 55: dp3 r2.y, v6.xyzx, v6.xyzx
    r2.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 56: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 57: mul r5.xyz, r2.yyyy, v6.xyzx
    r5.xyz = ((r2.yyyy)*(v6.xyzx)).xyz;
    // 58: dp3 r2.y, r5.xyzx, r4.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 59: mad r6.xy, r2.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 61: mul r6.yzw, r6.yyyy, cb0[20].xxyz
    r6.yzw = ((r6.yyyy)*(source[20].xxyz)).yzw;
    // 62: mad r6.xyz, r6.xxxx, cb0[19].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[19].xyzx)+(r6.yzwy)).xyz;
    // 63: mul r6.xyz, r6.xyzx, cb0[21].wwww
    r6.xyz = ((r6.xyzx)*(source[21].wwww)).xyz;
    // 64: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 65: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 66: mov_sat r0.w, cb0[7].y
    r0.w = (saturate(source[7].yyyy)).w;
    // 67: mad r6.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r6.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 68: mul r2.y, r0.w, l(0.080000)
    r2.y = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 69: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 70: mad r6.xyz, r2.wwww, r6.xyzx, r2.yyyy
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(r2.yyyy)).xyz;
    // 71: log r0.w, |r2.x|
    r0.w = (log2(abs(r2.xxxx))).w;
    // 72: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 73: mul r0.w, r0.w, cb0[8].x
    r0.w = ((r0.wwww)*(source[8].xxxx)).w;
    // 74: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 75: movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 76: max r0.w, r0.w, cb0[0].x
    r0.w = (max(r0.wwww,source[0].xxxx)).w;
    // 77: min r2.z, r0.w, l(1.000000)
    r2.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 78: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 79: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 80: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 81: dp3 r0.w, r4.xyzx, r7.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 82: deriv_rtx_coarse r2.x, r0.w
    r2.x = (ddx_coarse(r0.wwww)).x;
    // 83: deriv_rty_coarse r2.y, r0.w
    r2.y = (ddy_coarse(r0.wwww)).y;
    // 84: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 85: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 86: mad_sat r2.y, r2.x, l(0.300000), r2.z
    r2.y = (saturate((r2.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 87: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 88: add r2.z, -r2.y, l(1.000000)
    r2.z = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 89: max r8.xyz, r6.xyzx, r2.zzzz
    r8.xyz = (max(r6.xyzx,r2.zzzz)).xyz;
    // 90: add r8.xyz, -r6.xyzx, r8.xyzx
    r8.xyz = ((-(r6.xyzx))+(r8.xyzx)).xyz;
    // 91: mul_sat r2.z, r6.y, l(50.000000)
    r2.z = (saturate((r6.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 92: mul r8.xyz, r2.zzzz, r8.xyzx
    r8.xyz = ((r2.zzzz)*(r8.xyzx)).xyz;
    // 93: mul r9.xyz, r0.wwww, r4.xyzx
    r9.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 94: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 96: add r2.z, r9.z, l(1.000000)
    r2.z = ((r9.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 97: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: add_sat r2.x, r0.w, -r2.z
    r2.x = (saturate((r0.wwww)+(-(r2.zzzz)))).x;
    // 99: sample_indexable(texture2d)(float,float,float,float) r10.xy, r2.xyxx, t3.xyzw, s4
    r10.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 100: add r0.w, r1.w, r2.x
    r0.w = ((r1.wwww)+(r2.xxxx)).w;
    // 101: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 102: mul r11.xyz, r6.xyzx, r10.yyyy
    r11.xyz = ((r6.xyzx)*(r10.yyyy)).xyz;
    // 103: mad r8.xyz, r8.xyzx, r10.xxxx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xxxx)+(r11.xyzx)).xyz;
    // 104: div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r10.y
    r2.x = r10.y != 0.f ? 1.f / r10.y : 0.f;
    // 105: add r2.x, r2.x, l(-1.000000)
    r2.x = ((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 106: mad r10.xyz, r6.xyzx, r2.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((r6.xyzx)*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 107: dp3 r2.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 108: mad r6.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r6.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 109: mad r11.xyz, -r8.xyzx, r10.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r8.xyzx))*(r10.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 110: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 111: dp3 r2.x, v1.xyzx, v1.xyzx
    r2.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 112: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 113: mul r10.xyz, r2.xxxx, v1.xyzx
    r10.xyz = ((r2.xxxx)*(v1.xyzx)).xyz;
    // 114: dp3 r2.x, v0.xyzx, v0.xyzx
    r2.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 115: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 116: mul r12.xyz, r2.xxxx, v0.xyzx
    r12.xyz = ((r2.xxxx)*(v0.xyzx)).xyz;
    // 117: mul r13.xyz, r10.zxyz, r12.yzxy
    r13.xyz = ((r10.zxyz)*(r12.yzxy)).xyz;
    // 118: mad r13.xyz, r10.yzxy, r12.zxyz, -r13.xyzx
    r13.xyz = ((r10.yzxy)*(r12.zxyz)+(-(r13.xyzx))).xyz;
    // 119: mul r13.xyz, r13.xyzx, v1.wwww
    r13.xyz = ((r13.xyzx)*(v1.wwww)).xyz;
    // 120: dp3 r14.y, r13.xyzx, r4.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 121: dp3 r13.y, r13.xyzx, r9.xyzx
    r13.y = (dot((r13.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 122: dp3 r14.x, r12.xyzx, r4.xyzx
    r14.x = (dot((r12.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 123: dp3 r4.y, r10.xyzx, r4.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 124: dp3 r10.y, r10.xyzx, r9.xyzx
    r10.y = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 125: dp3 r13.x, r12.xyzx, r9.xyzx
    r13.x = (dot((r12.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 126: dp3 r2.x, r5.xyzx, r9.xyzx
    r2.x = (dot((r5.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 127: mad r2.xz, r2.xxxx, l(0.500000, 0.000000, -0.500000, 0.000000), l(0.500000, 0.000000, 0.500000, 0.000000)
    r2.xz = ((r2.xxxx)*(float4(0.500000,0.000000,-0.500000,0.000000))+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 128: mul r2.xz, r2.xxzx, r2.xxzx
    r2.xz = ((r2.xxzx)*(r2.xxzx)).xz;
    // 129: dp2 r4.z, r14.xyxx, cb0[10].xyxx
    r4.z = (dot((r14.xyxx).xy,(source[10].xyxx).xy).xxxx).z;
    // 130: mul r5.xy, cb0[10].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((source[10].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r4.x, r14.xyxx, r5.xyxx
    r4.x = (dot((r14.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 132: dp2 r10.x, r13.xyxx, r5.xyxx
    r10.x = (dot((r13.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 133: dp2 r10.z, r13.xyxx, cb0[10].xyxx
    r10.z = (dot((r13.xyxx).xy,(source[10].xyxx).xy).xxxx).z;
    // 134: mov r4.w, l(1.000000)
    r4.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 135: dp4 r5.x, cb0[11].xyzw, r4.xyzw
    r5.x = (dot((source[11].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).x;
    // 136: dp4 r5.y, cb0[12].xyzw, r4.xyzw
    r5.y = (dot((source[12].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).y;
    // 137: dp4 r5.z, cb0[13].xyzw, r4.xyzw
    r5.z = (dot((source[13].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).z;
    // 138: mul r9.xyzw, r4.yzzx, r4.xyzz
    r9.xyzw = ((r4.yzzx)*(r4.xyzz)).xyzw;
    // 139: dp4 r12.x, cb0[14].xyzw, r9.xyzw
    r12.x = (dot((source[14].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 140: dp4 r12.y, cb0[15].xyzw, r9.xyzw
    r12.y = (dot((source[15].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 141: dp4 r12.z, cb0[16].xyzw, r9.xyzw
    r12.z = (dot((source[16].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 142: add r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)+(r12.xyzx)).xyz;
    // 143: mul r3.w, r4.y, r4.y
    r3.w = ((r4.yyyy)*(r4.yyyy)).w;
    // 144: mov r14.z, r4.y
    r14.z = (r4.yyyy).z;
    // 145: mad r3.w, r4.x, r4.x, -r3.w
    r3.w = ((r4.xxxx)*(r4.xxxx)+(-(r3.wwww))).w;
    // 146: mad r4.xyz, cb0[17].xyzx, r3.wwww, r5.xyzx
    r4.xyz = ((source[17].xyzx)*(r3.wwww)+(r5.xyzx)).xyz;
    // 147: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 148: mul r4.xyz, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((r4.xyzx)*(source[9].xyzx)).xyz;
    // 149: mad r4.xyz, r4.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[9].wwww
    r4.xyz = ((r4.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[9].wwww)).xyz;
    // 150: mul r4.xyz, r11.xyzx, r4.xyzx
    r4.xyz = ((r11.xyzx)*(r4.xyzx)).xyz;
    // 151: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 152: mad r1.xyz, -r1.xyzx, r2.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r2.wwww)+(r1.xyzx)).xyz;
    // 153: mul r2.w, r2.y, r2.y
    r2.w = ((r2.yyyy)*(r2.yyyy)).w;
    // 154: mul r2.y, r2.y, l(5.000000)
    r2.y = ((r2.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 155: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r10.xyzx, t4.xyzw, s3, r2.y
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r10.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 156: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 157: mul r4.xyz, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((r4.xyzx)*(source[9].xyzx)).xyz;
    // 158: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[9].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[9].wwww)).xyz;
    // 159: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 160: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 161: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 162: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 163: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 164: mad r1.w, r0.w, r6.x, r6.y
    r1.w = ((r0.wwww)*(r6.xxxx)+(r6.yyyy)).w;
    // 165: mad r1.w, r1.w, r0.w, r6.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r6.zzzz)).w;
    // 166: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 167: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 168: mul r2.yzw, r2.zzzz, cb0[20].xxyz
    r2.yzw = ((r2.zzzz)*(source[20].xxyz)).yzw;
    // 169: mad r2.xyz, cb0[19].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[19].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 170: mul r2.xyz, r2.xyzx, cb0[21].wwww
    r2.xyz = ((r2.xyzx)*(source[21].wwww)).xyz;
    // 171: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 172: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 173: mad r1.xyz, r2.xyzx, r8.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r8.xyzx)+(r1.xyzx)).xyz;
    // 174: mul r2.xyz, r8.xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 175: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 176: dp3 r0.w, r3.xyzx, r7.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 177: add r1.w, -|r7.z|, l(1.000000)
    r1.w = ((-(abs(r7.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 180: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 181: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 182: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 183: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 184: mul r2.xyz, r0.wwww, cb0[2].xyzx
    r2.xyz = ((r0.wwww)*(source[2].xyzx)).xyz;
    // 185: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 186: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 187: add r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 188: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 189: mad o0.xyz, r0.xyzx, cb0[21].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[21].xyzx)+(r2.xyzx)).xyz;
    // 190: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 191: dp3 r0.x, r14.xyzx, r14.xyzx
    r0.x = (dot((r14.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 192: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 193: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 194: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 195: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 196: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 197: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 198: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 199: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 200: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 201: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 202: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 203: ftou r0.x, cb0[18].z
    r0.x = (asfloat((uint4)(source[18].zzzz))).x;
    // 204: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 205: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 206: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 207: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 208: ret
    return output;
}

// source.character.static-map-native-1140.v1 / source program 54b04e0842b20f49902b422d1bf349bc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1140(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    source[28]=1.f;
    source[29]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f;
    // 1: mul r0.xyz, cb0[8].xyzx, cb0[11].yyyy
    r0.xyz = ((source[8].xyzx)*(source[11].yyyy)).xyz;
    // 2: mul r1.xyz, cb0[7].xyzx, cb0[11].xxxx
    r1.xyz = ((source[7].xyzx)*(source[11].xxxx)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[10].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: add r2.xy, r2.wwww, cb0[13].zxzz
    r2.xy = ((r2.wwww)+(source[13].zxzz)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: mul r0.w, r3.z, cb0[11].z
    r0.w = ((r3.zzzz)*(source[11].zzzz)).w;
    // 12: mul r2.zw, r3.yyyx, cb0[14].xxxz
    r2.zw = ((r3.yyyx)*(source[14].xxxz)).zw;
    // 13: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 14: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 15: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 16: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 17: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 18: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: mul r1.xyz, r0.xyzx, cb0[12].xxxx
    r1.xyz = ((r0.xyzx)*(source[12].xxxx)).xyz;
    // 21: mad r0.xyz, cb0[12].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[12].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 22: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 26: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 27: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 28: log r4.xy, |r2.zwzz|
    r4.xy = (log2(abs(r2.zwzz))).xy;
    // 29: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 30: mul r4.xy, r4.xyxx, cb0[14].ywyy
    r4.xy = ((r4.xyxx)*(source[14].ywyy)).xy;
    // 31: exp r4.xy, r4.xyxx
    r4.xy = (exp2(r4.xyxx)).xy;
    // 32: min r1.w, r4.y, l(1.000000)
    r1.w = (min(r4.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: movc r2.z, r2.z, l(0), r4.x
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).z;
    // 34: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 35: max r2.z, r2.z, cb0[0].x
    r2.z = (max(r2.zzzz,source[0].xxxx)).z;
    // 36: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 38: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 39: mad r1.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 40: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 41: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 42: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 43: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 44: mul r3.xyz, r2.wwww, v7.xyzx
    r3.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 46: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 47: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 48: mul r4.xy, r4.xyxx, cb0[10].xxxx
    r4.xy = ((r4.xyxx)*(source[10].xxxx)).xy;
    // 49: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 50: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 51: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 53: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 54: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r5.xyz, r2.wwww, r4.xyzx
    r5.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 60: dp3 r2.w, r3.xyzx, r5.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 61: dp3 r3.x, -r3.xyzx, r5.xyzx
    r3.x = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).x;
    // 62: mad r3.xy, r3.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 63: mad r3.zw, r2.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r3.zw = ((r2.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 64: mul r3.xyzw, r3.xyzw, r3.xyzw
    r3.xyzw = ((r3.xyzw)*(r3.xyzw)).xyzw;
    // 65: mul r6.xyz, r3.wwww, cb0[26].xyzx
    r6.xyz = ((r3.wwww)*(source[26].xyzx)).xyz;
    // 66: mad r6.xyz, r3.zzzz, cb0[25].xyzx, r6.xyzx
    r6.xyz = ((r3.zzzz)*(source[25].xyzx)+(r6.xyzx)).xyz;
    // 67: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 68: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 69: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 70: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 71: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 72: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 73: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t9.xyzw, s6
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 74: mul r9.xyz, r9.xyzx, cb0[29].xyzx
    r9.xyz = ((r9.xyzx)*(source[29].xyzx)).xyz;
    // 75: dp3 r2.w, r9.xyzx, r8.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 76: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s6
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 77: mul r8.xyz, r8.xyzx, cb0[28].xyzx
    r8.xyz = ((r8.xyzx)*(source[28].xyzx)).xyz;
    // 78: mul r10.xyz, r2.wwww, r8.xyzx
    r10.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 79: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 80: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 81: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 82: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 83: mul r10.xyz, r3.zzzz, v1.xyzx
    r10.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 84: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 85: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 86: mul r11.xyz, r3.zzzz, v0.xyzx
    r11.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 87: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 88: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 89: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 90: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 91: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 92: dp2 r14.z, r13.xyxx, cb0[17].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 93: mul r3.zw, cb0[17].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[17].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 94: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 95: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 96: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 97: dp4 r13.x, cb0[18].xyzw, r14.xyzw
    r13.x = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 98: dp4 r13.y, cb0[19].xyzw, r14.xyzw
    r13.y = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 99: dp4 r13.z, cb0[20].xyzw, r14.xyzw
    r13.z = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 100: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 101: mul r4.w, r14.y, r14.y
    r4.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 102: mad r4.w, r14.x, r14.x, -r4.w
    r4.w = ((r14.xxxx)*(r14.xxxx)+(-(r4.wwww))).w;
    // 103: dp4 r14.x, cb0[21].xyzw, r15.xyzw
    r14.x = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 104: dp4 r14.y, cb0[22].xyzw, r15.xyzw
    r14.y = (dot((source[22].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 105: dp4 r14.z, cb0[23].xyzw, r15.xyzw
    r14.z = (dot((source[23].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 106: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 107: mad r13.xyz, cb0[24].xyzx, r4.wwww, r13.xyzx
    r13.xyz = ((source[24].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 108: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 109: mul r13.xyz, r13.xyzx, cb0[16].xyzx
    r13.xyz = ((r13.xyzx)*(source[16].xyzx)).xyz;
    // 110: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 111: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 112: dp3 r5.w, v6.xyzx, v6.xyzx
    r5.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 113: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 114: mul r14.xyz, r5.wwww, v6.xyzx
    r14.xyz = ((r5.wwww)*(v6.xyzx)).xyz;
    // 115: dp3 r5.w, r5.xyzx, r14.xyzx
    r5.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 116: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 117: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 118: deriv_rtx_coarse r15.x, r5.w
    r15.x = (ddx_coarse(r5.wwww)).x;
    // 119: deriv_rty_coarse r15.y, r5.w
    r15.y = (ddy_coarse(r5.wwww)).y;
    // 120: dp2 r6.w, r15.xyxx, r15.xyxx
    r6.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 121: sqrt r6.w, r6.w
    r6.w = (sqrt(r6.wwww)).w;
    // 122: mad_sat r15.y, r6.w, l(0.300000), r2.z
    r15.y = (saturate((r6.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 123: mad r6.w, r15.y, l(0.200000), l(0.200000)
    r6.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 124: div r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)/(r6.wwww)).w;
    // 125: dp3 r7.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r7.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 126: mad r4.w, r7.w, l(5.000000), r4.w
    r4.w = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r8.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 128: mad r2.x, r8.w, -r2.x, r2.x
    r2.x = ((r8.wwww)*(-(r2.xxxx))+(r2.xxxx)).x;
    // 129: add_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)+(r2.xxxx))).w;
    // 130: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 131: add_sat r2.x, r0.w, r4.w
    r2.x = (saturate((r0.wwww)+(r4.wwww))).x;
    // 132: mad r4.w, r2.x, l(-2.000000), l(3.000000)
    r4.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 133: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 134: mul r2.x, r2.x, r4.w
    r2.x = ((r2.xxxx)*(r4.wwww)).x;
    // 135: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 136: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 137: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 138: mul r13.xyz, r2.xxxx, r13.xyzx
    r13.xyz = ((r2.xxxx)*(r13.xyzx)).xyz;
    // 139: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 140: add r2.x, -r2.y, l(1.000000)
    r2.x = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 141: mad_sat r2.x, r8.w, r2.x, r2.y
    r2.x = (saturate((r8.wwww)*(r2.xxxx)+(r2.yyyy))).x;
    // 142: mad r2.x, -r2.x, cb0[2].x, l(1.000000)
    r2.x = ((-(r2.xxxx))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 144: mad r4.w, r2.y, l(0.350000), l(1.000000)
    r4.w = ((r2.yyyy)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: div_sat r2.x, r2.x, r4.w
    r2.x = (saturate((r2.xxxx)/(r4.wwww))).x;
    // 146: add r4.w, r5.w, l(1.000000)
    r4.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mov_sat r5.w, r5.w
    r5.w = (saturate(r5.wwww)).w;
    // 148: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 149: mul r5.w, r5.w, cb0[1].y
    r5.w = ((r5.wwww)*(source[1].yyyy)).w;
    // 150: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 151: mad_sat r5.w, r5.w, cb0[1].w, cb0[1].z
    r5.w = (saturate((r5.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 152: add r8.w, r5.z, l(1.000000)
    r8.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: add_sat r15.x, r4.w, -r8.w
    r15.x = (saturate((r4.wwww)+(-(r8.wwww)))).x;
    // 155: add r15.zw, -r15.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r15.zw = ((-(r15.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 156: mov_sat r4.w, cb0[12].z
    r4.w = (saturate(source[12].zzzz)).w;
    // 157: mad r16.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 158: mul r4.w, r4.w, l(0.080000)
    r4.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 159: mad r16.xyz, r0.wwww, r16.xyzx, r4.wwww
    r16.xyz = ((r0.wwww)*(r16.xyzx)+(r4.wwww)).xyz;
    // 160: max r17.xyz, r15.zzzz, r16.xyzx
    r17.xyz = (max(r15.zzzz,r16.xyzx)).xyz;
    // 161: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 162: mul_sat r4.w, r16.y, l(50.000000)
    r4.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 163: mul r17.xyz, r4.wwww, r17.xyzx
    r17.xyz = ((r4.wwww)*(r17.xyzx)).xyz;
    // 164: sample_indexable(texture2d)(float,float,float,float) r18.xy, r15.xyxx, t6.xyzw, s8
    r18.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 165: add r8.w, r1.w, r15.x
    r8.w = ((r1.wwww)+(r15.xxxx)).w;
    // 166: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 167: mul r2.y, r2.y, r8.w
    r2.y = ((r2.yyyy)*(r8.wwww)).y;
    // 168: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 169: add r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)+(r2.yyyy)).w;
    // 170: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 171: mul r2.y, r15.y, l(5.000000)
    r2.y = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 172: mul r15.xyz, r16.xyzx, r18.yyyy
    r15.xyz = ((r16.xyzx)*(r18.yyyy)).xyz;
    // 173: mad r15.xyz, r17.xyzx, r18.xxxx, r15.xyzx
    r15.xyz = ((r17.xyzx)*(r18.xxxx)+(r15.xyzx)).xyz;
    // 174: div r8.w, l(1.000000, 1.000000, 1.000000, 1.000000), r18.y
    r8.w = r18.y != 0.f ? 1.f / r18.y : 0.f;
    // 175: add r8.w, r8.w, l(-1.000000)
    r8.w = ((r8.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 176: mad r17.xyz, r16.xyzx, r8.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r8.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 179: mul r8.w, r15.w, r15.w
    r8.w = ((r15.wwww)*(r15.wwww)).w;
    // 180: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 181: mul r9.w, r15.w, r8.w
    r9.w = ((r15.wwww)*(r8.wwww)).w;
    // 182: mad r8.w, -r8.w, r15.w, l(1.000000)
    r8.w = ((-(r8.wwww))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: mul r17.xyz, r16.xyzx, r8.wwww
    r17.xyz = ((r16.xyzx)*(r8.wwww)).xyz;
    // 184: dp3 r8.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 185: mad r16.xyz, r8.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r8.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r17.xyz, r4.wwww, r9.wwww, r17.xyzx
    r17.xyz = ((r4.wwww)*(r9.wwww)+(r17.xyzx)).xyz;
    // 187: add r17.xyz, -r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r17.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 189: mad r19.xyz, -r2.xxxx, r17.xyzx, r18.xyzx
    r19.xyz = ((-(r2.xxxx))*(r17.xyzx)+(r18.xyzx)).xyz;
    // 190: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 191: mul r17.xyz, r2.xxxx, r17.xyzx
    r17.xyz = ((r2.xxxx)*(r17.xyzx)).xyz;
    // 192: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 193: mad r17.xyz, -r17.xyzx, r0.wwww, r17.xyzx
    r17.xyz = ((-(r17.xyzx))*(r0.wwww)+(r17.xyzx)).xyz;
    // 194: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 195: mad r7.xyz, -r7.xyzx, r0.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.wwww)+(r7.xyzx)).xyz;
    // 196: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 197: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 198: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 199: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 200: dp3 r4.w, r9.xyzx, r19.xyzx
    r4.w = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).w;
    // 201: add r2.w, r2.w, -r4.w
    r2.w = ((r2.wwww)+(-(r4.wwww))).w;
    // 202: mad r2.z, r2.z, r2.w, r4.w
    r2.z = ((r2.zzzz)*(r2.wwww)+(r4.wwww)).z;
    // 203: mad r6.xyz, r8.xyzx, r2.zzzz, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r2.zzzz)+(r6.xyzx)).xyz;
    // 204: mad r2.z, r1.w, r16.x, r16.y
    r2.z = ((r1.wwww)*(r16.xxxx)+(r16.yyyy)).z;
    // 205: mad r2.z, r2.z, r1.w, r16.z
    r2.z = ((r2.zzzz)*(r1.wwww)+(r16.zzzz)).z;
    // 206: mul r2.z, r1.w, r2.z
    r2.z = ((r1.wwww)*(r2.zzzz)).z;
    // 207: max r1.w, r1.w, r2.z
    r1.w = (max(r1.wwww,r2.zzzz)).w;
    // 208: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 209: dp3 r11.x, r11.xyzx, r5.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 210: dp3 r11.y, r12.xyzx, r5.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 211: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 212: dp2 r5.x, r11.xyxx, r3.zwzz
    r5.x = (dot((r11.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 213: dp2 r5.z, r11.xyxx, cb0[17].xyxx
    r5.z = (dot((r11.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 214: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t7.xyzw, s7, r2.y
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 215: mul r2.yzw, r10.xxyz, r10.wwww
    r2.yzw = ((r10.xxyz)*(r10.wwww)).yzw;
    // 216: mul r2.yzw, r2.yyzw, cb0[16].xxyz
    r2.yzw = ((r2.yyzw)*(source[16].xxyz)).yzw;
    // 217: mad r2.yzw, r2.yyzw, l(0.000000, 6.000000, 6.000000, 6.000000), cb0[16].wwww
    r2.yzw = ((r2.yyzw)*(float4(0.000000,6.000000,6.000000,6.000000))+(source[16].wwww)).yzw;
    // 218: dp3 r3.z, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 219: div r3.z, r3.z, r6.w
    r3.z = ((r3.zzzz)/(r6.wwww)).z;
    // 220: mad r3.z, r7.w, l(5.000000), r3.z
    r3.z = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.zzzz)).z;
    // 221: add_sat r3.z, r0.w, r3.z
    r3.z = (saturate((r0.wwww)+(r3.zzzz))).z;
    // 222: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mad r3.w, r3.z, l(-2.000000), l(3.000000)
    r3.w = ((r3.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 224: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 225: mul r3.z, r3.z, r3.w
    r3.z = ((r3.zzzz)*(r3.wwww)).z;
    // 226: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 227: mul r3.z, r3.z, l(1.500000)
    r3.z = ((r3.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 228: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 229: mul r2.yzw, r2.yyzw, r3.zzzz
    r2.yzw = ((r2.yyzw)*(r3.zzzz)).yzw;
    // 230: mul r5.xyz, r6.xyzx, r2.yzwy
    r5.xyz = ((r6.xyzx)*(r2.yzwy)).xyz;
    // 231: mul r2.yzw, r2.yyzw, r15.xxyz
    r2.yzw = ((r2.yyzw)*(r15.xxyz)).yzw;
    // 232: mad r5.xyz, r5.xyzx, r15.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)+(r7.xyzx)).xyz;
    // 233: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 234: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 235: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 236: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 237: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 238: add_sat r6.xyz, r6.xyzx, cb0[15].yyyy
    r6.xyz = (saturate((r6.xyzx)+(source[15].yyyy))).xyz;
    // 239: mul r3.w, r6.x, cb0[15].z
    r3.w = ((r6.xxxx)*(source[15].zzzz)).w;
    // 240: mul_sat r6.xyz, r6.xyzx, cb0[9].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[9].xyzx))).xyz;
    // 241: mul r3.w, r3.w, r5.w
    r3.w = ((r3.wwww)*(r5.wwww)).w;
    // 242: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 243: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 244: mul r7.xyz, r0.wwww, r18.xyzx
    r7.xyz = ((r0.wwww)*(r18.xyzx)).xyz;
    // 245: mul r7.xyz, r13.xyzx, r7.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)).xyz;
    // 246: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 247: mad r1.xyz, r2.yzwy, r1.wwww, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r1.wwww)+(r1.xyzx)).xyz;
    // 248: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 249: mul r2.yzw, r3.xxyz, r6.xxyz
    r2.yzw = ((r3.xxyz)*(r6.xxyz)).yzw;
    // 250: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 251: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 252: mul r3.xyz, r0.wwww, r8.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 253: mul r2.yzw, r0.xxyz, r2.yyzw
    r2.yzw = ((r0.xxyz)*(r2.yyzw)).yzw;
    // 254: mad r0.xyz, r0.xyzx, r3.xyzx, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r2.yzwy)).xyz;
    // 255: dp3 r0.w, r4.xyzx, r14.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 256: add r1.w, -|r14.z|, l(1.000000)
    r1.w = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 257: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 258: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 259: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 260: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 261: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 262: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 263: mul r2.yzw, r0.wwww, cb0[4].xxyz
    r2.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 264: movc r2.yzw, r1.wwww, l(0,0,0,0), r2.yyzw
    r2.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyzw)).yzw;
    // 265: mul r3.xy, v4.xyxx, cb0[5].xyxx
    r3.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 266: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 267: mul r4.xyz, cb0[6].xyzx, cb0[10].yyyy
    r4.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 268: mad r2.yzw, r3.xxyz, r4.xxyz, r2.yyzw
    r2.yzw = ((r3.xxyz)*(r4.xxyz)+(r2.yyzw)).yzw;
    // 269: add r2.yzw, r2.yyzw, cb0[3].xxyz
    r2.yzw = ((r2.yyzw)+(source[3].xxyz)).yzw;
    // 270: add r0.w, -r2.x, l(1.000000)
    r0.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mad r2.yzw, r1.xxyz, r0.wwww, r2.yyzw
    r2.yzw = ((r1.xxyz)*(r0.wwww)+(r2.yyzw)).yzw;
    // 272: mul r1.xyz, r2.xxxx, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)).xyz;
    // 273: mul r3.xyz, r2.xxxx, r0.xyzx
    r3.xyz = ((r2.xxxx)*(r0.xyzx)).xyz;
    // 274: mad r0.xyz, r0.xyzx, r0.wwww, r2.yzwy
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r2.yzwy)).xyz;
    // 275: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 276: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 277: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r17.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r17.xyzx)).xyz;
    // 278: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 279: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 280: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 281: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 282: ret
    return output;
}

// source.character.static-map-native-1140.v1 / source program aa830b3a7122af49813f1e13f104ba6c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1140(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1140(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: mul r0.xyz, cb0[8].xyzx, cb0[11].yyyy
    r0.xyz = ((source[8].xyzx)*(source[11].yyyy)).xyz;
    // 2: mul r1.xyz, cb0[7].xyzx, cb0[11].xxxx
    r1.xyz = ((source[7].xyzx)*(source[11].xxxx)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[10].wwww, r3.xyzx, r2.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: add r2.xy, r2.wwww, cb0[13].zxzz
    r2.xy = ((r2.wwww)+(source[13].zxzz)).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: mul r0.w, r3.z, cb0[11].z
    r0.w = ((r3.zzzz)*(source[11].zzzz)).w;
    // 12: mul r2.zw, r3.yyyx, cb0[14].xxxz
    r2.zw = ((r3.yyyx)*(source[14].xxxz)).zw;
    // 13: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 14: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 15: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 16: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 17: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 18: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: mul r1.xyz, r0.xyzx, cb0[12].xxxx
    r1.xyz = ((r0.xyzx)*(source[12].xxxx)).xyz;
    // 21: mad r0.xyz, cb0[12].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[12].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 22: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 25: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 26: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 27: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 28: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 29: log r5.xy, |r2.zwzz|
    r5.xy = (log2(abs(r2.zwzz))).xy;
    // 30: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 31: mul r5.xy, r5.xyxx, cb0[14].ywyy
    r5.xy = ((r5.xyxx)*(source[14].ywyy)).xy;
    // 32: exp r5.xy, r5.xyxx
    r5.xy = (exp2(r5.xyxx)).xy;
    // 33: min r1.w, r5.y, l(1.000000)
    r1.w = (min(r5.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: movc r2.z, r2.z, l(0), r5.x
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xxxx)).z;
    // 35: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 36: max r2.z, r2.z, cb0[0].x
    r2.z = (max(r2.zzzz,source[0].xxxx)).z;
    // 37: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 39: mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 40: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 41: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 43: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 44: dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 45: mul r3.xy, r3.xyxx, cb0[10].xxxx
    r3.xy = ((r3.xyxx)*(source[10].xxxx)).xy;
    // 46: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 47: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 49: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 50: add r3.z, r2.w, l(0.000010)
    r3.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 51: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 52: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 53: div r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = ((r3.xyzx)/(r2.wwww)).xyz;
    // 54: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r4.xyz, r2.wwww, r3.xyzx
    r4.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 57: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r5.xyz, r2.wwww, v7.xyzx
    r5.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 60: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 61: mad r6.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 62: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 63: mul r6.yzw, r6.yyyy, cb0[26].xxyz
    r6.yzw = ((r6.yyyy)*(source[26].xxyz)).yzw;
    // 64: mad r6.xyz, r6.xxxx, cb0[25].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[25].xyzx)+(r6.yzwy)).xyz;
    // 65: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 66: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 67: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 68: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 69: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 70: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 71: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 72: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 73: mul r8.xyz, r2.wwww, v0.xyzx
    r8.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 74: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 75: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 76: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 77: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 78: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 79: dp2 r11.z, r10.xyxx, cb0[17].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 80: mul r10.zw, cb0[17].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r10.zw = ((source[17].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 81: dp2 r11.x, r10.xyxx, r10.zwzz
    r11.x = (dot((r10.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 82: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 83: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 84: dp4 r12.x, cb0[18].xyzw, r11.xyzw
    r12.x = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 85: dp4 r12.y, cb0[19].xyzw, r11.xyzw
    r12.y = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 86: dp4 r12.z, cb0[20].xyzw, r11.xyzw
    r12.z = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 87: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 88: mul r2.w, r11.y, r11.y
    r2.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 89: mad r2.w, r11.x, r11.x, -r2.w
    r2.w = ((r11.xxxx)*(r11.xxxx)+(-(r2.wwww))).w;
    // 90: dp4 r11.x, cb0[21].xyzw, r13.xyzw
    r11.x = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 91: dp4 r11.y, cb0[22].xyzw, r13.xyzw
    r11.y = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 92: dp4 r11.z, cb0[23].xyzw, r13.xyzw
    r11.z = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 93: add r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)+(r12.xyzx)).xyz;
    // 94: mad r11.xyz, cb0[24].xyzx, r2.wwww, r11.xyzx
    r11.xyz = ((source[24].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 95: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 96: mul r11.xyz, r11.xyzx, cb0[16].xyzx
    r11.xyz = ((r11.xyzx)*(source[16].xyzx)).xyz;
    // 97: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 98: dp3 r2.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 99: dp3 r3.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 101: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 102: mul r12.xyz, r4.wwww, v6.xyzx
    r12.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 103: dp3 r4.w, r4.xyzx, r12.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 104: deriv_rtx_coarse r10.x, r4.w
    r10.x = (ddx_coarse(r4.wwww)).x;
    // 105: deriv_rty_coarse r10.y, r4.w
    r10.y = (ddy_coarse(r4.wwww)).y;
    // 106: dp2 r5.w, r10.xyxx, r10.xyxx
    r5.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 107: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 108: mad_sat r10.y, r5.w, l(0.300000), r2.z
    r10.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 109: mad r2.z, r10.y, l(0.200000), l(0.200000)
    r2.z = ((r10.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 110: div r3.w, r3.w, r2.z
    r3.w = ((r3.wwww)/(r2.zzzz)).w;
    // 111: mad r3.w, r2.w, l(5.000000), r3.w
    r3.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r5.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r5.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 113: mad r2.x, r5.w, -r2.x, r2.x
    r2.x = ((r5.wwww)*(-(r2.xxxx))+(r2.xxxx)).x;
    // 114: add_sat r0.w, r0.w, r2.x
    r0.w = (saturate((r0.wwww)+(r2.xxxx))).w;
    // 115: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 116: add_sat r2.x, r0.w, r3.w
    r2.x = (saturate((r0.wwww)+(r3.wwww))).x;
    // 117: mad r3.w, r2.x, l(-2.000000), l(3.000000)
    r3.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 118: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 119: mul r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)*(r3.wwww)).x;
    // 120: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 121: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 122: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 123: mul r11.xyz, r2.xxxx, r11.xyzx
    r11.xyz = ((r2.xxxx)*(r11.xyzx)).xyz;
    // 124: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 125: mul r13.xyz, r4.wwww, r4.xyzx
    r13.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 126: dp3 r2.x, -r5.xyzx, r4.xyzx
    r2.x = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).x;
    // 127: mad r4.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 128: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 129: add r2.x, r13.z, l(1.000000)
    r2.x = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 130: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 131: add r3.w, r4.w, l(1.000000)
    r3.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 133: log r4.z, r4.w
    r4.z = (log2(r4.wwww)).z;
    // 134: mul r4.z, r4.z, cb0[1].y
    r4.z = ((r4.zzzz)*(source[1].yyyy)).z;
    // 135: exp r4.z, r4.z
    r4.z = (exp2(r4.zzzz)).z;
    // 136: mad_sat r4.z, r4.z, cb0[1].w, cb0[1].z
    r4.z = (saturate((r4.zzzz)*(source[1].wwww)+(source[1].zzzz))).z;
    // 137: add_sat r10.x, -r2.x, r3.w
    r10.x = (saturate((-(r2.xxxx))+(r3.wwww))).x;
    // 138: sample_indexable(texture2d)(float,float,float,float) r14.xy, r10.xyxx, t6.xyzw, s7
    r14.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 139: add r14.zw, -r10.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r14.zw = ((-(r10.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 140: add r2.x, r1.w, r10.x
    r2.x = ((r1.wwww)+(r10.xxxx)).x;
    // 141: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 142: mov_sat r3.w, cb0[12].z
    r3.w = (saturate(source[12].zzzz)).w;
    // 143: mad r15.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 144: mul r3.w, r3.w, l(0.080000)
    r3.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 145: mad r15.xyz, r0.wwww, r15.xyzx, r3.wwww
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r3.wwww)).xyz;
    // 146: max r16.xyz, r14.zzzz, r15.xyzx
    r16.xyz = (max(r14.zzzz,r15.xyzx)).xyz;
    // 147: add r16.xyz, -r15.xyzx, r16.xyzx
    r16.xyz = ((-(r15.xyzx))+(r16.xyzx)).xyz;
    // 148: mul_sat r3.w, r15.y, l(50.000000)
    r3.w = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 149: mul r16.xyz, r3.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r16.xyzx)).xyz;
    // 150: mul r17.xyz, r14.yyyy, r15.xyzx
    r17.xyz = ((r14.yyyy)*(r15.xyzx)).xyz;
    // 151: mad r16.xyz, r16.xyzx, r14.xxxx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r14.xxxx)+(r17.xyzx)).xyz;
    // 152: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r14.y
    r4.w = r14.y != 0.f ? 1.f / r14.y : 0.f;
    // 153: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 154: mad r14.xyz, r15.xyzx, r4.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r15.xyzx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 155: mad r17.xyz, -r16.xyzx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 156: mul r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r16.xyzx)).xyz;
    // 157: mul r4.w, r14.w, r14.w
    r4.w = ((r14.wwww)*(r14.wwww)).w;
    // 158: mul r4.xyw, r4.xyxw, r4.xyxw
    r4.xyw = ((r4.xyxw)*(r4.xyxw)).xyw;
    // 159: mul r6.w, r14.w, r4.w
    r6.w = ((r14.wwww)*(r4.wwww)).w;
    // 160: mad r4.w, -r4.w, r14.w, l(1.000000)
    r4.w = ((-(r4.wwww))*(r14.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: mul r16.xyz, r15.xyzx, r4.wwww
    r16.xyz = ((r15.xyzx)*(r4.wwww)).xyz;
    // 162: dp3 r4.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: mad r15.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 164: mad r16.xyz, r3.wwww, r6.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r6.wwww)+(r16.xyzx)).xyz;
    // 165: add r16.xyz, -r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r16.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 166: mul r16.xyz, r16.xyzx, r16.xyzx
    r16.xyz = ((r16.xyzx)*(r16.xyzx)).xyz;
    // 167: add r3.w, -r2.y, l(1.000000)
    r3.w = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: mad_sat r2.y, r5.w, r3.w, r2.y
    r2.y = (saturate((r5.wwww)*(r3.wwww)+(r2.yyyy))).y;
    // 169: mad r2.y, -r2.y, cb0[2].x, l(1.000000)
    r2.y = ((-(r2.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 170: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 171: mul r4.w, r10.y, l(5.000000)
    r4.w = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 172: mad r5.w, r3.w, l(0.350000), l(1.000000)
    r5.w = ((r3.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: mul r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)*(r3.wwww)).x;
    // 174: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 175: add r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)+(r2.xxxx)).w;
    // 176: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 177: div_sat r2.x, r2.y, r5.w
    r2.x = (saturate((r2.yyyy)/(r5.wwww))).x;
    // 178: mad r18.xyz, -r2.xxxx, r16.xyzx, r17.xyzx
    r18.xyz = ((-(r2.xxxx))*(r16.xyzx)+(r17.xyzx)).xyz;
    // 179: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 180: mul r16.xyz, r16.xyzx, r2.xxxx
    r16.xyz = ((r16.xyzx)*(r2.xxxx)).xyz;
    // 181: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 182: mad r16.xyz, -r16.xyzx, r0.wwww, r16.xyzx
    r16.xyz = ((-(r16.xyzx))*(r0.wwww)+(r16.xyzx)).xyz;
    // 183: mul r6.xyz, r6.xyzx, r18.xyzx
    r6.xyz = ((r6.xyzx)*(r18.xyzx)).xyz;
    // 184: mad r6.xyz, -r6.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.wwww)+(r6.xyzx)).xyz;
    // 185: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 186: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 187: dp2 r9.x, r8.xyxx, r10.zwzz
    r9.x = (dot((r8.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 188: dp2 r9.z, r8.xyxx, cb0[17].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 189: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 190: dp3 r2.y, r5.xyzx, r13.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 191: mad r5.xy, r2.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 192: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 193: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t7.xyzw, s6, r4.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r4.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 194: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 195: mul r7.xyz, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r7.xyzx)*(source[16].xyzx)).xyz;
    // 196: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 197: dp3 r2.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 198: div r2.y, r2.y, r2.z
    r2.y = ((r2.yyyy)/(r2.zzzz)).y;
    // 199: mad r2.y, r2.w, l(5.000000), r2.y
    r2.y = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.yyyy)).y;
    // 200: add_sat r2.y, r0.w, r2.y
    r2.y = (saturate((r0.wwww)+(r2.yyyy))).y;
    // 201: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 202: mad r2.z, r2.y, l(-2.000000), l(3.000000)
    r2.z = ((r2.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 203: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 204: mul r2.y, r2.y, r2.z
    r2.y = ((r2.yyyy)*(r2.zzzz)).y;
    // 205: log r2.y, r2.y
    r2.y = (log2(r2.yyyy)).y;
    // 206: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 207: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 208: mul r2.yzw, r2.yyyy, r7.xxyz
    r2.yzw = ((r2.yyyy)*(r7.xxyz)).yzw;
    // 209: mad r3.w, r1.w, r15.x, r15.y
    r3.w = ((r1.wwww)*(r15.xxxx)+(r15.yyyy)).w;
    // 210: mad r3.w, r3.w, r1.w, r15.z
    r3.w = ((r3.wwww)*(r1.wwww)+(r15.zzzz)).w;
    // 211: mul r3.w, r1.w, r3.w
    r3.w = ((r1.wwww)*(r3.wwww)).w;
    // 212: max r1.w, r1.w, r3.w
    r1.w = (max(r1.wwww,r3.wwww)).w;
    // 213: mul r5.yzw, r5.yyyy, cb0[26].xxyz
    r5.yzw = ((r5.yyyy)*(source[26].xxyz)).yzw;
    // 214: mad r5.xyz, cb0[25].xyzx, r5.xxxx, r5.yzwy
    r5.xyz = ((source[25].xyzx)*(r5.xxxx)+(r5.yzwy)).xyz;
    // 215: mul r5.xyz, r5.xyzx, cb0[27].wwww
    r5.xyz = ((r5.xyzx)*(source[27].wwww)).xyz;
    // 216: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 217: mul r5.xyz, r2.yzwy, r5.xyzx
    r5.xyz = ((r2.yzwy)*(r5.xyzx)).xyz;
    // 218: mul r2.yzw, r2.yyzw, r14.xxyz
    r2.yzw = ((r2.yyzw)*(r14.xxyz)).yzw;
    // 219: mad r5.xyz, r5.xyzx, r14.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r14.xyzx)+(r6.xyzx)).xyz;
    // 220: dp3 r3.x, r3.xyzx, r12.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 221: add r3.y, -|r12.z|, l(1.000000)
    r3.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 222: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 223: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 224: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 225: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 226: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 227: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 228: mul r3.xzw, r3.xxxx, cb0[4].xxyz
    r3.xzw = ((r3.xxxx)*(source[4].xxyz)).xzw;
    // 229: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 230: mul r6.xy, v4.xyxx, cb0[5].xyxx
    r6.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 231: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 232: mul r7.xyz, cb0[6].xyzx, cb0[10].yyyy
    r7.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 233: mad r3.xyz, r6.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 234: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 235: mul r6.xyz, r0.wwww, r17.xyzx
    r6.xyz = ((r0.wwww)*(r17.xyzx)).xyz;
    // 236: mul r6.xyz, r11.xyzx, r6.xyzx
    r6.xyz = ((r11.xyzx)*(r6.xyzx)).xyz;
    // 237: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 238: mad r1.xyz, r2.yzwy, r1.wwww, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r1.wwww)+(r1.xyzx)).xyz;
    // 239: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 240: add r1.w, -r2.x, l(1.000000)
    r1.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 241: mad r2.yzw, r1.xxyz, r1.wwww, r3.xxyz
    r2.yzw = ((r1.xxyz)*(r1.wwww)+(r3.xxyz)).yzw;
    // 242: mul r1.xyz, r2.xxxx, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)).xyz;
    // 243: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r16.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r16.xyzx)).xyz;
    // 244: mul r3.xyz, r4.yyyy, cb0[26].xyzx
    r3.xyz = ((r4.yyyy)*(source[26].xyzx)).xyz;
    // 245: mad r3.xyz, r4.xxxx, cb0[25].xyzx, r3.xyzx
    r3.xyz = ((r4.xxxx)*(source[25].xyzx)+(r3.xyzx)).xyz;
    // 246: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r4.xyw, v4.xyxx, t5.xywz, s5, l(0.000000)
    r4.xyw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyw;
    // 248: add_sat r4.xyw, -r4.xyxw, l(1.000000, 1.000000, 0.000000, 1.000000)
    r4.xyw = (saturate((-(r4.xyxw))+(float4(1.000000,1.000000,0.000000,1.000000)))).xyw;
    // 249: add_sat r4.xyw, r4.xyxw, cb0[15].yyyy
    r4.xyw = (saturate((r4.xyxw)+(source[15].yyyy))).xyw;
    // 250: mul r3.w, r4.x, cb0[15].z
    r3.w = ((r4.xxxx)*(source[15].zzzz)).w;
    // 251: mul_sat r4.xyw, r4.xyxw, cb0[9].xyxz
    r4.xyw = (saturate((r4.xyxw)*(source[9].xyxz))).xyw;
    // 252: mul r3.w, r3.w, r4.z
    r3.w = ((r3.wwww)*(r4.zzzz)).w;
    // 253: mul r4.xyz, r4.xywx, r3.wwww
    r4.xyz = ((r4.xywx)*(r3.wwww)).xyz;
    // 254: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 255: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 256: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 257: mad r2.yzw, r0.xxyz, r1.wwww, r2.yyzw
    r2.yzw = ((r0.xxyz)*(r1.wwww)+(r2.yyzw)).yzw;
    // 258: mul r0.xyz, r2.xxxx, r0.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)).xyz;
    // 259: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 260: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 261: add r0.xyz, r2.yzwy, r5.xyzx
    r0.xyz = ((r2.yzwy)+(r5.xyzx)).xyz;
    // 262: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 263: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 264: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 265: ret
    return output;
}

// source.character.static-map-native-1141.v1 / source program d8ccb5441006794c9737fe27b40b18f1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1141(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    source[16]=g_SourceCharacterBaseConstants[13];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    source[29]=1.f;
    source[30]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f;
    // 1: mul r0.xyz, cb0[8].xyzx, cb0[11].yyyy
    r0.xyz = ((source[8].xyzx)*(source[11].yyyy)).xyz;
    // 2: mul r1.xyz, cb0[7].xyzx, cb0[11].xxxx
    r1.xyz = ((source[7].xyzx)*(source[11].xxxx)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r4.xyz, cb0[10].wwww, r3.xyzx, r2.xyzx
    r4.xyz = ((source[10].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mad r2.xyz, cb0[11].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[11].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: mul r0.w, r2.z, cb0[12].x
    r0.w = ((r2.zzzz)*(source[12].xxxx)).w;
    // 12: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 13: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.w, r1.w, cb0[12].y
    r1.w = ((r1.wwww)*(source[12].yyyy)).w;
    // 15: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 16: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 17: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 19: mul r1.xyz, r0.xyzx, cb0[12].zzzz
    r1.xyz = ((r0.xyzx)*(source[12].zzzz)).xyz;
    // 20: mad r0.xyz, cb0[12].wwww, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[12].wwww)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 21: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 25: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 26: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 27: mul r1.w, r2.x, cb0[15].x
    r1.w = ((r2.xxxx)*(source[15].xxxx)).w;
    // 28: mul r2.x, r2.y, cb0[14].z
    r2.x = ((r2.yyyy)*(source[14].zzzz)).x;
    // 29: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 30: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 31: mul r2.y, r2.y, cb0[15].y
    r2.y = ((r2.yyyy)*(source[15].yyyy)).y;
    // 32: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 33: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 35: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 36: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 37: mad r1.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 38: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 39: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 40: dp3 r2.y, v7.xyzx, v7.xyzx
    r2.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 41: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 42: mul r3.xyz, r2.yyyy, v7.xyzx
    r3.xyz = ((r2.yyyy)*(v7.xyzx)).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 44: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 45: dp2 r3.w, r2.yzyy, r2.yzyy
    r3.w = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).w;
    // 46: mul r2.yz, r2.yyzy, cb0[10].xxxx
    r2.yz = ((r2.yyzy)*(source[10].xxxx)).yz;
    // 47: mul r4.xy, r2.yzyy, v2.wwww
    r4.xy = ((r2.yzyy)*(v2.wwww)).xy;
    // 48: add r2.y, -r3.w, l(1.000000)
    r2.y = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 50: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 51: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 53: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 54: div r4.xyz, r4.xyzx, r2.yyyy
    r4.xyz = ((r4.xyzx)/(r2.yyyy)).xyz;
    // 55: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 56: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 57: mul r5.xyz, r2.yyyy, r4.xyzx
    r5.xyz = ((r2.yyyy)*(r4.xyzx)).xyz;
    // 58: dp3 r2.y, r3.xyzx, r5.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 59: dp3 r2.z, -r3.xyzx, r5.xyzx
    r2.z = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).z;
    // 60: mad r3.xy, r2.zzzz, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.zzzz)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 61: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 62: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 63: mul r6.xyz, r2.zzzz, cb0[27].xyzx
    r6.xyz = ((r2.zzzz)*(source[27].xyzx)).xyz;
    // 64: mad r6.xyz, r2.yyyy, cb0[26].xyzx, r6.xyzx
    r6.xyz = ((r2.yyyy)*(source[26].xyzx)+(r6.xyzx)).xyz;
    // 65: mul r6.xyz, r6.xyzx, cb0[28].wwww
    r6.xyz = ((r6.xyzx)*(source[28].wwww)).xyz;
    // 66: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 67: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 68: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 69: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 70: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 71: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t9.xyzw, s6
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 72: mul r9.xyz, r9.xyzx, cb0[30].xyzx
    r9.xyz = ((r9.xyzx)*(source[30].xyzx)).xyz;
    // 73: dp3 r2.y, r9.xyzx, r8.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 74: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s6
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 75: mul r8.xyz, r8.xyzx, cb0[29].xyzx
    r8.xyz = ((r8.xyzx)*(source[29].xyzx)).xyz;
    // 76: mul r10.xyz, r2.yyyy, r8.xyzx
    r10.xyz = ((r2.yyyy)*(r8.xyzx)).xyz;
    // 77: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 78: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 79: dp3 r2.z, v1.xyzx, v1.xyzx
    r2.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 80: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 81: mul r10.xyz, r2.zzzz, v1.xyzx
    r10.xyz = ((r2.zzzz)*(v1.xyzx)).xyz;
    // 82: dp3 r2.z, v0.xyzx, v0.xyzx
    r2.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 83: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 84: mul r11.xyz, r2.zzzz, v0.xyzx
    r11.xyz = ((r2.zzzz)*(v0.xyzx)).xyz;
    // 85: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 86: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 87: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 88: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 89: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 90: dp2 r14.z, r13.xyxx, cb0[18].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 91: mul r3.zw, cb0[18].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[18].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 92: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 93: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 94: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 95: dp4 r13.x, cb0[19].xyzw, r14.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 96: dp4 r13.y, cb0[20].xyzw, r14.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 97: dp4 r13.z, cb0[21].xyzw, r14.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 98: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 99: mul r2.z, r14.y, r14.y
    r2.z = ((r14.yyyy)*(r14.yyyy)).z;
    // 100: mad r2.z, r14.x, r14.x, -r2.z
    r2.z = ((r14.xxxx)*(r14.xxxx)+(-(r2.zzzz))).z;
    // 101: dp4 r14.x, cb0[22].xyzw, r15.xyzw
    r14.x = (dot((source[22].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 102: dp4 r14.y, cb0[23].xyzw, r15.xyzw
    r14.y = (dot((source[23].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 103: dp4 r14.z, cb0[24].xyzw, r15.xyzw
    r14.z = (dot((source[24].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 104: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 105: mad r13.xyz, cb0[25].xyzx, r2.zzzz, r13.xyzx
    r13.xyz = ((source[25].xyzx)*(r2.zzzz)+(r13.xyzx)).xyz;
    // 106: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 107: mul r13.xyz, r13.xyzx, cb0[17].xyzx
    r13.xyz = ((r13.xyzx)*(source[17].xyzx)).xyz;
    // 108: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 109: dp3 r2.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 110: log r4.w, |r2.x|
    r4.w = (log2(abs(r2.xxxx))).w;
    // 111: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 112: mul r4.w, r4.w, cb0[14].w
    r4.w = ((r4.wwww)*(source[14].wwww)).w;
    // 113: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 114: movc r2.x, r2.x, l(0), r4.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).x;
    // 115: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 116: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 118: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 119: mul r14.xyz, r4.wwww, v6.xyzx
    r14.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 120: dp3 r4.w, r5.xyzx, r14.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 121: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 122: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 123: deriv_rtx_coarse r15.x, r4.w
    r15.x = (ddx_coarse(r4.wwww)).x;
    // 124: deriv_rty_coarse r15.y, r4.w
    r15.y = (ddy_coarse(r4.wwww)).y;
    // 125: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 126: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 127: mad_sat r15.y, r5.w, l(0.300000), r2.x
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 128: mad r5.w, r15.y, l(0.200000), l(0.200000)
    r5.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 129: div r2.z, r2.z, r5.w
    r2.z = ((r2.zzzz)/(r5.wwww)).z;
    // 130: dp3 r6.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 131: mad r2.z, r6.w, l(5.000000), r2.z
    r2.z = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.zzzz)).z;
    // 132: add r7.w, r2.w, cb0[14].x
    r7.w = ((r2.wwww)+(source[14].xxxx)).w;
    // 133: add r2.w, r2.w, cb0[13].z
    r2.w = ((r2.wwww)+(source[13].zzzz)).w;
    // 134: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r8.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 135: mad r7.w, r8.w, -r7.w, r7.w
    r7.w = ((r8.wwww)*(-(r7.wwww))+(r7.wwww)).w;
    // 136: add_sat r0.w, r0.w, r7.w
    r0.w = (saturate((r0.wwww)+(r7.wwww))).w;
    // 137: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 138: add_sat r2.z, r0.w, r2.z
    r2.z = (saturate((r0.wwww)+(r2.zzzz))).z;
    // 139: mad r7.w, r2.z, l(-2.000000), l(3.000000)
    r7.w = ((r2.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 140: mul r2.z, r2.z, r2.z
    r2.z = ((r2.zzzz)*(r2.zzzz)).z;
    // 141: mul r2.z, r2.z, r7.w
    r2.z = ((r2.zzzz)*(r7.wwww)).z;
    // 142: log r2.z, r2.z
    r2.z = (log2(r2.zzzz)).z;
    // 143: mul r2.z, r2.z, l(1.500000)
    r2.z = ((r2.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 144: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 145: mul r13.xyz, r2.zzzz, r13.xyzx
    r13.xyz = ((r2.zzzz)*(r13.xyzx)).xyz;
    // 146: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 147: add r2.z, -r2.w, l(1.000000)
    r2.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 148: mad_sat r2.z, r8.w, r2.z, r2.w
    r2.z = (saturate((r8.wwww)*(r2.zzzz)+(r2.wwww))).z;
    // 149: mad r2.z, -r2.z, cb0[2].x, l(1.000000)
    r2.z = ((-(r2.zzzz))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 150: mul r2.w, r15.y, r15.y
    r2.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 151: mad r7.w, r2.w, l(0.350000), l(1.000000)
    r7.w = ((r2.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: div_sat r2.z, r2.z, r7.w
    r2.z = (saturate((r2.zzzz)/(r7.wwww))).z;
    // 153: mov_sat r7.w, cb0[13].x
    r7.w = (saturate(source[13].xxxx)).w;
    // 154: mad r16.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 155: mul r7.w, r7.w, l(0.080000)
    r7.w = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 156: mad r16.xyz, r0.wwww, r16.xyzx, r7.wwww
    r16.xyz = ((r0.wwww)*(r16.xyzx)+(r7.wwww)).xyz;
    // 157: add r7.w, r4.w, l(1.000000)
    r7.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 159: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 160: mul r4.w, r4.w, cb0[1].y
    r4.w = ((r4.wwww)*(source[1].yyyy)).w;
    // 161: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 162: mad_sat r4.w, r4.w, cb0[1].w, cb0[1].z
    r4.w = (saturate((r4.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 163: add r8.w, r5.z, l(1.000000)
    r8.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: add_sat r15.x, r7.w, -r8.w
    r15.x = (saturate((r7.wwww)+(-(r8.wwww)))).x;
    // 166: add r15.zw, -r15.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r15.zw = ((-(r15.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 167: max r17.xyz, r16.xyzx, r15.zzzz
    r17.xyz = (max(r16.xyzx,r15.zzzz)).xyz;
    // 168: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 169: mul_sat r7.w, r16.y, l(50.000000)
    r7.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 170: mul r17.xyz, r7.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r17.xyzx)).xyz;
    // 171: sample_indexable(texture2d)(float,float,float,float) r18.xy, r15.xyxx, t6.xyzw, s8
    r18.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 172: mul r8.w, r15.y, l(5.000000)
    r8.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 173: add r9.w, r1.w, r15.x
    r9.w = ((r1.wwww)+(r15.xxxx)).w;
    // 174: log r9.w, r9.w
    r9.w = (log2(r9.wwww)).w;
    // 175: mul r2.w, r2.w, r9.w
    r2.w = ((r2.wwww)*(r9.wwww)).w;
    // 176: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 177: add r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)+(r2.wwww)).w;
    // 178: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 179: mul r15.xyz, r16.xyzx, r18.yyyy
    r15.xyz = ((r16.xyzx)*(r18.yyyy)).xyz;
    // 180: mad r15.xyz, r17.xyzx, r18.xxxx, r15.xyzx
    r15.xyz = ((r17.xyzx)*(r18.xxxx)+(r15.xyzx)).xyz;
    // 181: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r18.y
    r2.w = r18.y != 0.f ? 1.f / r18.y : 0.f;
    // 182: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r17.xyz, r16.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 186: mul r2.w, r15.w, r15.w
    r2.w = ((r15.wwww)*(r15.wwww)).w;
    // 187: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 188: mad r9.w, -r2.w, r15.w, l(1.000000)
    r9.w = ((-(r2.wwww))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mul r2.w, r15.w, r2.w
    r2.w = ((r15.wwww)*(r2.wwww)).w;
    // 190: mul r17.xyz, r16.xyzx, r9.wwww
    r17.xyz = ((r16.xyzx)*(r9.wwww)).xyz;
    // 191: dp3 r9.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r9.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 192: mad r16.xyz, r9.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r9.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 193: mad r17.xyz, r7.wwww, r2.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r2.wwww)+(r17.xyzx)).xyz;
    // 194: add r17.xyz, -r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r17.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 196: mad r19.xyz, -r2.zzzz, r17.xyzx, r18.xyzx
    r19.xyz = ((-(r2.zzzz))*(r17.xyzx)+(r18.xyzx)).xyz;
    // 197: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 198: mul r17.xyz, r2.zzzz, r17.xyzx
    r17.xyz = ((r2.zzzz)*(r17.xyzx)).xyz;
    // 199: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 200: mad r17.xyz, -r17.xyzx, r0.wwww, r17.xyzx
    r17.xyz = ((-(r17.xyzx))*(r0.wwww)+(r17.xyzx)).xyz;
    // 201: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 202: mad r7.xyz, -r7.xyzx, r0.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.wwww)+(r7.xyzx)).xyz;
    // 203: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 204: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 205: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 206: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 207: dp3 r2.w, r9.xyzx, r19.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).w;
    // 208: add r2.y, -r2.w, r2.y
    r2.y = ((-(r2.wwww))+(r2.yyyy)).y;
    // 209: mad r2.x, r2.x, r2.y, r2.w
    r2.x = ((r2.xxxx)*(r2.yyyy)+(r2.wwww)).x;
    // 210: mad r2.xyw, r8.xyxz, r2.xxxx, r6.xyxz
    r2.xyw = ((r8.xyxz)*(r2.xxxx)+(r6.xyxz)).xyw;
    // 211: mad r6.x, r1.w, r16.x, r16.y
    r6.x = ((r1.wwww)*(r16.xxxx)+(r16.yyyy)).x;
    // 212: mad r6.x, r6.x, r1.w, r16.z
    r6.x = ((r6.xxxx)*(r1.wwww)+(r16.zzzz)).x;
    // 213: mul r6.x, r1.w, r6.x
    r6.x = ((r1.wwww)*(r6.xxxx)).x;
    // 214: max r1.w, r1.w, r6.x
    r1.w = (max(r1.wwww,r6.xxxx)).w;
    // 215: mul r2.xyw, r1.wwww, r2.xyxw
    r2.xyw = ((r1.wwww)*(r2.xyxw)).xyw;
    // 216: dp3 r6.x, r11.xyzx, r5.xyzx
    r6.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 217: dp3 r6.y, r12.xyzx, r5.xyzx
    r6.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 218: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 219: dp2 r5.x, r6.xyxx, r3.zwzz
    r5.x = (dot((r6.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 220: dp2 r5.z, r6.xyxx, cb0[18].xyxx
    r5.z = (dot((r6.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 221: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t7.xyzw, s7, r8.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r8.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 222: mul r5.xyz, r10.xyzx, r10.wwww
    r5.xyz = ((r10.xyzx)*(r10.wwww)).xyz;
    // 223: mul r5.xyz, r5.xyzx, cb0[17].xyzx
    r5.xyz = ((r5.xyzx)*(source[17].xyzx)).xyz;
    // 224: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[17].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[17].wwww)).xyz;
    // 225: dp3 r3.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 226: div r3.z, r3.z, r5.w
    r3.z = ((r3.zzzz)/(r5.wwww)).z;
    // 227: mad r3.z, r6.w, l(5.000000), r3.z
    r3.z = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.zzzz)).z;
    // 228: add_sat r3.z, r0.w, r3.z
    r3.z = (saturate((r0.wwww)+(r3.zzzz))).z;
    // 229: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 230: mad r3.w, r3.z, l(-2.000000), l(3.000000)
    r3.w = ((r3.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 231: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 232: mul r3.xyz, r3.xyzx, r3.xywx
    r3.xyz = ((r3.xyzx)*(r3.xywx)).xyz;
    // 233: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 234: mul r3.z, r3.z, l(1.500000)
    r3.z = ((r3.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 235: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 236: mul r5.xyz, r3.zzzz, r5.xyzx
    r5.xyz = ((r3.zzzz)*(r5.xyzx)).xyz;
    // 237: mul r2.xyw, r2.xyxw, r5.xyxz
    r2.xyw = ((r2.xyxw)*(r5.xyxz)).xyw;
    // 238: mul r5.xyz, r5.xyzx, r15.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)).xyz;
    // 239: mad r2.xyw, r2.xyxw, r15.xyxz, r7.xyxz
    r2.xyw = ((r2.xyxw)*(r15.xyxz)+(r7.xyxz)).xyw;
    // 240: mul r3.yzw, r3.yyyy, cb0[27].xxyz
    r3.yzw = ((r3.yyyy)*(source[27].xxyz)).yzw;
    // 241: mad r3.xyz, r3.xxxx, cb0[26].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[26].xyzx)+(r3.yzwy)).xyz;
    // 242: mul r3.xyz, r3.xyzx, cb0[28].wwww
    r3.xyz = ((r3.xyzx)*(source[28].wwww)).xyz;
    // 243: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 244: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 245: add_sat r6.xyz, r6.xyzx, cb0[15].wwww
    r6.xyz = (saturate((r6.xyzx)+(source[15].wwww))).xyz;
    // 246: mul r3.w, r6.x, cb0[16].x
    r3.w = ((r6.xxxx)*(source[16].xxxx)).w;
    // 247: mul_sat r6.xyz, r6.xyzx, cb0[9].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[9].xyzx))).xyz;
    // 248: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 249: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 250: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 251: mul r7.xyz, r0.wwww, r18.xyzx
    r7.xyz = ((r0.wwww)*(r18.xyzx)).xyz;
    // 252: mul r7.xyz, r13.xyzx, r7.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)).xyz;
    // 253: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 254: mad r1.xyz, r5.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 255: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 256: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 257: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 258: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 259: mul r5.xyz, r0.wwww, r8.xyzx
    r5.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 260: mul r3.xyz, r0.xyzx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 261: mad r0.xyz, r0.xyzx, r5.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 262: dp3 r0.w, r4.xyzx, r14.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 263: add r1.w, -|r14.z|, l(1.000000)
    r1.w = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 264: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 265: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 266: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 267: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 268: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 269: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 270: mul r3.xyz, r0.wwww, cb0[4].xyzx
    r3.xyz = ((r0.wwww)*(source[4].xyzx)).xyz;
    // 271: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 272: mul r4.xy, v4.xyxx, cb0[5].xyxx
    r4.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 273: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t4.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 274: mul r5.xyz, cb0[6].xyzx, cb0[10].yyyy
    r5.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 275: mad r3.xyz, r4.xyzx, r5.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 276: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 277: add r0.w, -r2.z, l(1.000000)
    r0.w = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 278: mad r3.xyz, r1.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 279: mul r1.xyz, r2.zzzz, r1.xyzx
    r1.xyz = ((r2.zzzz)*(r1.xyzx)).xyz;
    // 280: mul r4.xyz, r2.zzzz, r0.xyzx
    r4.xyz = ((r2.zzzz)*(r0.xyzx)).xyz;
    // 281: mad r0.xyz, r0.xyzx, r0.wwww, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 282: add r0.xyz, r0.xyzx, r2.xywx
    r0.xyz = ((r0.xyzx)+(r2.xywx)).xyz;
    // 283: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 284: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r17.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r17.xyzx)).xyz;
    // 285: mad r0.xyz, r4.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r4.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 286: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 287: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 288: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 289: ret
    return output;
}

// source.character.static-map-native-1141.v1 / source program 69952d4f7eb75e44839c44314d0ffc77
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1141(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1141(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    source[16]=g_SourceCharacterBaseConstants[13];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xyz, cb0[8].xyzx, cb0[11].yyyy
    r0.xyz = ((source[8].xyzx)*(source[11].yyyy)).xyz;
    // 2: mul r1.xyz, cb0[7].xyzx, cb0[11].xxxx
    r1.xyz = ((source[7].xyzx)*(source[11].xxxx)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r4.xyz, cb0[10].wwww, r3.xyzx, r2.xyzx
    r4.xyz = ((source[10].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mad r2.xyz, cb0[11].wwww, r3.xyzx, r2.xyzx
    r2.xyz = ((source[11].wwww)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: mul r0.w, r2.z, cb0[12].x
    r0.w = ((r2.zzzz)*(source[12].xxxx)).w;
    // 12: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 13: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.w, r1.w, cb0[12].y
    r1.w = ((r1.wwww)*(source[12].yyyy)).w;
    // 15: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 16: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 17: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 19: mul r1.xyz, r0.xyzx, cb0[12].zzzz
    r1.xyz = ((r0.xyzx)*(source[12].zzzz)).xyz;
    // 20: mad r0.xyz, cb0[12].wwww, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[12].wwww)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 21: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 23: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 24: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 25: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 26: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 27: mul r1.w, r2.x, cb0[15].x
    r1.w = ((r2.xxxx)*(source[15].xxxx)).w;
    // 28: mul r2.x, r2.y, cb0[14].z
    r2.x = ((r2.yyyy)*(source[14].zzzz)).x;
    // 29: log r2.y, |r1.w|
    r2.y = (log2(abs(r1.wwww))).y;
    // 30: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 31: mul r2.y, r2.y, cb0[15].y
    r2.y = ((r2.yyyy)*(source[15].yyyy)).y;
    // 32: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 33: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: movc r1.w, r1.w, l(0), r2.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 35: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 36: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 37: mad r1.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 38: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 39: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.yz, v4.xyxx, t0.zxyw, s0, l(0.000000)
    r2.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 41: mad r2.yz, r2.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r2.yz = ((r2.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 42: dp2 r3.x, r2.yzyy, r2.yzyy
    r3.x = (dot((r2.yzyy).xy,(r2.yzyy).xy).xxxx).x;
    // 43: mul r2.yz, r2.yyzy, cb0[10].xxxx
    r2.yz = ((r2.yyzy)*(source[10].xxxx)).yz;
    // 44: mul r4.xy, r2.yzyy, v2.wwww
    r4.xy = ((r2.yzyy)*(v2.wwww)).xy;
    // 45: add r2.y, -r3.x, l(1.000000)
    r2.y = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: max r2.y, r2.y, l(0.000000)
    r2.y = (max(r2.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 47: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 48: add r4.z, r2.y, l(0.000010)
    r4.z = ((r2.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 49: dp3 r2.y, r4.xyzx, r4.xyzx
    r2.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 50: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 51: div r3.xyz, r4.xyzx, r2.yyyy
    r3.xyz = ((r4.xyzx)/(r2.yyyy)).xyz;
    // 52: dp3 r2.y, r3.xyzx, r3.xyzx
    r2.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 53: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 54: mul r4.xyz, r2.yyyy, r3.xyzx
    r4.xyz = ((r2.yyyy)*(r3.xyzx)).xyz;
    // 55: dp3 r2.y, v7.xyzx, v7.xyzx
    r2.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 56: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 57: mul r5.xyz, r2.yyyy, v7.xyzx
    r5.xyz = ((r2.yyyy)*(v7.xyzx)).xyz;
    // 58: dp3 r2.y, r5.xyzx, r4.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 59: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 60: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 61: mul r6.xyz, r2.zzzz, cb0[27].xyzx
    r6.xyz = ((r2.zzzz)*(source[27].xyzx)).xyz;
    // 62: mad r6.xyz, r2.yyyy, cb0[26].xyzx, r6.xyzx
    r6.xyz = ((r2.yyyy)*(source[26].xyzx)+(r6.xyzx)).xyz;
    // 63: mul r6.xyz, r6.xyzx, cb0[28].wwww
    r6.xyz = ((r6.xyzx)*(source[28].wwww)).xyz;
    // 64: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 65: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 66: dp3 r2.y, v1.xyzx, v1.xyzx
    r2.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 67: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 68: mul r7.xyz, r2.yyyy, v1.xyzx
    r7.xyz = ((r2.yyyy)*(v1.xyzx)).xyz;
    // 69: dp3 r2.y, v0.xyzx, v0.xyzx
    r2.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 70: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 71: mul r8.xyz, r2.yyyy, v0.xyzx
    r8.xyz = ((r2.yyyy)*(v0.xyzx)).xyz;
    // 72: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 73: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 74: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 75: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 76: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 77: dp2 r11.z, r10.xyxx, cb0[18].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 78: mul r2.yz, cb0[18].yyxy, l(0.000000, 1.000000, -1.000000, 0.000000)
    r2.yz = ((source[18].yyxy)*(float4(0.000000,1.000000,-1.000000,0.000000))).yz;
    // 79: dp2 r11.x, r10.xyxx, r2.yzyy
    r11.x = (dot((r10.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 80: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 81: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 82: dp4 r10.x, cb0[19].xyzw, r11.xyzw
    r10.x = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 83: dp4 r10.y, cb0[20].xyzw, r11.xyzw
    r10.y = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 84: dp4 r10.z, cb0[21].xyzw, r11.xyzw
    r10.z = (dot((source[21].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 85: mul r12.xyzw, r11.yzzx, r11.xyzz
    r12.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 86: mul r3.w, r11.y, r11.y
    r3.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 87: mad r3.w, r11.x, r11.x, -r3.w
    r3.w = ((r11.xxxx)*(r11.xxxx)+(-(r3.wwww))).w;
    // 88: dp4 r11.x, cb0[22].xyzw, r12.xyzw
    r11.x = (dot((source[22].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 89: dp4 r11.y, cb0[23].xyzw, r12.xyzw
    r11.y = (dot((source[23].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 90: dp4 r11.z, cb0[24].xyzw, r12.xyzw
    r11.z = (dot((source[24].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 91: add r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)+(r11.xyzx)).xyz;
    // 92: mad r10.xyz, cb0[25].xyzx, r3.wwww, r10.xyzx
    r10.xyz = ((source[25].xyzx)*(r3.wwww)+(r10.xyzx)).xyz;
    // 93: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 94: mul r10.xyz, r10.xyzx, cb0[17].xyzx
    r10.xyz = ((r10.xyzx)*(source[17].xyzx)).xyz;
    // 95: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 96: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 97: log r4.w, |r2.x|
    r4.w = (log2(abs(r2.xxxx))).w;
    // 98: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 99: mul r4.w, r4.w, cb0[14].w
    r4.w = ((r4.wwww)*(source[14].wwww)).w;
    // 100: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 101: movc r2.x, r2.x, l(0), r4.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).x;
    // 102: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 103: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 105: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 106: mul r11.xyz, r4.wwww, v6.xyzx
    r11.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 107: dp3 r4.w, r4.xyzx, r11.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 108: deriv_rtx_coarse r12.x, r4.w
    r12.x = (ddx_coarse(r4.wwww)).x;
    // 109: deriv_rty_coarse r12.y, r4.w
    r12.y = (ddy_coarse(r4.wwww)).y;
    // 110: dp2 r5.w, r12.xyxx, r12.xyxx
    r5.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 111: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 112: mad_sat r12.y, r5.w, l(0.300000), r2.x
    r12.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.xxxx))).y;
    // 113: mad r2.x, r12.y, l(0.200000), l(0.200000)
    r2.x = ((r12.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 114: div r3.w, r3.w, r2.x
    r3.w = ((r3.wwww)/(r2.xxxx)).w;
    // 115: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 117: add r6.w, r2.w, cb0[14].x
    r6.w = ((r2.wwww)+(source[14].xxxx)).w;
    // 118: add r2.w, r2.w, cb0[13].z
    r2.w = ((r2.wwww)+(source[13].zzzz)).w;
    // 119: sample_b_indexable(texture2d)(float,float,float,float) r7.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r7.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 120: mad r6.w, r7.w, -r6.w, r6.w
    r6.w = ((r7.wwww)*(-(r6.wwww))+(r6.wwww)).w;
    // 121: add_sat r0.w, r0.w, r6.w
    r0.w = (saturate((r0.wwww)+(r6.wwww))).w;
    // 122: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 123: add_sat r3.w, r0.w, r3.w
    r3.w = (saturate((r0.wwww)+(r3.wwww))).w;
    // 124: mad r6.w, r3.w, l(-2.000000), l(3.000000)
    r6.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 125: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 126: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 127: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 128: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 129: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 130: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 131: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 132: mul r13.xyz, r4.wwww, r4.xyzx
    r13.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 133: dp3 r3.w, -r5.xyzx, r4.xyzx
    r3.w = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).w;
    // 134: mad r4.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 136: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 137: add r3.w, r13.z, l(1.000000)
    r3.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: add r4.z, r4.w, l(1.000000)
    r4.z = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 140: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 141: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 142: mul r4.w, r4.w, cb0[1].y
    r4.w = ((r4.wwww)*(source[1].yyyy)).w;
    // 143: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 144: mad_sat r4.w, r4.w, cb0[1].w, cb0[1].z
    r4.w = (saturate((r4.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 145: add_sat r12.x, -r3.w, r4.z
    r12.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 146: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t6.zwxy, s7
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 147: add r14.xy, -r12.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r14.xy = ((-(r12.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 148: add r3.w, r1.w, r12.x
    r3.w = ((r1.wwww)+(r12.xxxx)).w;
    // 149: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 150: mov_sat r4.z, cb0[13].x
    r4.z = (saturate(source[13].xxxx)).z;
    // 151: mad r15.xyz, -r4.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r4.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 152: mul r4.z, r4.z, l(0.080000)
    r4.z = ((r4.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 153: mad r15.xyz, r0.wwww, r15.xyzx, r4.zzzz
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r4.zzzz)).xyz;
    // 154: max r14.xzw, r14.xxxx, r15.xxyz
    r14.xzw = (max(r14.xxxx,r15.xxyz)).xzw;
    // 155: add r14.xzw, -r15.xxyz, r14.xxzw
    r14.xzw = ((-(r15.xxyz))+(r14.xxzw)).xzw;
    // 156: mul_sat r4.z, r15.y, l(50.000000)
    r4.z = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 157: mul r14.xzw, r4.zzzz, r14.xxzw
    r14.xzw = ((r4.zzzz)*(r14.xxzw)).xzw;
    // 158: mul r16.xyz, r12.wwww, r15.xyzx
    r16.xyz = ((r12.wwww)*(r15.xyzx)).xyz;
    // 159: mad r14.xzw, r14.xxzw, r12.zzzz, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r12.zzzz)+(r16.xxyz)).xzw;
    // 160: div r6.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r6.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 161: add r6.w, r6.w, l(-1.000000)
    r6.w = ((r6.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 162: mad r12.xzw, r15.xxyz, r6.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r15.xxyz)*(r6.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 163: mad r16.xyz, -r14.xzwx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xzwx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 164: mul r12.xzw, r12.xxzw, r14.xxzw
    r12.xzw = ((r12.xxzw)*(r14.xxzw)).xzw;
    // 165: mul r6.w, r14.y, r14.y
    r6.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 166: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 167: mul r8.w, r14.y, r6.w
    r8.w = ((r14.yyyy)*(r6.wwww)).w;
    // 168: mad r6.w, -r6.w, r14.y, l(1.000000)
    r6.w = ((-(r6.wwww))*(r14.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: mul r14.xyz, r15.xyzx, r6.wwww
    r14.xyz = ((r15.xyzx)*(r6.wwww)).xyz;
    // 170: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 171: mad r15.xyz, r6.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r6.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 172: mad r14.xyz, r4.zzzz, r8.wwww, r14.xyzx
    r14.xyz = ((r4.zzzz)*(r8.wwww)+(r14.xyzx)).xyz;
    // 173: add r14.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: mul r14.xyz, r14.xyzx, r14.xyzx
    r14.xyz = ((r14.xyzx)*(r14.xyzx)).xyz;
    // 175: add r4.z, -r2.w, l(1.000000)
    r4.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 176: mad_sat r2.w, r7.w, r4.z, r2.w
    r2.w = (saturate((r7.wwww)*(r4.zzzz)+(r2.wwww))).w;
    // 177: mad r2.w, -r2.w, cb0[2].x, l(1.000000)
    r2.w = ((-(r2.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: mul r4.z, r12.y, r12.y
    r4.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 179: mul r6.w, r12.y, l(5.000000)
    r6.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 180: mad r7.w, r4.z, l(0.350000), l(1.000000)
    r7.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: mul r3.w, r3.w, r4.z
    r3.w = ((r3.wwww)*(r4.zzzz)).w;
    // 182: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 183: add r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)+(r3.wwww)).w;
    // 184: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 185: div_sat r2.w, r2.w, r7.w
    r2.w = (saturate((r2.wwww)/(r7.wwww))).w;
    // 186: mad r17.xyz, -r2.wwww, r14.xyzx, r16.xyzx
    r17.xyz = ((-(r2.wwww))*(r14.xyzx)+(r16.xyzx)).xyz;
    // 187: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 188: mul r14.xyz, r14.xyzx, r2.wwww
    r14.xyz = ((r14.xyzx)*(r2.wwww)).xyz;
    // 189: mul r14.xyz, r0.xyzx, r14.xyzx
    r14.xyz = ((r0.xyzx)*(r14.xyzx)).xyz;
    // 190: mad r14.xyz, -r14.xyzx, r0.wwww, r14.xyzx
    r14.xyz = ((-(r14.xyzx))*(r0.wwww)+(r14.xyzx)).xyz;
    // 191: mul r6.xyz, r6.xyzx, r17.xyzx
    r6.xyz = ((r6.xyzx)*(r17.xyzx)).xyz;
    // 192: mad r6.xyz, -r6.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.wwww)+(r6.xyzx)).xyz;
    // 193: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 194: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 195: dp2 r9.x, r8.xyxx, r2.yzyy
    r9.x = (dot((r8.xyxx).xy,(r2.yzyy).xy).xxxx).x;
    // 196: dp2 r9.z, r8.xyxx, cb0[18].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 197: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 198: dp3 r2.y, r5.xyzx, r13.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 199: mad r2.yz, r2.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r2.yz = ((r2.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 200: mul r2.yz, r2.yyzy, r2.yyzy
    r2.yz = ((r2.yyzy)*(r2.yyzy)).yz;
    // 201: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t7.xyzw, s6, r6.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 202: mul r5.xyz, r7.xyzx, r7.wwww
    r5.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 203: mul r5.xyz, r5.xyzx, cb0[17].xyzx
    r5.xyz = ((r5.xyzx)*(source[17].xyzx)).xyz;
    // 204: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[17].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[17].wwww)).xyz;
    // 205: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 206: div r2.x, r3.w, r2.x
    r2.x = ((r3.wwww)/(r2.xxxx)).x;
    // 207: mad r2.x, r5.w, l(5.000000), r2.x
    r2.x = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.xxxx)).x;
    // 208: add_sat r2.x, r0.w, r2.x
    r2.x = (saturate((r0.wwww)+(r2.xxxx))).x;
    // 209: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: mad r3.w, r2.x, l(-2.000000), l(3.000000)
    r3.w = ((r2.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 211: mul r2.x, r2.x, r2.x
    r2.x = ((r2.xxxx)*(r2.xxxx)).x;
    // 212: mul r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)*(r3.wwww)).x;
    // 213: log r2.x, r2.x
    r2.x = (log2(r2.xxxx)).x;
    // 214: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 215: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 216: mul r5.xyz, r2.xxxx, r5.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)).xyz;
    // 217: mad r2.x, r1.w, r15.x, r15.y
    r2.x = ((r1.wwww)*(r15.xxxx)+(r15.yyyy)).x;
    // 218: mad r2.x, r2.x, r1.w, r15.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r15.zzzz)).x;
    // 219: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 220: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 221: mul r7.xyz, r2.zzzz, cb0[27].xyzx
    r7.xyz = ((r2.zzzz)*(source[27].xyzx)).xyz;
    // 222: mad r2.xyz, cb0[26].xyzx, r2.yyyy, r7.xyzx
    r2.xyz = ((source[26].xyzx)*(r2.yyyy)+(r7.xyzx)).xyz;
    // 223: mul r2.xyz, r2.xyzx, cb0[28].wwww
    r2.xyz = ((r2.xyzx)*(source[28].wwww)).xyz;
    // 224: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 225: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 226: mul r5.xyz, r5.xyzx, r12.xzwx
    r5.xyz = ((r5.xyzx)*(r12.xzwx)).xyz;
    // 227: mad r2.xyz, r2.xyzx, r12.xzwx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r12.xzwx)+(r6.xyzx)).xyz;
    // 228: dp3 r3.x, r3.xyzx, r11.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 229: add r3.y, -|r11.z|, l(1.000000)
    r3.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 230: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 231: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 232: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 233: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 234: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 235: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 236: mul r3.xzw, r3.xxxx, cb0[4].xxyz
    r3.xzw = ((r3.xxxx)*(source[4].xxyz)).xzw;
    // 237: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 238: mul r6.xy, v4.xyxx, cb0[5].xyxx
    r6.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 239: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 240: mul r7.xyz, cb0[6].xyzx, cb0[10].yyyy
    r7.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 241: mad r3.xyz, r6.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r7.xyzx)+(r3.xyzx)).xyz;
    // 242: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 243: mul r6.xyz, r0.wwww, r16.xyzx
    r6.xyz = ((r0.wwww)*(r16.xyzx)).xyz;
    // 244: mul r6.xyz, r10.xyzx, r6.xyzx
    r6.xyz = ((r10.xyzx)*(r6.xyzx)).xyz;
    // 245: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 246: mad r1.xyz, r5.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 247: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 248: add r1.w, -r2.w, l(1.000000)
    r1.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 249: mad r3.xyz, r1.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 250: mul r1.xyz, r2.wwww, r1.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)).xyz;
    // 251: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r14.xyzx)).xyz;
    // 252: mul r5.xyz, r4.yyyy, cb0[27].xyzx
    r5.xyz = ((r4.yyyy)*(source[27].xyzx)).xyz;
    // 253: mad r4.xyz, r4.xxxx, cb0[26].xyzx, r5.xyzx
    r4.xyz = ((r4.xxxx)*(source[26].xyzx)+(r5.xyzx)).xyz;
    // 254: mul r4.xyz, r4.xyzx, cb0[28].wwww
    r4.xyz = ((r4.xyzx)*(source[28].wwww)).xyz;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 256: add_sat r5.xyz, -r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (saturate((-(r5.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 257: add_sat r5.xyz, r5.xyzx, cb0[15].wwww
    r5.xyz = (saturate((r5.xyzx)+(source[15].wwww))).xyz;
    // 258: mul r3.w, r5.x, cb0[16].x
    r3.w = ((r5.xxxx)*(source[16].xxxx)).w;
    // 259: mul_sat r5.xyz, r5.xyzx, cb0[9].xyzx
    r5.xyz = (saturate((r5.xyzx)*(source[9].xyzx))).xyz;
    // 260: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 261: mul r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)*(r3.wwww)).xyz;
    // 262: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 263: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 264: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 265: mad r3.xyz, r0.xyzx, r1.wwww, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r1.wwww)+(r3.xyzx)).xyz;
    // 266: mul r0.xyz, r2.wwww, r0.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)).xyz;
    // 267: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 268: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 269: add r0.xyz, r2.xyzx, r3.xyzx
    r0.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 270: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 271: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 272: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 273: ret
    return output;
}

// source.character.static-map-native-1142.v1 / source program 6dec2d516b948547a1ade959a994bfef
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1142(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    source[28]=1.f;
    source[29]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f, r20=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 3: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 4: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 5: mul r0.w, r0.w, cb0[11].y
    r0.w = ((r0.wwww)*(source[11].yyyy)).w;
    // 6: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 7: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 8: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: mul r1.xyz, cb0[8].xyzx, cb0[10].wwww
    r1.xyz = ((source[8].xyzx)*(source[10].wwww)).xyz;
    // 10: mul r2.xyz, cb0[7].xyzx, cb0[10].zzzz
    r2.xyz = ((source[7].xyzx)*(source[10].zzzz)).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 13: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 14: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 15: mul r2.xyz, r1.xyzx, cb0[11].zzzz
    r2.xyz = ((r1.xyzx)*(source[11].zzzz)).xyz;
    // 16: mad r1.xyz, cb0[11].wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[11].wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 17: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 18: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 19: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 20: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 21: mad r2.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 22: mad r3.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 23: mul r0.x, r0.x, cb0[14].x
    r0.x = ((r0.xxxx)*(source[14].xxxx)).x;
    // 24: mul r0.y, r0.y, cb0[13].z
    r0.y = ((r0.yyyy)*(source[13].zzzz)).y;
    // 25: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 26: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 27: mul r0.w, r0.w, cb0[14].y
    r0.w = ((r0.wwww)*(source[14].yyyy)).w;
    // 28: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 29: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 31: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 32: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 33: mad r2.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 34: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 35: max r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (max(r0.xxxx,r2.xyzx)).xyz;
    // 36: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 37: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 38: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 40: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 42: mul r4.xy, r4.xyxx, cb0[10].xxxx
    r4.xy = ((r4.xyxx)*(source[10].xxxx)).xy;
    // 43: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 44: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 49: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 50: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 51: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 54: dp3 r0.w, r3.xyzx, r5.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 55: dp3 r1.w, -r3.xyzx, r5.xyzx
    r1.w = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).w;
    // 56: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 57: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 58: mad r6.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 60: mul r6.yzw, r6.yyyy, cb0[26].xxyz
    r6.yzw = ((r6.yyyy)*(source[26].xxyz)).yzw;
    // 61: mad r6.xyz, r6.xxxx, cb0[25].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[25].xyzx)+(r6.yzwy)).xyz;
    // 62: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 63: mul r7.xyz, r1.xyzx, r6.xyzx
    r7.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 64: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 65: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 66: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 67: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 68: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t9.xyzw, s6
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 69: mul r9.xyz, r9.xyzx, cb0[29].xyzx
    r9.xyz = ((r9.xyzx)*(source[29].xyzx)).xyz;
    // 70: dp3 r0.w, r9.xyzx, r8.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 71: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s6
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 72: mul r8.xyz, r8.xyzx, cb0[28].xyzx
    r8.xyz = ((r8.xyzx)*(source[28].xyzx)).xyz;
    // 73: mul r10.xyz, r0.wwww, r8.xyzx
    r10.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 74: mad r7.xyz, r1.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 75: mul r7.xyz, r2.xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 76: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 77: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 78: mul r10.xyz, r1.wwww, v1.xyzx
    r10.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 79: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 80: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 81: mul r11.xyz, r1.wwww, v0.xyzx
    r11.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 82: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 83: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 84: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 85: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 86: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 87: dp2 r14.z, r13.xyxx, cb0[17].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 88: mul r13.zw, cb0[17].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r13.zw = ((source[17].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 89: dp2 r14.x, r13.xyxx, r13.zwzz
    r14.x = (dot((r13.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 90: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 91: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 92: dp4 r15.x, cb0[18].xyzw, r14.xyzw
    r15.x = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 93: dp4 r15.y, cb0[19].xyzw, r14.xyzw
    r15.y = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 94: dp4 r15.z, cb0[20].xyzw, r14.xyzw
    r15.z = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 95: mul r16.xyzw, r14.yzzx, r14.xyzz
    r16.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 96: mul r1.w, r14.y, r14.y
    r1.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 97: mad r1.w, r14.x, r14.x, -r1.w
    r1.w = ((r14.xxxx)*(r14.xxxx)+(-(r1.wwww))).w;
    // 98: dp4 r14.x, cb0[21].xyzw, r16.xyzw
    r14.x = (dot((source[21].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 99: dp4 r14.y, cb0[22].xyzw, r16.xyzw
    r14.y = (dot((source[22].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 100: dp4 r14.z, cb0[23].xyzw, r16.xyzw
    r14.z = (dot((source[23].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 101: add r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)+(r15.xyzx)).xyz;
    // 102: mad r14.xyz, cb0[24].xyzx, r1.wwww, r14.xyzx
    r14.xyz = ((source[24].xyzx)*(r1.wwww)+(r14.xyzx)).xyz;
    // 103: max r14.xyz, r14.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r14.xyz = (max(r14.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 104: mul r14.xyz, r14.xyzx, cb0[16].xyzx
    r14.xyz = ((r14.xyzx)*(source[16].xyzx)).xyz;
    // 105: mad r14.xyz, r14.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r14.xyz = ((r14.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 106: dp3 r1.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 107: log r2.w, |r0.y|
    r2.w = (log2(abs(r0.yyyy))).w;
    // 108: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 109: mul r2.w, r2.w, cb0[13].w
    r2.w = ((r2.wwww)*(source[13].wwww)).w;
    // 110: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 111: movc r0.y, r0.y, l(0), r2.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 112: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 113: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 114: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 115: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 116: mul r15.xyz, r2.wwww, v6.xyzx
    r15.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 117: dp3 r2.w, r5.xyzx, r15.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r15.xyzx).xyz).xxxx).w;
    // 118: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 119: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 120: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 121: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 122: dp2 r3.z, r13.xyxx, r13.xyxx
    r3.z = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).z;
    // 123: sqrt r3.z, r3.z
    r3.z = (sqrt(r3.zzzz)).z;
    // 124: mad_sat r13.y, r3.z, l(0.300000), r0.y
    r13.y = (saturate((r3.zzzz)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy))).y;
    // 125: mad r3.z, r13.y, l(0.200000), l(0.200000)
    r3.z = ((r13.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 126: div r1.w, r1.w, r3.z
    r1.w = ((r1.wwww)/(r3.zzzz)).w;
    // 127: dp3 r4.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: mad r1.w, r4.w, l(5.000000), r1.w
    r1.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.wwww)).w;
    // 129: add r5.w, r3.w, cb0[13].x
    r5.w = ((r3.wwww)+(source[13].xxxx)).w;
    // 130: add r3.w, r3.w, cb0[12].z
    r3.w = ((r3.wwww)+(source[12].zzzz)).w;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r6.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r6.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 132: mad r5.w, r6.w, -r5.w, r5.w
    r5.w = ((r6.wwww)*(-(r5.wwww))+(r5.wwww)).w;
    // 133: add_sat r0.z, r0.z, r5.w
    r0.z = (saturate((r0.zzzz)+(r5.wwww))).z;
    // 134: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 135: add_sat r1.w, r0.z, r1.w
    r1.w = (saturate((r0.zzzz)+(r1.wwww))).w;
    // 136: mad r5.w, r1.w, l(-2.000000), l(3.000000)
    r5.w = ((r1.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 137: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 138: mul r1.w, r1.w, r5.w
    r1.w = ((r1.wwww)*(r5.wwww)).w;
    // 139: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 140: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 141: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 142: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 143: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 144: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mad_sat r1.w, r6.w, r1.w, r3.w
    r1.w = (saturate((r6.wwww)*(r1.wwww)+(r3.wwww))).w;
    // 146: mad r1.w, -r1.w, cb0[2].x, l(1.000000)
    r1.w = ((-(r1.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 148: mad r5.w, r3.w, l(0.350000), l(1.000000)
    r5.w = ((r3.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: div_sat r1.w, r1.w, r5.w
    r1.w = (saturate((r1.wwww)/(r5.wwww))).w;
    // 150: add r5.w, r2.w, l(1.000000)
    r5.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mov_sat r2.w, r2.w
    r2.w = (saturate(r2.wwww)).w;
    // 152: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 153: mul r2.w, r2.w, cb0[1].y
    r2.w = ((r2.wwww)*(source[1].yyyy)).w;
    // 154: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 155: mad_sat r2.w, r2.w, cb0[1].w, cb0[1].z
    r2.w = (saturate((r2.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 156: add r6.w, r5.z, l(1.000000)
    r6.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: min r6.w, r6.w, l(1.000000)
    r6.w = (min(r6.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: add_sat r13.x, r5.w, -r6.w
    r13.x = (saturate((r5.wwww)+(-(r6.wwww)))).x;
    // 159: sample_indexable(texture2d)(float,float,float,float) r16.xy, r13.xyxx, t6.xyzw, s8
    r16.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 160: add r16.zw, -r13.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r16.zw = ((-(r13.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 161: mul r5.w, r13.y, l(5.000000)
    r5.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 162: add r6.w, r0.x, r13.x
    r6.w = ((r0.xxxx)+(r13.xxxx)).w;
    // 163: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 164: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 165: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 166: add r0.x, r0.x, r3.w
    r0.x = ((r0.xxxx)+(r3.wwww)).x;
    // 167: add_sat r0.x, r0.x, l(-1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 168: mov_sat r3.w, cb0[12].x
    r3.w = (saturate(source[12].xxxx)).w;
    // 169: mad r17.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r17.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 170: mul r3.w, r3.w, l(0.080000)
    r3.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 171: mad r17.xyz, r0.zzzz, r17.xyzx, r3.wwww
    r17.xyz = ((r0.zzzz)*(r17.xyzx)+(r3.wwww)).xyz;
    // 172: max r18.xyz, r16.zzzz, r17.xyzx
    r18.xyz = (max(r16.zzzz,r17.xyzx)).xyz;
    // 173: add r18.xyz, -r17.xyzx, r18.xyzx
    r18.xyz = ((-(r17.xyzx))+(r18.xyzx)).xyz;
    // 174: mul_sat r3.w, r17.y, l(50.000000)
    r3.w = (saturate((r17.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 175: mul r18.xyz, r3.wwww, r18.xyzx
    r18.xyz = ((r3.wwww)*(r18.xyzx)).xyz;
    // 176: mul r19.xyz, r16.yyyy, r17.xyzx
    r19.xyz = ((r16.yyyy)*(r17.xyzx)).xyz;
    // 177: mad r18.xyz, r18.xyzx, r16.xxxx, r19.xyzx
    r18.xyz = ((r18.xyzx)*(r16.xxxx)+(r19.xyzx)).xyz;
    // 178: div r6.w, l(1.000000, 1.000000, 1.000000, 1.000000), r16.y
    r6.w = r16.y != 0.f ? 1.f / r16.y : 0.f;
    // 179: add r6.w, r6.w, l(-1.000000)
    r6.w = ((r6.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 180: mad r16.xyz, r17.xyzx, r6.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((r17.xyzx)*(r6.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mad r19.xyz, -r18.xyzx, r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r19.xyz = ((-(r18.xyzx))*(r16.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: mul r16.xyz, r16.xyzx, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r18.xyzx)).xyz;
    // 183: mul r6.w, r16.w, r16.w
    r6.w = ((r16.wwww)*(r16.wwww)).w;
    // 184: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 185: mul r7.w, r16.w, r6.w
    r7.w = ((r16.wwww)*(r6.wwww)).w;
    // 186: mad r6.w, -r6.w, r16.w, l(1.000000)
    r6.w = ((-(r6.wwww))*(r16.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: mul r18.xyz, r17.xyzx, r6.wwww
    r18.xyz = ((r17.xyzx)*(r6.wwww)).xyz;
    // 188: dp3 r6.w, r17.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r17.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: mad r17.xyz, r6.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r17.xyz = ((r6.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 190: mad r18.xyz, r3.wwww, r7.wwww, r18.xyzx
    r18.xyz = ((r3.wwww)*(r7.wwww)+(r18.xyzx)).xyz;
    // 191: add r18.xyz, -r18.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r18.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r18.xyz, r18.xyzx, r18.xyzx
    r18.xyz = ((r18.xyzx)*(r18.xyzx)).xyz;
    // 193: mad r20.xyz, -r1.wwww, r18.xyzx, r19.xyzx
    r20.xyz = ((-(r1.wwww))*(r18.xyzx)+(r19.xyzx)).xyz;
    // 194: mul r19.xyz, r1.xyzx, r19.xyzx
    r19.xyz = ((r1.xyzx)*(r19.xyzx)).xyz;
    // 195: mul r18.xyz, r1.wwww, r18.xyzx
    r18.xyz = ((r1.wwww)*(r18.xyzx)).xyz;
    // 196: mul r18.xyz, r1.xyzx, r18.xyzx
    r18.xyz = ((r1.xyzx)*(r18.xyzx)).xyz;
    // 197: mad r18.xyz, -r18.xyzx, r0.zzzz, r18.xyzx
    r18.xyz = ((-(r18.xyzx))*(r0.zzzz)+(r18.xyzx)).xyz;
    // 198: mul r7.xyz, r7.xyzx, r20.xyzx
    r7.xyz = ((r7.xyzx)*(r20.xyzx)).xyz;
    // 199: mad r7.xyz, -r7.xyzx, r0.zzzz, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.zzzz)+(r7.xyzx)).xyz;
    // 200: dp2_sat r20.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r20.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 201: dp3_sat r20.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r20.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 202: dp3_sat r20.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r20.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 203: mul r20.xyz, r20.xyzx, r20.xyzx
    r20.xyz = ((r20.xyzx)*(r20.xyzx)).xyz;
    // 204: dp3 r3.w, r9.xyzx, r20.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r20.xyzx).xyz).xxxx).w;
    // 205: add r0.w, r0.w, -r3.w
    r0.w = ((r0.wwww)+(-(r3.wwww))).w;
    // 206: mad r0.y, r0.y, r0.w, r3.w
    r0.y = ((r0.yyyy)*(r0.wwww)+(r3.wwww)).y;
    // 207: mad r6.xyz, r8.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 208: mad r0.y, r0.x, r17.x, r17.y
    r0.y = ((r0.xxxx)*(r17.xxxx)+(r17.yyyy)).y;
    // 209: mad r0.y, r0.y, r0.x, r17.z
    r0.y = ((r0.yyyy)*(r0.xxxx)+(r17.zzzz)).y;
    // 210: mul r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)*(r0.yyyy)).y;
    // 211: max r0.x, r0.y, r0.x
    r0.x = (max(r0.yyyy,r0.xxxx)).x;
    // 212: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 213: dp3 r11.x, r11.xyzx, r5.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 214: dp3 r11.y, r12.xyzx, r5.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 215: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 216: dp2 r5.x, r11.xyxx, r13.zwzz
    r5.x = (dot((r11.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 217: dp2 r5.z, r11.xyxx, cb0[17].xyxx
    r5.z = (dot((r11.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 218: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r5.xyzx, t7.xyzw, s7, r5.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 219: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 220: mul r5.xyz, r5.xyzx, cb0[16].xyzx
    r5.xyz = ((r5.xyzx)*(source[16].xyzx)).xyz;
    // 221: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 222: dp3 r0.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 223: div r0.y, r0.y, r3.z
    r0.y = ((r0.yyyy)/(r3.zzzz)).y;
    // 224: mad r0.y, r4.w, l(5.000000), r0.y
    r0.y = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.yyyy)).y;
    // 225: add_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)+(r0.yyyy))).y;
    // 226: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 227: mad r0.w, r0.y, l(-2.000000), l(3.000000)
    r0.w = ((r0.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 228: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 229: mul r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)*(r0.wwww)).y;
    // 230: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 231: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 232: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 233: mul r5.xyz, r0.yyyy, r5.xyzx
    r5.xyz = ((r0.yyyy)*(r5.xyzx)).xyz;
    // 234: mul r6.xyz, r6.xyzx, r5.xyzx
    r6.xyz = ((r6.xyzx)*(r5.xyzx)).xyz;
    // 235: mul r5.xyz, r5.xyzx, r16.xyzx
    r5.xyz = ((r5.xyzx)*(r16.xyzx)).xyz;
    // 236: mad r6.xyz, r6.xyzx, r16.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r16.xyzx)+(r7.xyzx)).xyz;
    // 237: mul r3.yzw, r3.yyyy, cb0[26].xxyz
    r3.yzw = ((r3.yyyy)*(source[26].xxyz)).yzw;
    // 238: mad r3.xyz, r3.xxxx, cb0[25].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[25].xyzx)+(r3.yzwy)).xyz;
    // 239: mul r3.xyz, r3.xyzx, cb0[27].wwww
    r3.xyz = ((r3.xyzx)*(source[27].wwww)).xyz;
    // 240: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 241: add_sat r7.xyz, -r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = (saturate((-(r7.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 242: add_sat r7.xyz, r7.xyzx, cb0[14].wwww
    r7.xyz = (saturate((r7.xyzx)+(source[14].wwww))).xyz;
    // 243: mul r0.y, r7.x, cb0[15].x
    r0.y = ((r7.xxxx)*(source[15].xxxx)).y;
    // 244: mul_sat r7.xyz, r7.xyzx, cb0[9].xyzx
    r7.xyz = (saturate((r7.xyzx)*(source[9].xyzx))).xyz;
    // 245: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 246: mul r7.xyz, r7.xyzx, r0.yyyy
    r7.xyz = ((r7.xyzx)*(r0.yyyy)).xyz;
    // 247: mul r7.xyz, r0.zzzz, r7.xyzx
    r7.xyz = ((r0.zzzz)*(r7.xyzx)).xyz;
    // 248: mul r0.yzw, r0.zzzz, r19.xxyz
    r0.yzw = ((r0.zzzz)*(r19.xxyz)).yzw;
    // 249: mul r0.yzw, r14.xxyz, r0.yyzw
    r0.yzw = ((r14.xxyz)*(r0.yyzw)).yzw;
    // 250: mul r0.yzw, r2.xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(r0.yyzw)).yzw;
    // 251: mad r0.xyz, r5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((r5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 252: mul r2.xyz, r3.xyzx, r7.xyzx
    r2.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 253: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 254: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 255: mul r3.xyz, r0.wwww, r8.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 256: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 257: mad r1.xyz, r1.xyzx, r3.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 258: dp3 r0.w, r4.xyzx, r15.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r15.xyzx).xyz).xxxx).w;
    // 259: add r2.x, -|r15.z|, l(1.000000)
    r2.x = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 260: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 261: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 262: lt r2.x, |r0.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 263: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 264: mul r0.xyzw, r0.xyzw, l(0.300000, 0.300000, 0.300000, 1.500000)
    r0.xyzw = ((r0.xyzw)*(float4(0.300000,0.300000,0.300000,1.500000))).xyzw;
    // 265: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 266: mul r2.yzw, r0.wwww, cb0[4].xxyz
    r2.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 267: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 268: mul r3.xy, v4.xyxx, cb0[5].xyxx
    r3.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 269: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t4.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 270: mul r4.xyz, cb0[6].xyzx, cb0[10].yyyy
    r4.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 271: mad r2.xyz, r3.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 272: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 273: add r0.w, -r1.w, l(1.000000)
    r0.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 274: mad r2.xyz, r0.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 275: mul r0.xyz, r1.wwww, r0.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)).xyz;
    // 276: mul r3.xyz, r1.wwww, r1.xyzx
    r3.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 277: mad r1.xyz, r1.xyzx, r0.wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 278: add r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)+(r6.xyzx)).xyz;
    // 279: mad o0.xyz, v5.wwww, r1.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r1.xyzx)+(v5.xyzx)).xyz;
    // 280: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r18.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r18.xyzx)).xyz;
    // 281: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 282: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 283: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 284: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 285: ret
    return output;
}

// source.character.static-map-native-1142.v1 / source program e84da85b599d8547b9c749f792484338
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1142(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1142(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    source[13]=g_SourceCharacterBaseConstants[10];
    source[14]=g_SourceCharacterBaseConstants[11];
    source[15]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 3: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 4: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 5: mul r0.w, r0.w, cb0[11].y
    r0.w = ((r0.wwww)*(source[11].yyyy)).w;
    // 6: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 7: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 8: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 9: mul r1.xyz, cb0[8].xyzx, cb0[10].wwww
    r1.xyz = ((source[8].xyzx)*(source[10].wwww)).xyz;
    // 10: mul r2.xyz, cb0[7].xyzx, cb0[10].zzzz
    r2.xyz = ((source[7].xyzx)*(source[10].zzzz)).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 13: mad r1.xyz, r1.xyzx, r3.xyzx, -r2.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(-(r2.xyzx))).xyz;
    // 14: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 15: mul r2.xyz, r1.xyzx, cb0[11].zzzz
    r2.xyz = ((r1.xyzx)*(source[11].zzzz)).xyz;
    // 16: mad r1.xyz, cb0[11].wwww, r1.xyzx, -r2.xyzx
    r1.xyz = ((source[11].wwww)*(r1.xyzx)+(-(r2.xyzx))).xyz;
    // 17: mad r1.xyz, r0.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 18: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 19: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 20: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 21: mad r2.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 22: mad r3.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 23: mul r0.x, r0.x, cb0[14].x
    r0.x = ((r0.xxxx)*(source[14].xxxx)).x;
    // 24: mul r0.y, r0.y, cb0[13].z
    r0.y = ((r0.yyyy)*(source[13].zzzz)).y;
    // 25: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 26: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 27: mul r0.w, r0.w, cb0[14].y
    r0.w = ((r0.wwww)*(source[14].yyyy)).w;
    // 28: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 29: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 31: mad r2.xyz, r0.xxxx, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 32: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 33: mad r2.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 34: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 35: max r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = (max(r0.xxxx,r2.xyzx)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 37: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 38: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 39: mul r3.xy, r3.xyxx, cb0[10].xxxx
    r3.xy = ((r3.xyxx)*(source[10].xxxx)).xy;
    // 40: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 41: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 44: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 48: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 51: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r5.xyz, r0.wwww, v7.xyzx
    r5.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 54: dp3 r0.w, r5.xyzx, r4.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: mad r6.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 56: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 57: mul r6.yzw, r6.yyyy, cb0[26].xxyz
    r6.yzw = ((r6.yyyy)*(source[26].xxyz)).yzw;
    // 58: mad r6.xyz, r6.xxxx, cb0[25].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[25].xyzx)+(r6.yzwy)).xyz;
    // 59: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 60: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 61: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 62: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 63: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 64: mul r7.xyz, r0.wwww, v1.xyzx
    r7.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 65: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r8.xyz, r0.wwww, v0.xyzx
    r8.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 68: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 69: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 70: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 71: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 72: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 73: dp2 r11.z, r10.xyxx, cb0[17].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 74: mul r10.zw, cb0[17].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r10.zw = ((source[17].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 75: dp2 r11.x, r10.xyxx, r10.zwzz
    r11.x = (dot((r10.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 76: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 77: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 78: dp4 r12.x, cb0[18].xyzw, r11.xyzw
    r12.x = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 79: dp4 r12.y, cb0[19].xyzw, r11.xyzw
    r12.y = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 80: dp4 r12.z, cb0[20].xyzw, r11.xyzw
    r12.z = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 81: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 82: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 83: mad r0.w, r11.x, r11.x, -r0.w
    r0.w = ((r11.xxxx)*(r11.xxxx)+(-(r0.wwww))).w;
    // 84: dp4 r11.x, cb0[21].xyzw, r13.xyzw
    r11.x = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 85: dp4 r11.y, cb0[22].xyzw, r13.xyzw
    r11.y = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 86: dp4 r11.z, cb0[23].xyzw, r13.xyzw
    r11.z = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 87: add r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)+(r12.xyzx)).xyz;
    // 88: mad r11.xyz, cb0[24].xyzx, r0.wwww, r11.xyzx
    r11.xyz = ((source[24].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 89: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: mul r11.xyz, r11.xyzx, cb0[16].xyzx
    r11.xyz = ((r11.xyzx)*(source[16].xyzx)).xyz;
    // 91: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 92: dp3 r0.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 94: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 95: mul r1.w, r1.w, cb0[13].w
    r1.w = ((r1.wwww)*(source[13].wwww)).w;
    // 96: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 97: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 98: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 99: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 100: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 101: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 102: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 103: dp3 r1.w, r4.xyzx, r12.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 104: deriv_rtx_coarse r10.x, r1.w
    r10.x = (ddx_coarse(r1.wwww)).x;
    // 105: deriv_rty_coarse r10.y, r1.w
    r10.y = (ddy_coarse(r1.wwww)).y;
    // 106: dp2 r2.w, r10.xyxx, r10.xyxx
    r2.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 107: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 108: mad_sat r10.y, r2.w, l(0.300000), r0.y
    r10.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.yyyy))).y;
    // 109: mad r0.y, r10.y, l(0.200000), l(0.200000)
    r0.y = ((r10.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 110: div r0.w, r0.w, r0.y
    r0.w = ((r0.wwww)/(r0.yyyy)).w;
    // 111: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 112: mad r0.w, r2.w, l(5.000000), r0.w
    r0.w = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).w;
    // 113: add r4.w, r3.w, cb0[13].x
    r4.w = ((r3.wwww)+(source[13].xxxx)).w;
    // 114: add r3.w, r3.w, cb0[12].z
    r3.w = ((r3.wwww)+(source[12].zzzz)).w;
    // 115: sample_b_indexable(texture2d)(float,float,float,float) r5.w, v4.xyxx, t2.yzwx, s4, l(0.000000)
    r5.w = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 116: mad r4.w, r5.w, -r4.w, r4.w
    r4.w = ((r5.wwww)*(-(r4.wwww))+(r4.wwww)).w;
    // 117: add_sat r0.z, r0.z, r4.w
    r0.z = (saturate((r0.zzzz)+(r4.wwww))).z;
    // 118: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 119: add_sat r0.w, r0.z, r0.w
    r0.w = (saturate((r0.zzzz)+(r0.wwww))).w;
    // 120: mad r4.w, r0.w, l(-2.000000), l(3.000000)
    r4.w = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 121: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 122: mul r0.w, r0.w, r4.w
    r0.w = ((r0.wwww)*(r4.wwww)).w;
    // 123: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 124: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 125: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 126: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 127: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 128: mul r13.xyz, r1.wwww, r4.xyzx
    r13.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 129: dp3 r0.w, -r5.xyzx, r4.xyzx
    r0.w = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).w;
    // 130: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 131: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 132: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 133: add r0.w, r13.z, l(1.000000)
    r0.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 134: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: add r4.z, r1.w, l(1.000000)
    r4.z = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 136: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 137: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 138: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 139: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 140: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 141: add_sat r10.x, -r0.w, r4.z
    r10.x = (saturate((-(r0.wwww))+(r4.zzzz))).x;
    // 142: sample_indexable(texture2d)(float,float,float,float) r4.zw, r10.xyxx, t6.zwxy, s7
    r4.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 143: add r14.xy, -r10.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r14.xy = ((-(r10.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 144: add r0.w, r0.x, r10.x
    r0.w = ((r0.xxxx)+(r10.xxxx)).w;
    // 145: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 146: mov_sat r6.w, cb0[12].x
    r6.w = (saturate(source[12].xxxx)).w;
    // 147: mad r15.xyz, -r6.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r15.xyz = ((-(r6.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 148: mul r6.w, r6.w, l(0.080000)
    r6.w = ((r6.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 149: mad r15.xyz, r0.zzzz, r15.xyzx, r6.wwww
    r15.xyz = ((r0.zzzz)*(r15.xyzx)+(r6.wwww)).xyz;
    // 150: max r14.xzw, r14.xxxx, r15.xxyz
    r14.xzw = (max(r14.xxxx,r15.xxyz)).xzw;
    // 151: add r14.xzw, -r15.xxyz, r14.xxzw
    r14.xzw = ((-(r15.xxyz))+(r14.xxzw)).xzw;
    // 152: mul_sat r6.w, r15.y, l(50.000000)
    r6.w = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 153: mul r14.xzw, r6.wwww, r14.xxzw
    r14.xzw = ((r6.wwww)*(r14.xxzw)).xzw;
    // 154: mul r16.xyz, r4.wwww, r15.xyzx
    r16.xyz = ((r4.wwww)*(r15.xyzx)).xyz;
    // 155: mad r14.xzw, r14.xxzw, r4.zzzz, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r4.zzzz)+(r16.xxyz)).xzw;
    // 156: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r4.z = r4.w != 0.f ? 1.f / r4.w : 0.f;
    // 157: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 158: mad r16.xyz, r15.xyzx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((r15.xyzx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 159: mad r17.xyz, -r14.xzwx, r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r14.xzwx))*(r16.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mul r14.xzw, r14.xxzw, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r16.xxyz)).xzw;
    // 161: mul r4.z, r14.y, r14.y
    r4.z = ((r14.yyyy)*(r14.yyyy)).z;
    // 162: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 163: mul r4.w, r14.y, r4.z
    r4.w = ((r14.yyyy)*(r4.zzzz)).w;
    // 164: mad r4.z, -r4.z, r14.y, l(1.000000)
    r4.z = ((-(r4.zzzz))*(r14.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 165: mul r16.xyz, r15.xyzx, r4.zzzz
    r16.xyz = ((r15.xyzx)*(r4.zzzz)).xyz;
    // 166: dp3 r4.z, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.z = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 167: mad r15.xyz, r4.zzzz, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r4.zzzz)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 168: mad r16.xyz, r6.wwww, r4.wwww, r16.xyzx
    r16.xyz = ((r6.wwww)*(r4.wwww)+(r16.xyzx)).xyz;
    // 169: add r16.xyz, -r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r16.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r16.xyz, r16.xyzx, r16.xyzx
    r16.xyz = ((r16.xyzx)*(r16.xyzx)).xyz;
    // 171: add r4.z, -r3.w, l(1.000000)
    r4.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 172: mad_sat r3.w, r5.w, r4.z, r3.w
    r3.w = (saturate((r5.wwww)*(r4.zzzz)+(r3.wwww))).w;
    // 173: mad r3.w, -r3.w, cb0[2].x, l(1.000000)
    r3.w = ((-(r3.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: mul r4.z, r10.y, r10.y
    r4.z = ((r10.yyyy)*(r10.yyyy)).z;
    // 175: mul r4.w, r10.y, l(5.000000)
    r4.w = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 176: mad r5.w, r4.z, l(0.350000), l(1.000000)
    r5.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r0.w, r0.w, r4.z
    r0.w = ((r0.wwww)*(r4.zzzz)).w;
    // 178: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 179: add r0.x, r0.x, r0.w
    r0.x = ((r0.xxxx)+(r0.wwww)).x;
    // 180: add_sat r0.x, r0.x, l(-1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 181: div_sat r0.w, r3.w, r5.w
    r0.w = (saturate((r3.wwww)/(r5.wwww))).w;
    // 182: mad r18.xyz, -r0.wwww, r16.xyzx, r17.xyzx
    r18.xyz = ((-(r0.wwww))*(r16.xyzx)+(r17.xyzx)).xyz;
    // 183: mul r17.xyz, r1.xyzx, r17.xyzx
    r17.xyz = ((r1.xyzx)*(r17.xyzx)).xyz;
    // 184: mul r16.xyz, r16.xyzx, r0.wwww
    r16.xyz = ((r16.xyzx)*(r0.wwww)).xyz;
    // 185: mul r16.xyz, r1.xyzx, r16.xyzx
    r16.xyz = ((r1.xyzx)*(r16.xyzx)).xyz;
    // 186: mad r16.xyz, -r16.xyzx, r0.zzzz, r16.xyzx
    r16.xyz = ((-(r16.xyzx))*(r0.zzzz)+(r16.xyzx)).xyz;
    // 187: mul r6.xyz, r6.xyzx, r18.xyzx
    r6.xyz = ((r6.xyzx)*(r18.xyzx)).xyz;
    // 188: mad r6.xyz, -r6.xyzx, r0.zzzz, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.zzzz)+(r6.xyzx)).xyz;
    // 189: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 190: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 191: dp2 r9.x, r8.xyxx, r10.zwzz
    r9.x = (dot((r8.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 192: dp2 r9.z, r8.xyxx, cb0[17].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 193: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 194: dp3 r3.w, r5.xyzx, r13.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 195: mad r5.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 196: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 197: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t7.xyzw, s6, r4.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r4.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 198: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 199: mul r7.xyz, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r7.xyzx)*(source[16].xyzx)).xyz;
    // 200: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 201: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 202: div r0.y, r3.w, r0.y
    r0.y = ((r3.wwww)/(r0.yyyy)).y;
    // 203: mad r0.y, r2.w, l(5.000000), r0.y
    r0.y = ((r2.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.yyyy)).y;
    // 204: add_sat r0.y, r0.z, r0.y
    r0.y = (saturate((r0.zzzz)+(r0.yyyy))).y;
    // 205: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 206: mad r2.w, r0.y, l(-2.000000), l(3.000000)
    r2.w = ((r0.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 207: mul r0.y, r0.y, r0.y
    r0.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 208: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 209: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 210: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 211: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 212: mul r7.xyz, r0.yyyy, r7.xyzx
    r7.xyz = ((r0.yyyy)*(r7.xyzx)).xyz;
    // 213: mad r0.y, r0.x, r15.x, r15.y
    r0.y = ((r0.xxxx)*(r15.xxxx)+(r15.yyyy)).y;
    // 214: mad r0.y, r0.y, r0.x, r15.z
    r0.y = ((r0.yyyy)*(r0.xxxx)+(r15.zzzz)).y;
    // 215: mul r0.y, r0.x, r0.y
    r0.y = ((r0.xxxx)*(r0.yyyy)).y;
    // 216: max r0.x, r0.y, r0.x
    r0.x = (max(r0.yyyy,r0.xxxx)).x;
    // 217: mul r5.yzw, r5.yyyy, cb0[26].xxyz
    r5.yzw = ((r5.yyyy)*(source[26].xxyz)).yzw;
    // 218: mad r5.xyz, cb0[25].xyzx, r5.xxxx, r5.yzwy
    r5.xyz = ((source[25].xyzx)*(r5.xxxx)+(r5.yzwy)).xyz;
    // 219: mul r5.xyz, r5.xyzx, cb0[27].wwww
    r5.xyz = ((r5.xyzx)*(source[27].wwww)).xyz;
    // 220: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 221: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 222: mul r7.xyz, r7.xyzx, r14.xzwx
    r7.xyz = ((r7.xyzx)*(r14.xzwx)).xyz;
    // 223: mad r5.xyz, r5.xyzx, r14.xzwx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r14.xzwx)+(r6.xyzx)).xyz;
    // 224: dp3 r0.y, r3.xyzx, r12.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 225: add r2.w, -|r12.z|, l(1.000000)
    r2.w = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 227: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 228: lt r2.w, |r0.y|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 229: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 230: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 231: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 232: mul r3.xyz, r0.yyyy, cb0[4].xyzx
    r3.xyz = ((r0.yyyy)*(source[4].xyzx)).xyz;
    // 233: movc r3.xyz, r2.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 234: mul r4.zw, v4.xxxy, cb0[5].xxxy
    r4.zw = ((v4.xxxy)*(source[5].xxxy)).zw;
    // 235: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r4.zwzz, t4.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 236: mul r8.xyz, cb0[6].xyzx, cb0[10].yyyy
    r8.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 237: mad r3.xyz, r6.xyzx, r8.xyzx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 238: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 239: mul r6.xyz, r0.zzzz, r17.xyzx
    r6.xyz = ((r0.zzzz)*(r17.xyzx)).xyz;
    // 240: mul r6.xyz, r11.xyzx, r6.xyzx
    r6.xyz = ((r11.xyzx)*(r6.xyzx)).xyz;
    // 241: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 242: mad r2.xyz, r7.xyzx, r0.xxxx, r2.xyzx
    r2.xyz = ((r7.xyzx)*(r0.xxxx)+(r2.xyzx)).xyz;
    // 243: mul r2.xyz, r2.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 244: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 245: mad r3.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 246: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 247: mad r2.xyz, r2.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r16.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r16.xyzx)).xyz;
    // 248: mul r4.yzw, r4.yyyy, cb0[26].xxyz
    r4.yzw = ((r4.yyyy)*(source[26].xxyz)).yzw;
    // 249: mad r4.xyz, r4.xxxx, cb0[25].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[25].xyzx)+(r4.yzwy)).xyz;
    // 250: mul r4.xyz, r4.xyzx, cb0[27].wwww
    r4.xyz = ((r4.xyzx)*(source[27].wwww)).xyz;
    // 251: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 252: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 253: add_sat r6.xyz, r6.xyzx, cb0[14].wwww
    r6.xyz = (saturate((r6.xyzx)+(source[14].wwww))).xyz;
    // 254: mul r0.y, r6.x, cb0[15].x
    r0.y = ((r6.xxxx)*(source[15].xxxx)).y;
    // 255: mul_sat r6.xyz, r6.xyzx, cb0[9].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[9].xyzx))).xyz;
    // 256: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 257: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 258: mul r6.xyz, r0.zzzz, r6.xyzx
    r6.xyz = ((r0.zzzz)*(r6.xyzx)).xyz;
    // 259: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 260: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 261: mad r0.xyz, r1.xyzx, r0.xxxx, r3.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 262: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 263: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r2.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r2.xyzx)).xyz;
    // 264: mul o1.xyz, r1.xyzx, v5.wwww
    output.targets[1].xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 265: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 266: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 267: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 268: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 269: ret
    return output;
}

// source.character.static-map-native-1143.v1 / source program 33997f4065be2245a2f2670fd40a607b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1143(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[25]=1.f;
    source[26]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f, r19=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(0.900000)
    r1.x = ((r0.wwww)+(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 3: round_ni_sat r1.x, r1.x
    r1.x = (saturate(floor(r1.xxxx))).x;
    // 4: add r1.x, r1.x, l(-0.333300)
    r1.x = ((r1.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 5: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 6: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 7: mul r1.xyz, cb0[6].xyzx, cb0[8].zzzz
    r1.xyz = ((source[6].xyzx)*(source[8].zzzz)).xyz;
    // 8: mul r2.xyz, cb0[5].xyzx, cb0[8].yyyy
    r2.xyz = ((source[5].xyzx)*(source[8].yyyy)).xyz;
    // 9: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 11: add r1.xy, r0.wwww, cb0[10].wyww
    r1.xy = ((r0.wwww)+(source[10].wyww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 13: mul r0.w, r3.z, cb0[8].w
    r0.w = ((r3.zzzz)*(source[8].wwww)).w;
    // 14: mul r1.zw, r3.yyyx, cb0[11].yyyw
    r1.zw = ((r3.yyyx)*(source[11].yyyw)).zw;
    // 15: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 16: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r2.w, r2.w, cb0[9].x
    r2.w = ((r2.wwww)*(source[9].xxxx)).w;
    // 18: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 19: movc r0.w, r0.w, l(0), r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 20: min r2.w, r0.w, l(1.000000)
    r2.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 22: mul r2.xyz, r0.xyzx, cb0[9].yyyy
    r2.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 23: mad r0.xyz, cb0[9].zzzz, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[9].zzzz)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 24: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 25: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 26: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 27: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 28: mad r2.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r2.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 29: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 30: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 31: log r5.xy, |r1.zwzz|
    r5.xy = (log2(abs(r1.zwzz))).xy;
    // 32: lt r1.zw, |r1.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r1.zw = (asfloat((uint4)((abs(r1.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 33: mul r2.w, r5.y, cb0[12].x
    r2.w = ((r5.yyyy)*(source[12].xxxx)).w;
    // 34: mul r3.w, r5.x, cb0[11].z
    r3.w = ((r5.xxxx)*(source[11].zzzz)).w;
    // 35: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 36: movc r1.z, r1.z, l(0), r3.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).z;
    // 37: max r1.z, r1.z, cb0[0].x
    r1.z = (max(r1.zzzz,source[0].xxxx)).z;
    // 38: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 40: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 42: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 43: mad r2.xyz, r3.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 44: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 45: max r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = (max(r1.wwww,r2.xyzx)).xyz;
    // 46: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 47: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 48: mul r3.xyz, r2.wwww, v7.xyzx
    r3.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 50: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 51: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 52: mul r4.xy, r4.xyxx, cb0[8].xxxx
    r4.xy = ((r4.xyxx)*(source[8].xxxx)).xy;
    // 53: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 54: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 56: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 57: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 58: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 59: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 60: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 61: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r5.xyz, r2.wwww, r4.xyzx
    r5.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 64: dp3 r2.w, r3.xyzx, r5.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 65: dp3 r3.x, -r3.xyzx, r5.xyzx
    r3.x = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).x;
    // 66: mad r3.xy, r3.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r3.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: mad r3.zw, r2.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r3.zw = ((r2.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 68: mul r3.xyzw, r3.xyzw, r3.xyzw
    r3.xyzw = ((r3.xyzw)*(r3.xyzw)).xyzw;
    // 69: mul r6.xyz, r3.wwww, cb0[23].xyzx
    r6.xyz = ((r3.wwww)*(source[23].xyzx)).xyz;
    // 70: mad r6.xyz, r3.zzzz, cb0[22].xyzx, r6.xyzx
    r6.xyz = ((r3.zzzz)*(source[22].xyzx)+(r6.xyzx)).xyz;
    // 71: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 72: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 73: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 74: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 75: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 76: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 77: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s5
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 78: mul r9.xyz, r9.xyzx, cb0[26].xyzx
    r9.xyz = ((r9.xyzx)*(source[26].xyzx)).xyz;
    // 79: dp3 r2.w, r9.xyzx, r8.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 80: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 81: mul r8.xyz, r8.xyzx, cb0[25].xyzx
    r8.xyz = ((r8.xyzx)*(source[25].xyzx)).xyz;
    // 82: mul r10.xyz, r2.wwww, r8.xyzx
    r10.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 83: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 84: mul r7.xyz, r2.xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 85: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 86: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 87: mul r10.xyz, r3.zzzz, v1.xyzx
    r10.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 88: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 89: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 90: mul r11.xyz, r3.zzzz, v0.xyzx
    r11.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 91: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 92: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 93: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 94: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 95: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 96: dp2 r14.z, r13.xyxx, cb0[14].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 97: mul r3.zw, cb0[14].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[14].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 98: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 99: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 100: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 101: dp4 r13.x, cb0[15].xyzw, r14.xyzw
    r13.x = (dot((source[15].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 102: dp4 r13.y, cb0[16].xyzw, r14.xyzw
    r13.y = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 103: dp4 r13.z, cb0[17].xyzw, r14.xyzw
    r13.z = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 104: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 105: mul r4.w, r14.y, r14.y
    r4.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 106: mad r4.w, r14.x, r14.x, -r4.w
    r4.w = ((r14.xxxx)*(r14.xxxx)+(-(r4.wwww))).w;
    // 107: dp4 r14.x, cb0[18].xyzw, r15.xyzw
    r14.x = (dot((source[18].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 108: dp4 r14.y, cb0[19].xyzw, r15.xyzw
    r14.y = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 109: dp4 r14.z, cb0[20].xyzw, r15.xyzw
    r14.z = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 110: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 111: mad r13.xyz, cb0[21].xyzx, r4.wwww, r13.xyzx
    r13.xyz = ((source[21].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 112: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 113: mul r13.xyz, r13.xyzx, cb0[13].xyzx
    r13.xyz = ((r13.xyzx)*(source[13].xyzx)).xyz;
    // 114: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 115: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: dp3 r5.w, v6.xyzx, v6.xyzx
    r5.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 117: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 118: mul r14.xyz, r5.wwww, v6.xyzx
    r14.xyz = ((r5.wwww)*(v6.xyzx)).xyz;
    // 119: dp3 r5.w, r5.xyzx, r14.xyzx
    r5.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 120: mul r5.xyz, r5.wwww, r5.xyzx
    r5.xyz = ((r5.wwww)*(r5.xyzx)).xyz;
    // 121: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 122: deriv_rtx_coarse r15.x, r5.w
    r15.x = (ddx_coarse(r5.wwww)).x;
    // 123: deriv_rty_coarse r15.y, r5.w
    r15.y = (ddy_coarse(r5.wwww)).y;
    // 124: dp2 r6.w, r15.xyxx, r15.xyxx
    r6.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 125: sqrt r6.w, r6.w
    r6.w = (sqrt(r6.wwww)).w;
    // 126: mad_sat r15.y, r6.w, l(0.300000), r1.z
    r15.y = (saturate((r6.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 127: mad r6.w, r15.y, l(0.200000), l(0.200000)
    r6.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 128: div r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)/(r6.wwww)).w;
    // 129: dp3 r7.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r7.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r4.w, r7.w, l(5.000000), r4.w
    r4.w = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 131: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r8.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 132: mad r1.x, r8.w, -r1.x, r1.x
    r1.x = ((r8.wwww)*(-(r1.xxxx))+(r1.xxxx)).x;
    // 133: add_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)+(r1.xxxx))).w;
    // 134: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 135: add_sat r1.x, r0.w, r4.w
    r1.x = (saturate((r0.wwww)+(r4.wwww))).x;
    // 136: mad r4.w, r1.x, l(-2.000000), l(3.000000)
    r4.w = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 137: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 138: mul r1.x, r1.x, r4.w
    r1.x = ((r1.xxxx)*(r4.wwww)).x;
    // 139: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 140: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 141: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 142: mul r13.xyz, r1.xxxx, r13.xyzx
    r13.xyz = ((r1.xxxx)*(r13.xyzx)).xyz;
    // 143: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 144: add r1.x, -r1.y, l(1.000000)
    r1.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 145: mad_sat r1.x, r8.w, r1.x, r1.y
    r1.x = (saturate((r8.wwww)*(r1.xxxx)+(r1.yyyy))).x;
    // 146: mad r1.x, -r1.x, cb0[2].x, l(1.000000)
    r1.x = ((-(r1.xxxx))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 147: mul r1.y, r15.y, r15.y
    r1.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 148: mad r4.w, r1.y, l(0.350000), l(1.000000)
    r4.w = ((r1.yyyy)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 149: div_sat r1.x, r1.x, r4.w
    r1.x = (saturate((r1.xxxx)/(r4.wwww))).x;
    // 150: add r4.w, r5.w, l(1.000000)
    r4.w = ((r5.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mov_sat r5.w, r5.w
    r5.w = (saturate(r5.wwww)).w;
    // 152: log r5.w, r5.w
    r5.w = (log2(r5.wwww)).w;
    // 153: mul r5.w, r5.w, cb0[1].y
    r5.w = ((r5.wwww)*(source[1].yyyy)).w;
    // 154: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 155: mad_sat r5.w, r5.w, cb0[1].w, cb0[1].z
    r5.w = (saturate((r5.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 156: add r8.w, r5.z, l(1.000000)
    r8.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: add_sat r15.x, r4.w, -r8.w
    r15.x = (saturate((r4.wwww)+(-(r8.wwww)))).x;
    // 159: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 160: add r16.xy, -r15.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r16.xy = ((-(r15.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 161: mul r4.w, r15.y, l(5.000000)
    r4.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 162: add r8.w, r1.w, r15.x
    r8.w = ((r1.wwww)+(r15.xxxx)).w;
    // 163: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 164: mul r1.y, r1.y, r8.w
    r1.y = ((r1.yyyy)*(r8.wwww)).y;
    // 165: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 166: add r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)+(r1.yyyy)).y;
    // 167: add_sat r1.y, r1.y, l(-1.000000)
    r1.y = (saturate((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 168: mov_sat r1.w, cb0[9].w
    r1.w = (saturate(source[9].wwww)).w;
    // 169: mad r17.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r17.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 170: mul r1.w, r1.w, l(0.080000)
    r1.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 171: mad r17.xyz, r0.wwww, r17.xyzx, r1.wwww
    r17.xyz = ((r0.wwww)*(r17.xyzx)+(r1.wwww)).xyz;
    // 172: max r16.xzw, r16.xxxx, r17.xxyz
    r16.xzw = (max(r16.xxxx,r17.xxyz)).xzw;
    // 173: add r16.xzw, -r17.xxyz, r16.xxzw
    r16.xzw = ((-(r17.xxyz))+(r16.xxzw)).xzw;
    // 174: mul_sat r1.w, r17.y, l(50.000000)
    r1.w = (saturate((r17.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 175: mul r16.xzw, r1.wwww, r16.xxzw
    r16.xzw = ((r1.wwww)*(r16.xxzw)).xzw;
    // 176: mul r18.xyz, r15.wwww, r17.xyzx
    r18.xyz = ((r15.wwww)*(r17.xyzx)).xyz;
    // 177: mad r15.xyz, r16.xzwx, r15.zzzz, r18.xyzx
    r15.xyz = ((r16.xzwx)*(r15.zzzz)+(r18.xyzx)).xyz;
    // 178: div r8.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r8.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 179: add r8.w, r8.w, l(-1.000000)
    r8.w = ((r8.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 180: mad r16.xzw, r17.xxyz, r8.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r16.xzw = ((r17.xxyz)*(r8.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 181: mad r18.xyz, -r15.xyzx, r16.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r16.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: mul r15.xyz, r15.xyzx, r16.xzwx
    r15.xyz = ((r15.xyzx)*(r16.xzwx)).xyz;
    // 183: mul r8.w, r16.y, r16.y
    r8.w = ((r16.yyyy)*(r16.yyyy)).w;
    // 184: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 185: mul r9.w, r16.y, r8.w
    r9.w = ((r16.yyyy)*(r8.wwww)).w;
    // 186: mad r8.w, -r8.w, r16.y, l(1.000000)
    r8.w = ((-(r8.wwww))*(r16.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 187: mul r16.xyz, r17.xyzx, r8.wwww
    r16.xyz = ((r17.xyzx)*(r8.wwww)).xyz;
    // 188: dp3 r8.w, r17.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r17.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: mad r17.xyz, r8.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r17.xyz = ((r8.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 190: mad r16.xyz, r1.wwww, r9.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r9.wwww)+(r16.xyzx)).xyz;
    // 191: add r16.xyz, -r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r16.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: mul r16.xyz, r16.xyzx, r16.xyzx
    r16.xyz = ((r16.xyzx)*(r16.xyzx)).xyz;
    // 193: mad r19.xyz, -r1.xxxx, r16.xyzx, r18.xyzx
    r19.xyz = ((-(r1.xxxx))*(r16.xyzx)+(r18.xyzx)).xyz;
    // 194: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 195: mul r16.xyz, r1.xxxx, r16.xyzx
    r16.xyz = ((r1.xxxx)*(r16.xyzx)).xyz;
    // 196: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 197: mad r16.xyz, -r16.xyzx, r0.wwww, r16.xyzx
    r16.xyz = ((-(r16.xyzx))*(r0.wwww)+(r16.xyzx)).xyz;
    // 198: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 199: mad r7.xyz, -r7.xyzx, r0.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.wwww)+(r7.xyzx)).xyz;
    // 200: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 201: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 202: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 203: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 204: dp3 r1.w, r9.xyzx, r19.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).w;
    // 205: add r2.w, -r1.w, r2.w
    r2.w = ((-(r1.wwww))+(r2.wwww)).w;
    // 206: mad r1.z, r1.z, r2.w, r1.w
    r1.z = ((r1.zzzz)*(r2.wwww)+(r1.wwww)).z;
    // 207: mad r6.xyz, r8.xyzx, r1.zzzz, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r1.zzzz)+(r6.xyzx)).xyz;
    // 208: mad r1.z, r1.y, r17.x, r17.y
    r1.z = ((r1.yyyy)*(r17.xxxx)+(r17.yyyy)).z;
    // 209: mad r1.z, r1.z, r1.y, r17.z
    r1.z = ((r1.zzzz)*(r1.yyyy)+(r17.zzzz)).z;
    // 210: mul r1.z, r1.y, r1.z
    r1.z = ((r1.yyyy)*(r1.zzzz)).z;
    // 211: max r1.y, r1.z, r1.y
    r1.y = (max(r1.zzzz,r1.yyyy)).y;
    // 212: mul r6.xyz, r1.yyyy, r6.xyzx
    r6.xyz = ((r1.yyyy)*(r6.xyzx)).xyz;
    // 213: dp3 r11.x, r11.xyzx, r5.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 214: dp3 r11.y, r12.xyzx, r5.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 215: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 216: dp2 r5.x, r11.xyxx, r3.zwzz
    r5.x = (dot((r11.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 217: dp2 r5.z, r11.xyxx, cb0[14].xyxx
    r5.z = (dot((r11.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 218: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t6.xyzw, s6, r4.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r4.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 219: mul r5.xyz, r10.xyzx, r10.wwww
    r5.xyz = ((r10.xyzx)*(r10.wwww)).xyz;
    // 220: mul r5.xyz, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((r5.xyzx)*(source[13].xyzx)).xyz;
    // 221: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 222: dp3 r1.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 223: div r1.z, r1.z, r6.w
    r1.z = ((r1.zzzz)/(r6.wwww)).z;
    // 224: mad r1.z, r7.w, l(5.000000), r1.z
    r1.z = ((r7.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.zzzz)).z;
    // 225: add_sat r1.z, r0.w, r1.z
    r1.z = (saturate((r0.wwww)+(r1.zzzz))).z;
    // 226: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: mad r1.w, r1.z, l(-2.000000), l(3.000000)
    r1.w = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 228: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 229: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 230: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 231: mul r1.z, r1.z, l(1.500000)
    r1.z = ((r1.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 232: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 233: mul r5.xyz, r1.zzzz, r5.xyzx
    r5.xyz = ((r1.zzzz)*(r5.xyzx)).xyz;
    // 234: mul r6.xyz, r6.xyzx, r5.xyzx
    r6.xyz = ((r6.xyzx)*(r5.xyzx)).xyz;
    // 235: mul r5.xyz, r5.xyzx, r15.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)).xyz;
    // 236: mad r6.xyz, r6.xyzx, r15.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r15.xyzx)+(r7.xyzx)).xyz;
    // 237: mul r3.yzw, r3.yyyy, cb0[23].xxyz
    r3.yzw = ((r3.yyyy)*(source[23].xxyz)).yzw;
    // 238: mad r3.xyz, r3.xxxx, cb0[22].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[22].xyzx)+(r3.yzwy)).xyz;
    // 239: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 240: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 241: add_sat r7.xyz, -r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = (saturate((-(r7.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 242: add_sat r7.xyz, r7.xyzx, cb0[12].zzzz
    r7.xyz = (saturate((r7.xyzx)+(source[12].zzzz))).xyz;
    // 243: mul r1.z, r7.x, cb0[12].w
    r1.z = ((r7.xxxx)*(source[12].wwww)).z;
    // 244: mul_sat r7.xyz, r7.xyzx, cb0[7].xyzx
    r7.xyz = (saturate((r7.xyzx)*(source[7].xyzx))).xyz;
    // 245: mul r1.z, r1.z, r5.w
    r1.z = ((r1.zzzz)*(r5.wwww)).z;
    // 246: mul r7.xyz, r7.xyzx, r1.zzzz
    r7.xyz = ((r7.xyzx)*(r1.zzzz)).xyz;
    // 247: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 248: mul r10.xyz, r0.wwww, r18.xyzx
    r10.xyz = ((r0.wwww)*(r18.xyzx)).xyz;
    // 249: mul r10.xyz, r13.xyzx, r10.xyzx
    r10.xyz = ((r13.xyzx)*(r10.xyzx)).xyz;
    // 250: mul r2.xyz, r2.xyzx, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)).xyz;
    // 251: mad r1.yzw, r5.xxyz, r1.yyyy, r2.xxyz
    r1.yzw = ((r5.xxyz)*(r1.yyyy)+(r2.xxyz)).yzw;
    // 252: mul r1.yzw, r1.yyzw, l(0.000000, 0.300000, 0.300000, 0.300000)
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.300000,0.300000,0.300000))).yzw;
    // 253: mul r2.xyz, r3.xyzx, r7.xyzx
    r2.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 254: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 255: dp3 r0.w, r9.xyzx, r0.wwww
    r0.w = (dot((r9.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 256: mul r3.xyz, r0.wwww, r8.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 257: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 258: mad r0.xyz, r0.xyzx, r3.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 259: dp3 r0.w, r4.xyzx, r14.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 260: add r2.x, -|r14.z|, l(1.000000)
    r2.x = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 261: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 262: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 263: lt r2.x, |r0.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 264: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 265: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 266: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 267: mul r2.yzw, r0.wwww, cb0[4].xxyz
    r2.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 268: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 269: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 270: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 271: mad r2.xyz, r1.yzwy, r0.wwww, r2.xyzx
    r2.xyz = ((r1.yzwy)*(r0.wwww)+(r2.xyzx)).xyz;
    // 272: mul r1.yzw, r1.xxxx, r1.yyzw
    r1.yzw = ((r1.xxxx)*(r1.yyzw)).yzw;
    // 273: mul r3.xyz, r1.xxxx, r0.xyzx
    r3.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 274: mad r0.xyz, r0.xyzx, r0.wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 275: add r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)+(r6.xyzx)).xyz;
    // 276: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 277: mad r0.xyz, r1.yzwy, l(3.141593, 3.141593, 3.141593, 0.000000), r16.xyzx
    r0.xyz = ((r1.yzwy)*(float4(3.141593,3.141593,3.141593,0.000000))+(r16.xyzx)).xyz;
    // 278: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 279: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 280: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 281: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 282: ret
    return output;
}

// source.character.static-map-native-1143.v1 / source program 3d2407872008f44fa338ce800db6b48e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1143(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1143(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[3]=g_SourceCharacterBaseConstants[0];
    source[4]=g_SourceCharacterBaseConstants[1];
    source[5]=g_SourceCharacterBaseConstants[2];
    source[6]=g_SourceCharacterBaseConstants[3];
    source[7]=g_SourceCharacterBaseConstants[4];
    source[8]=g_SourceCharacterBaseConstants[5];
    source[9]=g_SourceCharacterBaseConstants[6];
    source[10]=g_SourceCharacterBaseConstants[7];
    source[11]=g_SourceCharacterBaseConstants[8];
    source[12]=g_SourceCharacterBaseConstants[9];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(0.900000)
    r1.x = ((r0.wwww)+(float4(0.900000,0.900000,0.900000,0.900000))).x;
    // 3: round_ni_sat r1.x, r1.x
    r1.x = (saturate(floor(r1.xxxx))).x;
    // 4: add r1.x, r1.x, l(-0.333300)
    r1.x = ((r1.xxxx)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 5: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 6: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 7: mul r1.xyz, cb0[6].xyzx, cb0[8].zzzz
    r1.xyz = ((source[6].xyzx)*(source[8].zzzz)).xyz;
    // 8: mul r2.xyz, cb0[5].xyzx, cb0[8].yyyy
    r2.xyz = ((source[5].xyzx)*(source[8].yyyy)).xyz;
    // 9: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, -r2.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 11: add r1.xy, r0.wwww, cb0[10].wyww
    r1.xy = ((r0.wwww)+(source[10].wyww)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 13: mul r0.w, r3.z, cb0[8].w
    r0.w = ((r3.zzzz)*(source[8].wwww)).w;
    // 14: mul r1.zw, r3.yyyx, cb0[11].yyyw
    r1.zw = ((r3.yyyx)*(source[11].yyyw)).zw;
    // 15: log r2.w, |r0.w|
    r2.w = (log2(abs(r0.wwww))).w;
    // 16: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r2.w, r2.w, cb0[9].x
    r2.w = ((r2.wwww)*(source[9].xxxx)).w;
    // 18: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 19: movc r0.w, r0.w, l(0), r2.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 20: min r2.w, r0.w, l(1.000000)
    r2.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 22: mul r2.xyz, r0.xyzx, cb0[9].yyyy
    r2.xyz = ((r0.xyzx)*(source[9].yyyy)).xyz;
    // 23: mad r0.xyz, cb0[9].zzzz, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[9].zzzz)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 24: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 25: add r2.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 26: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 27: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 28: mad r2.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r2.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 29: mad r3.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 30: mad r4.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 31: log r5.xy, |r1.zwzz|
    r5.xy = (log2(abs(r1.zwzz))).xy;
    // 32: lt r1.zw, |r1.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r1.zw = (asfloat((uint4)((abs(r1.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 33: mul r2.w, r5.y, cb0[12].x
    r2.w = ((r5.yyyy)*(source[12].xxxx)).w;
    // 34: mul r3.w, r5.x, cb0[11].z
    r3.w = ((r5.xxxx)*(source[11].zzzz)).w;
    // 35: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 36: movc r1.z, r1.z, l(0), r3.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).z;
    // 37: max r1.z, r1.z, cb0[0].x
    r1.z = (max(r1.zzzz,source[0].xxxx)).z;
    // 38: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 39: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 40: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 42: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 43: mad r2.xyz, r3.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 44: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 45: max r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = (max(r1.wwww,r2.xyzx)).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 47: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 48: dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 49: mul r3.xy, r3.xyxx, cb0[8].xxxx
    r3.xy = ((r3.xyxx)*(source[8].xxxx)).xy;
    // 50: mul r3.xy, r3.xyxx, v2.wwww
    r3.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 51: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 53: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 54: add r3.z, r2.w, l(0.000010)
    r3.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 55: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 56: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 57: div r3.xyz, r3.xyzx, r2.wwww
    r3.xyz = ((r3.xyzx)/(r2.wwww)).xyz;
    // 58: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 59: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 60: mul r4.xyz, r2.wwww, r3.xyzx
    r4.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 61: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 62: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 63: mul r5.xyz, r2.wwww, v7.xyzx
    r5.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 64: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 65: mad r6.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 66: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 67: mul r6.yzw, r6.yyyy, cb0[23].xxyz
    r6.yzw = ((r6.yyyy)*(source[23].xxyz)).yzw;
    // 68: mad r6.xyz, r6.xxxx, cb0[22].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[22].xyzx)+(r6.yzwy)).xyz;
    // 69: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 70: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 71: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 72: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 73: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 74: mul r7.xyz, r2.wwww, v1.xyzx
    r7.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 75: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 76: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 77: mul r8.xyz, r2.wwww, v0.xyzx
    r8.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 78: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 79: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 80: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 81: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 82: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 83: dp2 r11.z, r10.xyxx, cb0[14].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 84: mul r10.zw, cb0[14].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r10.zw = ((source[14].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 85: dp2 r11.x, r10.xyxx, r10.zwzz
    r11.x = (dot((r10.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 86: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 87: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 88: dp4 r12.x, cb0[15].xyzw, r11.xyzw
    r12.x = (dot((source[15].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 89: dp4 r12.y, cb0[16].xyzw, r11.xyzw
    r12.y = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 90: dp4 r12.z, cb0[17].xyzw, r11.xyzw
    r12.z = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 91: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 92: mul r2.w, r11.y, r11.y
    r2.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 93: mad r2.w, r11.x, r11.x, -r2.w
    r2.w = ((r11.xxxx)*(r11.xxxx)+(-(r2.wwww))).w;
    // 94: dp4 r11.x, cb0[18].xyzw, r13.xyzw
    r11.x = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 95: dp4 r11.y, cb0[19].xyzw, r13.xyzw
    r11.y = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 96: dp4 r11.z, cb0[20].xyzw, r13.xyzw
    r11.z = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 97: add r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)+(r12.xyzx)).xyz;
    // 98: mad r11.xyz, cb0[21].xyzx, r2.wwww, r11.xyzx
    r11.xyz = ((source[21].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 99: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 100: mul r11.xyz, r11.xyzx, cb0[13].xyzx
    r11.xyz = ((r11.xyzx)*(source[13].xyzx)).xyz;
    // 101: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 102: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 104: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 105: mul r12.xyz, r3.wwww, v6.xyzx
    r12.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 106: dp3 r3.w, r4.xyzx, r12.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 107: deriv_rtx_coarse r10.x, r3.w
    r10.x = (ddx_coarse(r3.wwww)).x;
    // 108: deriv_rty_coarse r10.y, r3.w
    r10.y = (ddy_coarse(r3.wwww)).y;
    // 109: dp2 r4.w, r10.xyxx, r10.xyxx
    r4.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 110: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 111: mad_sat r10.y, r4.w, l(0.300000), r1.z
    r10.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r1.zzzz))).y;
    // 112: mad r1.z, r10.y, l(0.200000), l(0.200000)
    r1.z = ((r10.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).z;
    // 113: div r2.w, r2.w, r1.z
    r2.w = ((r2.wwww)/(r1.zzzz)).w;
    // 114: dp3 r4.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 115: mad r2.w, r4.w, l(5.000000), r2.w
    r2.w = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 116: sample_b_indexable(texture2d)(float,float,float,float) r5.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r5.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 117: mad r1.x, r5.w, -r1.x, r1.x
    r1.x = ((r5.wwww)*(-(r1.xxxx))+(r1.xxxx)).x;
    // 118: add_sat r0.w, r0.w, r1.x
    r0.w = (saturate((r0.wwww)+(r1.xxxx))).w;
    // 119: mul_sat r0.w, r0.w, cb2[3].w
    r0.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 120: add_sat r1.x, r0.w, r2.w
    r1.x = (saturate((r0.wwww)+(r2.wwww))).x;
    // 121: mad r2.w, r1.x, l(-2.000000), l(3.000000)
    r2.w = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 122: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 123: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 124: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 125: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 126: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 127: mul r11.xyz, r1.xxxx, r11.xyzx
    r11.xyz = ((r1.xxxx)*(r11.xyzx)).xyz;
    // 128: mul r6.xyz, r6.xyzx, r11.xyzx
    r6.xyz = ((r6.xyzx)*(r11.xyzx)).xyz;
    // 129: mul r13.xyz, r3.wwww, r4.xyzx
    r13.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 130: dp3 r1.x, -r5.xyzx, r4.xyzx
    r1.x = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).x;
    // 131: mad r4.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 132: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 133: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 134: add r1.x, r13.z, l(1.000000)
    r1.x = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: add r2.w, r3.w, l(1.000000)
    r2.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mov_sat r3.w, r3.w
    r3.w = (saturate(r3.wwww)).w;
    // 138: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 139: mul r3.w, r3.w, cb0[1].y
    r3.w = ((r3.wwww)*(source[1].yyyy)).w;
    // 140: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 141: mad_sat r3.w, r3.w, cb0[1].w, cb0[1].z
    r3.w = (saturate((r3.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 142: add_sat r10.x, -r1.x, r2.w
    r10.x = (saturate((-(r1.xxxx))+(r2.wwww))).x;
    // 143: sample_indexable(texture2d)(float,float,float,float) r14.xy, r10.xyxx, t5.xyzw, s6
    r14.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 144: add r14.zw, -r10.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r14.zw = ((-(r10.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 145: add r1.x, r1.w, r10.x
    r1.x = ((r1.wwww)+(r10.xxxx)).x;
    // 146: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 147: mov_sat r2.w, cb0[9].w
    r2.w = (saturate(source[9].wwww)).w;
    // 148: mad r15.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 149: mul r2.w, r2.w, l(0.080000)
    r2.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 150: mad r15.xyz, r0.wwww, r15.xyzx, r2.wwww
    r15.xyz = ((r0.wwww)*(r15.xyzx)+(r2.wwww)).xyz;
    // 151: max r16.xyz, r14.zzzz, r15.xyzx
    r16.xyz = (max(r14.zzzz,r15.xyzx)).xyz;
    // 152: add r16.xyz, -r15.xyzx, r16.xyzx
    r16.xyz = ((-(r15.xyzx))+(r16.xyzx)).xyz;
    // 153: mul_sat r2.w, r15.y, l(50.000000)
    r2.w = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 155: mul r17.xyz, r14.yyyy, r15.xyzx
    r17.xyz = ((r14.yyyy)*(r15.xyzx)).xyz;
    // 156: mad r16.xyz, r16.xyzx, r14.xxxx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r14.xxxx)+(r17.xyzx)).xyz;
    // 157: div r4.z, l(1.000000, 1.000000, 1.000000, 1.000000), r14.y
    r4.z = r14.y != 0.f ? 1.f / r14.y : 0.f;
    // 158: add r4.z, r4.z, l(-1.000000)
    r4.z = ((r4.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 159: mad r14.xyz, r15.xyzx, r4.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((r15.xyzx)*(r4.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 160: mad r17.xyz, -r16.xyzx, r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r14.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 161: mul r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r16.xyzx)).xyz;
    // 162: add r4.z, -r1.y, l(1.000000)
    r4.z = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 163: mad_sat r1.y, r5.w, r4.z, r1.y
    r1.y = (saturate((r5.wwww)*(r4.zzzz)+(r1.yyyy))).y;
    // 164: mad r1.y, -r1.y, cb0[2].x, l(1.000000)
    r1.y = ((-(r1.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 165: mul r4.z, r10.y, r10.y
    r4.z = ((r10.yyyy)*(r10.yyyy)).z;
    // 166: mul r5.w, r10.y, l(5.000000)
    r5.w = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 167: mad r6.w, r4.z, l(0.350000), l(1.000000)
    r6.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: mul r1.x, r1.x, r4.z
    r1.x = ((r1.xxxx)*(r4.zzzz)).x;
    // 169: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 170: add r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)+(r1.xxxx)).x;
    // 171: add_sat r1.x, r1.x, l(-1.000000)
    r1.x = (saturate((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 172: div_sat r1.y, r1.y, r6.w
    r1.y = (saturate((r1.yyyy)/(r6.wwww))).y;
    // 173: mul r1.w, r14.w, r14.w
    r1.w = ((r14.wwww)*(r14.wwww)).w;
    // 174: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 175: mul r4.z, r14.w, r1.w
    r4.z = ((r14.wwww)*(r1.wwww)).z;
    // 176: mad r1.w, -r1.w, r14.w, l(1.000000)
    r1.w = ((-(r1.wwww))*(r14.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r16.xyz, r15.xyzx, r1.wwww
    r16.xyz = ((r15.xyzx)*(r1.wwww)).xyz;
    // 178: dp3 r1.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 179: mad r15.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 180: mad r16.xyz, r2.wwww, r4.zzzz, r16.xyzx
    r16.xyz = ((r2.wwww)*(r4.zzzz)+(r16.xyzx)).xyz;
    // 181: add r16.xyz, -r16.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r16.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: mul r16.xyz, r16.xyzx, r16.xyzx
    r16.xyz = ((r16.xyzx)*(r16.xyzx)).xyz;
    // 183: mad r18.xyz, -r1.yyyy, r16.xyzx, r17.xyzx
    r18.xyz = ((-(r1.yyyy))*(r16.xyzx)+(r17.xyzx)).xyz;
    // 184: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 185: mul r16.xyz, r1.yyyy, r16.xyzx
    r16.xyz = ((r1.yyyy)*(r16.xyzx)).xyz;
    // 186: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 187: mad r16.xyz, -r16.xyzx, r0.wwww, r16.xyzx
    r16.xyz = ((-(r16.xyzx))*(r0.wwww)+(r16.xyzx)).xyz;
    // 188: mul r6.xyz, r6.xyzx, r18.xyzx
    r6.xyz = ((r6.xyzx)*(r18.xyzx)).xyz;
    // 189: mad r6.xyz, -r6.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r0.wwww)+(r6.xyzx)).xyz;
    // 190: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 191: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 192: dp2 r9.x, r8.xyxx, r10.zwzz
    r9.x = (dot((r8.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 193: dp2 r9.z, r8.xyxx, cb0[14].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 194: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 195: dp3 r1.w, r5.xyzx, r13.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 196: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 197: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 198: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t6.xyzw, s5, r5.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r5.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 199: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 200: mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // 201: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 202: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 203: div r1.z, r1.w, r1.z
    r1.z = ((r1.wwww)/(r1.zzzz)).z;
    // 204: mad r1.z, r4.w, l(5.000000), r1.z
    r1.z = ((r4.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r1.zzzz)).z;
    // 205: add_sat r1.z, r0.w, r1.z
    r1.z = (saturate((r0.wwww)+(r1.zzzz))).z;
    // 206: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 207: mad r1.w, r1.z, l(-2.000000), l(3.000000)
    r1.w = ((r1.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 208: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 209: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 210: log r1.z, r1.z
    r1.z = (log2(r1.zzzz)).z;
    // 211: mul r1.z, r1.z, l(1.500000)
    r1.z = ((r1.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 212: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 213: mul r7.xyz, r1.zzzz, r7.xyzx
    r7.xyz = ((r1.zzzz)*(r7.xyzx)).xyz;
    // 214: mad r1.z, r1.x, r15.x, r15.y
    r1.z = ((r1.xxxx)*(r15.xxxx)+(r15.yyyy)).z;
    // 215: mad r1.z, r1.z, r1.x, r15.z
    r1.z = ((r1.zzzz)*(r1.xxxx)+(r15.zzzz)).z;
    // 216: mul r1.z, r1.x, r1.z
    r1.z = ((r1.xxxx)*(r1.zzzz)).z;
    // 217: max r1.x, r1.z, r1.x
    r1.x = (max(r1.zzzz,r1.xxxx)).x;
    // 218: mul r5.yzw, r5.yyyy, cb0[23].xxyz
    r5.yzw = ((r5.yyyy)*(source[23].xxyz)).yzw;
    // 219: mad r5.xyz, cb0[22].xyzx, r5.xxxx, r5.yzwy
    r5.xyz = ((source[22].xyzx)*(r5.xxxx)+(r5.yzwy)).xyz;
    // 220: mul r5.xyz, r5.xyzx, cb0[24].wwww
    r5.xyz = ((r5.xyzx)*(source[24].wwww)).xyz;
    // 221: mul r5.xyz, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.xxxx)*(r5.xyzx)).xyz;
    // 222: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 223: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 224: mad r5.xyz, r5.xyzx, r14.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r14.xyzx)+(r6.xyzx)).xyz;
    // 225: dp3 r1.z, r3.xyzx, r12.xyzx
    r1.z = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).z;
    // 226: add r1.w, -|r12.z|, l(1.000000)
    r1.w = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 227: add r1.z, -|r1.z|, l(1.000000)
    r1.z = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 228: mul r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)*(r1.wwww)).z;
    // 229: lt r1.w, |r1.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 230: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 231: mul r1.z, r1.z, l(1.500000)
    r1.z = ((r1.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 232: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 233: mul r3.xyz, r1.zzzz, cb0[4].xyzx
    r3.xyz = ((r1.zzzz)*(source[4].xyzx)).xyz;
    // 234: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 235: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 236: mul r6.xyz, r0.wwww, r17.xyzx
    r6.xyz = ((r0.wwww)*(r17.xyzx)).xyz;
    // 237: mul r6.xyz, r11.xyzx, r6.xyzx
    r6.xyz = ((r11.xyzx)*(r6.xyzx)).xyz;
    // 238: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 239: mad r1.xzw, r7.xxyz, r1.xxxx, r2.xxyz
    r1.xzw = ((r7.xxyz)*(r1.xxxx)+(r2.xxyz)).xzw;
    // 240: mul r1.xzw, r1.xxzw, l(0.300000, 0.000000, 0.300000, 0.300000)
    r1.xzw = ((r1.xxzw)*(float4(0.300000,0.000000,0.300000,0.300000))).xzw;
    // 241: add r2.x, -r1.y, l(1.000000)
    r2.x = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 242: mad r2.yzw, r1.xxzw, r2.xxxx, r3.xxyz
    r2.yzw = ((r1.xxzw)*(r2.xxxx)+(r3.xxyz)).yzw;
    // 243: mul r1.xzw, r1.yyyy, r1.xxzw
    r1.xzw = ((r1.yyyy)*(r1.xxzw)).xzw;
    // 244: mad r1.xzw, r1.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), r16.xxyz
    r1.xzw = ((r1.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(r16.xxyz)).xzw;
    // 245: mul r3.xyz, r4.yyyy, cb0[23].xyzx
    r3.xyz = ((r4.yyyy)*(source[23].xyzx)).xyz;
    // 246: mad r3.xyz, r4.xxxx, cb0[22].xyzx, r3.xyzx
    r3.xyz = ((r4.xxxx)*(source[22].xyzx)+(r3.xyzx)).xyz;
    // 247: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 248: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 249: add_sat r4.xyz, -r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (saturate((-(r4.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 250: add_sat r4.xyz, r4.xyzx, cb0[12].zzzz
    r4.xyz = (saturate((r4.xyzx)+(source[12].zzzz))).xyz;
    // 251: mul r4.w, r4.x, cb0[12].w
    r4.w = ((r4.xxxx)*(source[12].wwww)).w;
    // 252: mul_sat r4.xyz, r4.xyzx, cb0[7].xyzx
    r4.xyz = (saturate((r4.xyzx)*(source[7].xyzx))).xyz;
    // 253: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 254: mul r4.xyz, r4.xyzx, r3.wwww
    r4.xyz = ((r4.xyzx)*(r3.wwww)).xyz;
    // 255: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 256: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 257: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 258: mad r2.xyz, r0.xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((r0.xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 259: mul r0.xyz, r1.yyyy, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)).xyz;
    // 260: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xzwx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xzwx)).xyz;
    // 261: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 262: add r0.xyz, r2.xyzx, r5.xyzx
    r0.xyz = ((r2.xyzx)+(r5.xyzx)).xyz;
    // 263: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 264: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 265: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 266: ret
    return output;
}

// source.character.static-map-native-1144.v1 / source program 5bcb3bcdd098ee478e83a1043bda15c7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1144(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[11];
    source[9].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[9].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[9].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[27]=1.f;
    source[28]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: mul r0.w, r2.z, cb0[10].y
    r0.w = ((r2.zzzz)*(source[10].yyyy)).w;
    // 12: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 13: mul r2.xy, r2.xyxx, cb0[7].xxxx
    r2.xy = ((r2.xyxx)*(source[7].xxxx)).xy;
    // 14: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 15: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 16: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 17: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 18: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 19: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 22: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r3.xyz, r1.wwww, r2.xyzx
    r3.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 25: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 26: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 27: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 28: mul r1.yzw, r1.yyyy, cb0[25].xxyz
    r1.yzw = ((r1.yyyy)*(source[25].xxyz)).yzw;
    // 29: mad r1.xyz, r1.xxxx, cb0[24].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[24].xyzx)+(r1.yzwy)).xyz;
    // 30: mul r1.xyz, r1.xyzx, cb0[26].wwww
    r1.xyz = ((r1.xyzx)*(source[26].wwww)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: mul r1.w, r4.z, cb0[11].y
    r1.w = ((r4.zzzz)*(source[11].yyyy)).w;
    // 33: mul r4.xy, r4.yxyy, cb0[12].ywyy
    r4.xy = ((r4.yxyy)*(source[12].ywyy)).xy;
    // 34: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 35: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 36: mul r2.w, r2.w, cb0[11].z
    r2.w = ((r2.wwww)*(source[11].zzzz)).w;
    // 37: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 38: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 39: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 41: mul r5.xyz, cb0[6].xyzx, cb0[10].zzzz
    r5.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 42: mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 43: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 44: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 45: mul r5.xyz, r1.wwww, v5.xyzx
    r5.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 46: dp3 r1.w, r3.xyzx, r5.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 47: mul r6.xyz, r1.wwww, r3.xyzx
    r6.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 48: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r5.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r5.xyzx))).xyz;
    // 49: add r7.xyz, r6.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r7.xyz = ((r6.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 50: mad r7.xy, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 51: min r3.w, r7.z, l(1.000000)
    r3.w = (min(r7.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: mad r7.xy, r7.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t2.xyzw, s1, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 54: mul r7.xyz, r7.xyzx, cb0[5].xyzx
    r7.xyz = ((r7.xyzx)*(source[5].xyzx)).xyz;
    // 55: mad r7.xyz, cb0[10].xxxx, r7.xyzx, r7.xyzx
    r7.xyz = ((source[10].xxxx)*(r7.xyzx)+(r7.xyzx)).xyz;
    // 56: add r7.xyz, r7.xyzx, -cb0[10].xxxx
    r7.xyz = ((r7.xyzx)+(-(source[10].xxxx))).xyz;
    // 57: mov_sat r8.xyz, r7.xyzx
    r8.xyz = (saturate(r7.xyzx)).xyz;
    // 58: mov_sat r7.xyz, -r7.xyzx
    r7.xyz = (saturate(-(r7.xyzx))).xyz;
    // 59: mad r7.xyz, -r0.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r0.wwww))*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 60: mad r0.xyz, r0.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 61: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 62: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 63: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 64: mul r7.xyz, r0.xyzx, cb0[10].wwww
    r7.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 65: mad r0.xyz, cb0[11].xxxx, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[11].xxxx)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 66: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 67: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mul r0.xyz, r0.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // 69: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 70: mul r7.xyz, r0.xyzx, r1.xyzx
    r7.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 71: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 72: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 73: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 74: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 76: mul r9.xyz, r9.xyzx, cb0[28].xyzx
    r9.xyz = ((r9.xyzx)*(source[28].xyzx)).xyz;
    // 77: dp3 r2.w, r9.xyzx, r8.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 78: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 79: mul r8.xyz, r8.xyzx, cb0[27].xyzx
    r8.xyz = ((r8.xyzx)*(source[27].xyzx)).xyz;
    // 80: mul r10.xyz, r2.wwww, r8.xyzx
    r10.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 81: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 82: mad r10.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 83: mad r11.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 84: mad r12.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 85: log r13.xy, |r4.xyxx|
    r13.xy = (log2(abs(r4.xyxx))).xy;
    // 86: lt r4.xy, |r4.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r4.xy = (asfloat((uint4)((abs(r4.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 87: mul r5.w, r13.y, cb0[13].x
    r5.w = ((r13.yyyy)*(source[13].xxxx)).w;
    // 88: mul r6.w, r13.x, cb0[12].z
    r6.w = ((r13.xxxx)*(source[12].zzzz)).w;
    // 89: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 90: movc r4.x, r4.x, l(0), r6.w
    r4.x = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.wwww)).x;
    // 91: max r4.x, r4.x, cb0[0].x
    r4.x = (max(r4.xxxx,source[0].xxxx)).x;
    // 92: min r4.z, r4.x, l(1.000000)
    r4.z = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 93: exp r4.x, r5.w
    r4.x = (exp2(r5.wwww)).x;
    // 94: min r4.x, r4.x, l(1.000000)
    r4.x = (min(r4.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 95: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 96: mad r11.xyz, r4.xxxx, r11.xyzx, r12.xyzx
    r11.xyz = ((r4.xxxx)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 97: mad r10.xyz, r11.xyzx, r4.xxxx, r10.xyzx
    r10.xyz = ((r11.xyzx)*(r4.xxxx)+(r10.xyzx)).xyz;
    // 98: mul r10.xyz, r4.xxxx, r10.xyzx
    r10.xyz = ((r4.xxxx)*(r10.xyzx)).xyz;
    // 99: max r10.xyz, r4.xxxx, r10.xyzx
    r10.xyz = (max(r4.xxxx,r10.xyzx)).xyz;
    // 100: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 101: dp3 r4.y, v1.xyzx, v1.xyzx
    r4.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 102: rsq r4.y, r4.y
    r4.y = (rsqrt(r4.yyyy)).y;
    // 103: mul r10.xyz, r4.yyyy, v1.xyzx
    r10.xyz = ((r4.yyyy)*(v1.xyzx)).xyz;
    // 104: dp3 r4.y, v0.xyzx, v0.xyzx
    r4.y = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).y;
    // 105: rsq r4.y, r4.y
    r4.y = (rsqrt(r4.yyyy)).y;
    // 106: mul r11.xyz, r4.yyyy, v0.xyzx
    r11.xyz = ((r4.yyyy)*(v0.xyzx)).xyz;
    // 107: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 108: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 109: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 110: dp3 r13.y, r12.xyzx, r3.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 111: dp3 r12.y, r12.xyzx, r6.xyzx
    r12.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 112: dp3 r13.x, r11.xyzx, r3.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 113: dp3 r14.y, r10.xyzx, r3.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 114: dp3 r3.y, r10.xyzx, r6.xyzx
    r3.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 115: dp3 r12.x, r11.xyzx, r6.xyzx
    r12.x = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 116: dp2 r14.z, r13.xyxx, cb0[15].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 117: mul r10.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 118: dp2 r14.x, r13.xyxx, r10.xyxx
    r14.x = (dot((r13.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 119: dp2 r3.x, r12.xyxx, r10.xyxx
    r3.x = (dot((r12.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 120: dp2 r3.z, r12.xyxx, cb0[15].xyxx
    r3.z = (dot((r12.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 121: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 122: dp4 r10.x, cb0[16].xyzw, r14.xyzw
    r10.x = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 123: dp4 r10.y, cb0[17].xyzw, r14.xyzw
    r10.y = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 124: dp4 r10.z, cb0[18].xyzw, r14.xyzw
    r10.z = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 125: mul r11.xyzw, r14.yzzx, r14.xyzz
    r11.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 126: dp4 r12.x, cb0[19].xyzw, r11.xyzw
    r12.x = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 127: dp4 r12.y, cb0[20].xyzw, r11.xyzw
    r12.y = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 128: dp4 r12.z, cb0[21].xyzw, r11.xyzw
    r12.z = (dot((source[21].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 129: add r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)+(r12.xyzx)).xyz;
    // 130: mul r4.y, r14.y, r14.y
    r4.y = ((r14.yyyy)*(r14.yyyy)).y;
    // 131: mov r13.z, r14.y
    r13.z = (r14.yyyy).z;
    // 132: mad r4.y, r14.x, r14.x, -r4.y
    r4.y = ((r14.xxxx)*(r14.xxxx)+(-(r4.yyyy))).y;
    // 133: mad r10.xyz, cb0[22].xyzx, r4.yyyy, r10.xyzx
    r10.xyz = ((source[22].xyzx)*(r4.yyyy)+(r10.xyzx)).xyz;
    // 134: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 136: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 137: deriv_rtx_coarse r11.x, r1.w
    r11.x = (ddx_coarse(r1.wwww)).x;
    // 138: deriv_rty_coarse r11.y, r1.w
    r11.y = (ddy_coarse(r1.wwww)).y;
    // 139: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: add_sat r12.x, -r3.w, r1.w
    r12.x = (saturate((-(r3.wwww))+(r1.wwww))).x;
    // 141: dp2 r1.w, r11.xyxx, r11.xyxx
    r1.w = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).w;
    // 142: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 143: mad_sat r12.y, r1.w, l(0.300000), r4.z
    r12.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 144: add r1.w, -r12.y, l(1.000000)
    r1.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mov_sat r0.w, cb0[11].w
    r0.w = (saturate(source[11].wwww)).w;
    // 146: mad r11.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r11.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 147: mul r3.w, r0.w, l(0.080000)
    r3.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 148: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 149: mad r11.xyz, r4.wwww, r11.xyzx, r3.wwww
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r3.wwww)).xyz;
    // 150: max r14.xyz, r1.wwww, r11.xyzx
    r14.xyz = (max(r1.wwww,r11.xyzx)).xyz;
    // 151: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 152: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 153: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 154: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t4.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 155: add r0.w, r4.x, r12.x
    r0.w = ((r4.xxxx)+(r12.xxxx)).w;
    // 156: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 157: mul r15.xyz, r11.xyzx, r12.wwww
    r15.xyz = ((r11.xyzx)*(r12.wwww)).xyz;
    // 158: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 159: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r1.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 160: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 161: mad r12.xzw, r11.xxyz, r1.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r11.xxyz)*(r1.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 162: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: mad r11.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 164: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 166: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 167: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 168: mad r7.xyz, -r7.xyzx, r4.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r4.wwww)+(r7.xyzx)).xyz;
    // 169: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 170: dp2_sat r10.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 171: dp3_sat r10.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 172: dp3_sat r10.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 173: mul r6.xyz, r10.xyzx, r10.xyzx
    r6.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 174: dp3 r1.w, r9.xyzx, r6.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 175: add r2.w, -r1.w, r2.w
    r2.w = ((-(r1.wwww))+(r2.wwww)).w;
    // 176: mad r1.w, r4.z, r2.w, r1.w
    r1.w = ((r4.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 177: mad r1.xyz, r8.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r8.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 178: mul r4.yzw, r1.wwww, r8.xxyz
    r4.yzw = ((r1.wwww)*(r8.xxyz)).yzw;
    // 179: mul r1.w, r12.y, r12.y
    r1.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 180: mul r2.w, r12.y, l(5.000000)
    r2.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 181: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t5.xyzw, s5, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 182: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 183: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 184: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 185: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 186: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 187: add r0.w, r4.x, r0.w
    r0.w = ((r4.xxxx)+(r0.wwww)).w;
    // 188: mov o5.y, r4.x
    output.targets[5].y = (r4.xxxx).y;
    // 189: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 190: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 191: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 192: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 193: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 194: mul r6.xyz, r0.wwww, r1.xyzx
    r6.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 195: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 196: div r1.xyz, r4.yzwy, r1.xyzx
    r1.xyz = ((r4.yzwy)/(r1.xyzx)).xyz;
    // 197: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: mul r1.xyz, r3.xyzx, r6.xyzx
    r1.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 199: mad r3.xyz, r1.xyzx, r12.xzwx, r7.xyzx
    r3.xyz = ((r1.xyzx)*(r12.xzwx)+(r7.xyzx)).xyz;
    // 200: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 201: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 202: dp3 r1.x, r2.xyzx, r5.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 203: add r1.y, -|r5.z|, l(1.000000)
    r1.y = ((-(abs(r5.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 204: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 205: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 206: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 207: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 208: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 209: mul r1.yzw, r1.yyyy, cb0[2].xxyz
    r1.yzw = ((r1.yyyy)*(source[2].xxyz)).yzw;
    // 210: lt r2.x, |r1.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 211: movc r1.yzw, r2.xxxx, l(0,0,0,0), r1.yyzw
    r1.yzw = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzw)).yzw;
    // 212: mad_sat r2.x, r1.x, cb0[7].w, -cb0[8].x
    r2.x = (saturate((r1.xxxx)*(source[7].wwww)+(-(source[8].xxxx)))).x;
    // 213: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 214: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 215: mul r2.y, r2.y, cb0[8].y
    r2.y = ((r2.yyyy)*(source[8].yyyy)).y;
    // 216: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 217: mul r2.yzw, r2.yyyy, cb0[3].xxyz
    r2.yzw = ((r2.yyyy)*(source[3].xxyz)).yzw;
    // 218: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 219: add r2.xyz, r2.xyzx, -cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[3].xyzx))).xyz;
    // 220: mad r2.xyz, cb0[3].wwww, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((source[3].wwww)*(r2.xyzx)+(source[3].xyzx)).xyz;
    // 221: mul r4.xyz, cb0[4].xyzx, cb0[8].wwww
    r4.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 222: mul r4.xyz, r4.xyzx, cb0[9].zzzz
    r4.xyz = ((r4.xyzx)*(source[9].zzzz)).xyz;
    // 223: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 224: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 225: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 226: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 227: mul r4.xyz, r4.xyzx, cb0[9].wwww
    r4.xyz = ((r4.xyzx)*(source[9].wwww)).xyz;
    // 228: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 229: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 230: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 231: mad r1.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.yzwy
    r1.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.yzwy)).xyz;
    // 232: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 233: add r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)+(r1.xyzx)).xyz;
    // 234: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 235: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 236: dp3 r0.x, r13.xyzx, r13.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 237: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 238: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 239: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 240: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 241: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 242: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 243: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 244: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 245: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 246: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 247: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 248: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 249: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 250: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 251: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 252: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 253: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 254: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 255: ret
    return output;
}

// source.character.static-map-native-1144.v1 / source program 079fae4883ab604685b641412431e396
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1144(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1144(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[11];
    source[9].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[9].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[9].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: mul r0.w, r1.z, cb0[10].y
    r0.w = ((r1.zzzz)*(source[10].yyyy)).w;
    // 9: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 10: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
    // 11: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 12: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 13: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 14: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 15: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 17: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 18: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 19: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 20: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 21: mul r2.xyz, r1.wwww, r1.xyzx
    r2.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r3.xyz, r1.wwww, v1.xyzx
    r3.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 26: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 27: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 28: mul r5.xyz, r3.zxyz, r4.yzxy
    r5.xyz = ((r3.zxyz)*(r4.yzxy)).xyz;
    // 29: mad r5.xyz, r3.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r3.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 30: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 31: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 32: dp3 r6.x, r4.xyzx, r2.xyzx
    r6.x = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 33: dp2 r7.z, r6.xyxx, cb0[15].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 34: mul r8.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 35: dp2 r7.x, r6.xyxx, r8.xyxx
    r7.x = (dot((r6.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 36: dp3 r7.y, r3.xyzx, r2.xyzx
    r7.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 37: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 38: dp4 r9.x, cb0[16].xyzw, r7.xyzw
    r9.x = (dot((source[16].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 39: dp4 r9.y, cb0[17].xyzw, r7.xyzw
    r9.y = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 40: dp4 r9.z, cb0[18].xyzw, r7.xyzw
    r9.z = (dot((source[18].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 41: mul r10.xyzw, r7.yzzx, r7.xyzz
    r10.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 42: dp4 r11.x, cb0[19].xyzw, r10.xyzw
    r11.x = (dot((source[19].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 43: dp4 r11.y, cb0[20].xyzw, r10.xyzw
    r11.y = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 44: dp4 r11.z, cb0[21].xyzw, r10.xyzw
    r11.z = (dot((source[21].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 45: add r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)+(r11.xyzx)).xyz;
    // 46: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 47: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 48: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 49: mad r7.xyz, cb0[22].xyzx, r1.wwww, r9.xyzx
    r7.xyz = ((source[22].xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 50: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: mul r7.xyz, r7.xyzx, cb0[14].xyzx
    r7.xyz = ((r7.xyzx)*(source[14].xyzx)).xyz;
    // 52: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 53: mul r9.xyz, cb0[6].xyzx, cb0[10].zzzz
    r9.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 54: mul r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 55: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 56: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 57: mul r9.xyz, r1.wwww, v5.xyzx
    r9.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 58: dp3 r1.w, r2.xyzx, r9.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 59: mul r10.xyz, r1.wwww, r2.xyzx
    r10.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 60: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 61: add r11.xyz, r10.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r11.xyz = ((r10.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 62: mad r8.zw, r11.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000), -v4.xxxy
    r8.zw = ((r11.xxxy)*(float4(0.000000,0.000000,0.500000,0.500000))+(-(v4.xxxy))).zw;
    // 63: min r2.w, r11.z, l(1.000000)
    r2.w = (min(r11.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: mad r8.zw, r8.zzzw, l(0.000000, 0.000000, 0.750000, 0.750000), v4.xxxy
    r8.zw = ((r8.zzzw)*(float4(0.000000,0.000000,0.750000,0.750000))+(v4.xxxy)).zw;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, r8.zwzz, t2.xyzw, s1, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r8.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: mul r11.xyz, r11.xyzx, cb0[5].xyzx
    r11.xyz = ((r11.xyzx)*(source[5].xyzx)).xyz;
    // 67: mad r11.xyz, cb0[10].xxxx, r11.xyzx, r11.xyzx
    r11.xyz = ((source[10].xxxx)*(r11.xyzx)+(r11.xyzx)).xyz;
    // 68: add r11.xyz, r11.xyzx, -cb0[10].xxxx
    r11.xyz = ((r11.xyzx)+(-(source[10].xxxx))).xyz;
    // 69: mov_sat r12.xyz, r11.xyzx
    r12.xyz = (saturate(r11.xyzx)).xyz;
    // 70: mov_sat r11.xyz, -r11.xyzx
    r11.xyz = (saturate(-(r11.xyzx))).xyz;
    // 71: mad r11.xyz, -r0.wwww, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(r0.wwww))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mad r0.xyz, r0.wwww, r12.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r12.xyzx)+(r0.xyzx)).xyz;
    // 73: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 74: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 75: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 76: mul r11.xyz, r0.xyzx, cb0[10].wwww
    r11.xyz = ((r0.xyzx)*(source[10].wwww)).xyz;
    // 77: mad r0.xyz, cb0[11].xxxx, r0.xyzx, -r11.xyzx
    r0.xyz = ((source[11].xxxx)*(r0.xyzx)+(-(r11.xyzx))).xyz;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r12.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r12.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: mul r0.w, r12.z, cb0[11].y
    r0.w = ((r12.zzzz)*(source[11].yyyy)).w;
    // 80: mul r8.zw, r12.yyyx, cb0[12].yyyw
    r8.zw = ((r12.yyyx)*(source[12].yyyw)).zw;
    // 81: log r3.w, |r0.w|
    r3.w = (log2(abs(r0.wwww))).w;
    // 82: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 83: mul r3.w, r3.w, cb0[11].z
    r3.w = ((r3.wwww)*(source[11].zzzz)).w;
    // 84: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 85: movc r0.w, r0.w, l(0), r3.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 86: min r3.w, r0.w, l(1.000000)
    r3.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 87: mul_sat r12.w, r0.w, cb2[3].w
    r12.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 88: mad r0.xyz, r3.wwww, r0.xyzx, r11.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r11.xyzx)).xyz;
    // 89: add r11.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 90: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 91: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 92: mov_sat r0.w, cb0[11].w
    r0.w = (saturate(source[11].wwww)).w;
    // 93: mad r11.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r11.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 94: mul r3.w, r0.w, l(0.080000)
    r3.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 95: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 96: mad r11.xyz, r12.wwww, r11.xyzx, r3.wwww
    r11.xyz = ((r12.wwww)*(r11.xyzx)+(r3.wwww)).xyz;
    // 97: deriv_rtx_coarse r12.x, r1.w
    r12.x = (ddx_coarse(r1.wwww)).x;
    // 98: deriv_rty_coarse r12.y, r1.w
    r12.y = (ddy_coarse(r1.wwww)).y;
    // 99: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: add_sat r13.x, -r2.w, r0.w
    r13.x = (saturate((-(r2.wwww))+(r0.wwww))).x;
    // 101: dp2 r0.w, r12.xyxx, r12.xyxx
    r0.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 102: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 103: log r12.xy, |r8.zwzz|
    r12.xy = (log2(abs(r8.zwzz))).xy;
    // 104: lt r8.zw, |r8.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r8.zw = (asfloat((uint4)((abs(r8.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 105: mul r1.w, r12.x, cb0[12].z
    r1.w = ((r12.xxxx)*(source[12].zzzz)).w;
    // 106: mul r2.w, r12.y, cb0[13].x
    r2.w = ((r12.yyyy)*(source[13].xxxx)).w;
    // 107: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 108: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: movc r2.w, r8.w, l(0), r2.w
    r2.w = ((asuint(r8.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 110: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 111: movc r1.w, r8.z, l(0), r1.w
    r1.w = ((asuint(r8.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 112: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 113: min r12.z, r1.w, l(1.000000)
    r12.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: mad_sat r13.y, r0.w, l(0.300000), r12.z
    r13.y = (saturate((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r12.zzzz))).y;
    // 115: mov o2.zw, r12.zzzw
    output.targets[2].zw = (r12.zzzw).zw;
    // 116: add r0.w, -r13.y, l(1.000000)
    r0.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: max r12.xyz, r11.xyzx, r0.wwww
    r12.xyz = (max(r11.xyzx,r0.wwww)).xyz;
    // 118: add r12.xyz, -r11.xyzx, r12.xyzx
    r12.xyz = ((-(r11.xyzx))+(r12.xyzx)).xyz;
    // 119: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 120: mul r12.xyz, r0.wwww, r12.xyzx
    r12.xyz = ((r0.wwww)*(r12.xyzx)).xyz;
    // 121: sample_indexable(texture2d)(float,float,float,float) r8.zw, r13.xyxx, t4.zwxy, s5
    r8.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 122: add r0.w, r2.w, r13.x
    r0.w = ((r2.wwww)+(r13.xxxx)).w;
    // 123: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 124: mul r13.xzw, r8.wwww, r11.xxyz
    r13.xzw = ((r8.wwww)*(r11.xxyz)).xzw;
    // 125: mad r12.xyz, r12.xyzx, r8.zzzz, r13.xzwx
    r12.xyz = ((r12.xyzx)*(r8.zzzz)+(r13.xzwx)).xyz;
    // 126: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r8.w
    r1.w = r8.w != 0.f ? 1.f / r8.w : 0.f;
    // 127: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 128: mad r13.xzw, r11.xxyz, r1.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r1.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 129: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r11.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 131: mad r14.xyz, -r12.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 132: mul r12.xyz, r12.xyzx, r13.xzwx
    r12.xyz = ((r12.xyzx)*(r13.xzwx)).xyz;
    // 133: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 134: mad r13.xzw, r0.xxyz, l(2.755200, 0.000000, 2.755200, 2.755200), l(0.690300, 0.000000, 0.690300, 0.690300)
    r13.xzw = ((r0.xxyz)*(float4(2.755200,0.000000,2.755200,2.755200))+(float4(0.690300,0.000000,0.690300,0.690300))).xzw;
    // 135: mad r14.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 136: mad r15.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 137: mad r14.xyz, r2.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 138: mad r13.xzw, r14.xxyz, r2.wwww, r13.xxzw
    r13.xzw = ((r14.xxyz)*(r2.wwww)+(r13.xxzw)).xzw;
    // 139: mul r13.xzw, r2.wwww, r13.xxzw
    r13.xzw = ((r2.wwww)*(r13.xxzw)).xzw;
    // 140: max r13.xzw, r2.wwww, r13.xxzw
    r13.xzw = (max(r2.wwww,r13.xxzw)).xzw;
    // 141: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 142: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 143: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 144: dp3 r1.w, r14.xyzx, r2.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 145: dp3 r2.x, r14.xyzx, r10.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 146: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 147: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 148: mad r8.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r8.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 149: mul r8.zw, r8.zzzw, r8.zzzw
    r8.zw = ((r8.zzzw)*(r8.zzzw)).zw;
    // 150: mul r14.xyz, r8.wwww, cb0[25].xyzx
    r14.xyz = ((r8.wwww)*(source[25].xyzx)).xyz;
    // 151: mad r14.xyz, r8.zzzz, cb0[24].xyzx, r14.xyzx
    r14.xyz = ((r8.zzzz)*(source[24].xyzx)+(r14.xyzx)).xyz;
    // 152: mul r14.xyz, r14.xyzx, cb0[26].wwww
    r14.xyz = ((r14.xyzx)*(source[26].wwww)).xyz;
    // 153: mul r14.xyz, r0.xyzx, r14.xyzx
    r14.xyz = ((r0.xyzx)*(r14.xyzx)).xyz;
    // 154: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 155: mul r7.xyz, r7.xyzx, r13.xzwx
    r7.xyz = ((r7.xyzx)*(r13.xzwx)).xyz;
    // 156: mad r7.xyz, -r7.xyzx, r12.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r12.wwww)+(r7.xyzx)).xyz;
    // 157: dp3 r4.x, r4.xyzx, r10.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 158: dp3 r4.y, r5.xyzx, r10.xyzx
    r4.y = (dot((r5.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 159: dp3 r3.y, r3.xyzx, r10.xyzx
    r3.y = (dot((r3.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 160: dp2 r3.x, r4.xyxx, r8.xyxx
    r3.x = (dot((r4.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 161: dp2 r3.z, r4.xyxx, cb0[15].xyxx
    r3.z = (dot((r4.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 162: mul r1.w, r13.y, l(5.000000)
    r1.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 163: mul r2.z, r13.y, r13.y
    r2.z = ((r13.yyyy)*(r13.yyyy)).z;
    // 164: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 165: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 166: add r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)+(r0.wwww)).w;
    // 167: mov o5.y, r2.w
    output.targets[5].y = (r2.wwww).y;
    // 168: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 169: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t5.xyzw, s4, r1.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 170: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 171: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 172: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 173: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 174: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 175: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 176: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 177: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 178: mad r2.xyz, cb0[24].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[24].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 179: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 180: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 181: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 182: mad r3.xyz, r2.xyzx, r12.xyzx, r7.xyzx
    r3.xyz = ((r2.xyzx)*(r12.xyzx)+(r7.xyzx)).xyz;
    // 183: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 184: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: dp3 r0.w, r1.xyzx, r9.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 186: add r1.x, -|r9.z|, l(1.000000)
    r1.x = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 187: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 189: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 190: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 191: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 192: mul r1.xyz, r1.xxxx, cb0[2].xyzx
    r1.xyz = ((r1.xxxx)*(source[2].xyzx)).xyz;
    // 193: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 194: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 195: mad_sat r1.w, r0.w, cb0[7].w, -cb0[8].x
    r1.w = (saturate((r0.wwww)*(source[7].wwww)+(-(source[8].xxxx)))).w;
    // 196: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 197: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 198: mul r2.x, r2.x, cb0[8].y
    r2.x = ((r2.xxxx)*(source[8].yyyy)).x;
    // 199: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 200: mul r2.xyz, r2.xxxx, cb0[3].xyzx
    r2.xyz = ((r2.xxxx)*(source[3].xyzx)).xyz;
    // 201: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 202: add r2.xyz, r2.xyzx, -cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[3].xyzx))).xyz;
    // 203: mad r2.xyz, cb0[3].wwww, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((source[3].wwww)*(r2.xyzx)+(source[3].xyzx)).xyz;
    // 204: mul r4.xyz, cb0[4].xyzx, cb0[8].wwww
    r4.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 205: mul r4.xyz, r4.xyzx, cb0[9].zzzz
    r4.xyz = ((r4.xyzx)*(source[9].zzzz)).xyz;
    // 206: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 207: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 208: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 209: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 210: mul r4.xyz, r4.xyzx, cb0[9].wwww
    r4.xyz = ((r4.xyzx)*(source[9].wwww)).xyz;
    // 211: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 212: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 213: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 214: mad r1.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.xyzx)).xyz;
    // 215: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 216: add r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)+(r1.xyzx)).xyz;
    // 217: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 218: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 219: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 220: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 221: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 222: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 223: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 224: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 225: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 226: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 227: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 228: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 229: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 230: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 231: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 232: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 233: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 234: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 235: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 236: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 237: ret
    return output;
}

// source.character.static-map-native-1145.v1 / source program 79a3889502ce5d47a0993b92eca23510
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1145(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    source[25]=1.f;
    source[26]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
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
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
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
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
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

// source.character.static-map-native-1145.v1 / source program 13794a555393054d8c8e645595dd7b09
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1145(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1145(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
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
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
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
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
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

// source.character.static-map-native-1146.v1 / source program ff6ba42b8c2fcd46acbf122c14751516
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1146(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[9]=1.f;
    source[10]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
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

// source.character.static-map-native-1146.v1 / source program 6449403c587a784295a7fad9dd9ae108
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1146(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1146(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[3];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
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

// source.character.static-map-native-1147.v1 / source program d16b89781e7e3d4abd0d039d238618fc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1147(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[27]=1.f;
    source[28]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: mul r1.zw, r1.xxxy, cb0[8].xxxx
    r1.zw = ((r1.xxxy)*(source[8].xxxx)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.zw, r1.zzzw, cb0[8].yyyy
    r1.zw = ((r1.zzzw)*(source[8].yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mad r1.zw, cb0[7].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 24: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 27: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 28: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 29: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 30: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 31: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 32: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 33: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r0.x, r0.x, l(0.500000), cb0[9].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].wwww)).x;
    // 35: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[8].zzzz
    r0.zw = ((v4.xxxy)*(source[8].zzzz)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 43: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 44: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 45: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 46: max r1.w, cb0[9].x, l(0.000000)
    r1.w = (max(source[9].xxxx,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 47: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 48: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 49: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 50: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 51: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 53: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 54: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 55: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 56: mul r8.xyz, cb0[6].xyzx, cb0[10].yyyy
    r8.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 57: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 58: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 60: mad r7.xyz, cb0[10].wwww, r7.xyzx, r9.xyzx
    r7.xyz = ((source[10].wwww)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 61: mul r8.xyz, cb0[5].xyzx, cb0[10].xxxx
    r8.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 62: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 63: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 64: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r7.xyz, r4.xyzx, cb0[11].xxxx
    r7.xyz = ((r4.xyzx)*(source[11].xxxx)).xyz;
    // 67: mad r4.xyz, cb0[11].yyyy, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 68: mul r0.y, r1.z, cb0[11].z
    r0.y = ((r1.zzzz)*(source[11].zzzz)).y;
    // 69: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 70: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 71: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 72: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 73: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 74: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 75: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 76: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 77: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 79: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: mad r7.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r7.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 81: mad r9.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 82: mad r10.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 83: mul r0.y, r1.x, cb0[13].x
    r0.y = ((r1.xxxx)*(source[13].xxxx)).y;
    // 84: mul r1.x, r1.y, cb0[12].z
    r1.x = ((r1.yyyy)*(source[12].zzzz)).x;
    // 85: log r1.y, |r0.y|
    r1.y = (log2(abs(r0.yyyy))).y;
    // 86: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 87: mul r1.y, r1.y, cb0[13].y
    r1.y = ((r1.yyyy)*(source[13].yyyy)).y;
    // 88: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 89: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: movc r0.y, r0.y, l(0), r1.y
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 91: mad r1.yzw, r0.yyyy, r9.xxyz, r10.xxyz
    r1.yzw = ((r0.yyyy)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 92: mad r1.yzw, r1.yyzw, r0.yyyy, r7.xxyz
    r1.yzw = ((r1.yyzw)*(r0.yyyy)+(r7.xxyz)).yzw;
    // 93: mul r1.yzw, r0.yyyy, r1.yyzw
    r1.yzw = ((r0.yyyy)*(r1.yyzw)).yzw;
    // 94: max r1.yzw, r0.yyyy, r1.yyzw
    r1.yzw = (max(r0.yyyy,r1.yyzw)).yzw;
    // 95: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 96: mul r7.xy, r0.zwzz, cb0[8].wwww
    r7.xy = ((r0.zwzz)*(source[8].wwww)).xy;
    // 97: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 99: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 100: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 101: add r7.xyz, -r2.xyzx, r7.xyzx
    r7.xyz = ((-(r2.xyzx))+(r7.xyzx)).xyz;
    // 102: mad r0.xzw, r0.xxxx, r7.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r7.xxyz)+(r2.xxyz)).xzw;
    // 103: dp3 r2.x, r0.xzwx, r0.xzwx
    r2.x = (dot((r0.xzwx).xyz,(r0.xzwx).xyz).xxxx).x;
    // 104: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 105: mul r2.xyz, r0.xzwx, r2.xxxx
    r2.xyz = ((r0.xzwx)*(r2.xxxx)).xyz;
    // 106: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 107: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 108: mul r7.xyz, r2.wwww, v6.xyzx
    r7.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 109: dp3 r2.w, r7.xyzx, r2.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 110: mad r7.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 111: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 112: mul r7.yzw, r7.yyyy, cb0[25].xxyz
    r7.yzw = ((r7.yyyy)*(source[25].xxyz)).yzw;
    // 113: mad r7.xyz, r7.xxxx, cb0[24].xyzx, r7.yzwy
    r7.xyz = ((r7.xxxx)*(source[24].xyzx)+(r7.yzwy)).xyz;
    // 114: mul r7.xyz, r7.xyzx, cb0[26].wwww
    r7.xyz = ((r7.xyzx)*(source[26].wwww)).xyz;
    // 115: mul r9.xyz, r4.xyzx, r7.xyzx
    r9.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 116: dp2_sat r10.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 117: dp3_sat r10.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 118: dp3_sat r10.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 119: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 120: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t9.xyzw, s6
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 121: mul r11.xyz, r11.xyzx, cb0[28].xyzx
    r11.xyz = ((r11.xyzx)*(source[28].xyzx)).xyz;
    // 122: dp3 r2.w, r11.xyzx, r10.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 123: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s6
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 124: mul r10.xyz, r10.xyzx, cb0[27].xyzx
    r10.xyz = ((r10.xyzx)*(source[27].xyzx)).xyz;
    // 125: mul r12.xyz, r2.wwww, r10.xyzx
    r12.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 126: mad r9.xyz, r4.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 127: mul r1.yzw, r1.yyzw, r9.xxyz
    r1.yzw = ((r1.yyzw)*(r9.xxyz)).yzw;
    // 128: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 129: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 130: dp2 r12.z, r9.xyxx, cb0[15].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 131: dp3 r12.y, r5.xyzx, r2.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 132: mul r8.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 133: dp2 r12.x, r9.xyxx, r8.xyxx
    r12.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 134: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 135: dp4 r13.x, cb0[16].xyzw, r12.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 136: dp4 r13.y, cb0[17].xyzw, r12.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 137: dp4 r13.z, cb0[18].xyzw, r12.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 138: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 139: dp4 r15.x, cb0[19].xyzw, r14.xyzw
    r15.x = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 140: dp4 r15.y, cb0[20].xyzw, r14.xyzw
    r15.y = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 141: dp4 r15.z, cb0[21].xyzw, r14.xyzw
    r15.z = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 142: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 143: mul r3.w, r12.y, r12.y
    r3.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 144: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 145: mad r3.w, r12.x, r12.x, -r3.w
    r3.w = ((r12.xxxx)*(r12.xxxx)+(-(r3.wwww))).w;
    // 146: mad r12.xyz, cb0[22].xyzx, r3.wwww, r13.xyzx
    r12.xyz = ((source[22].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 147: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 148: mul r12.xyz, r12.xyzx, cb0[14].xyzx
    r12.xyz = ((r12.xyzx)*(source[14].xyzx)).xyz;
    // 149: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 150: mov_sat r4.w, cb0[12].x
    r4.w = (saturate(source[12].xxxx)).w;
    // 151: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 152: mul r3.w, r4.w, l(0.080000)
    r3.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 153: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 154: mad r13.xyz, r8.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r8.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 155: mul_sat r3.w, r13.y, l(50.000000)
    r3.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 156: log r4.w, |r1.x|
    r4.w = (log2(abs(r1.xxxx))).w;
    // 157: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 158: mul r4.w, r4.w, cb0[12].w
    r4.w = ((r4.wwww)*(source[12].wwww)).w;
    // 159: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 160: movc r1.x, r1.x, l(0), r4.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).x;
    // 161: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 162: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 163: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 164: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 165: mul r14.xyz, r1.xxxx, v5.xyzx
    r14.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 166: dp3 r1.x, r2.xyzx, r14.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 167: mul r2.xyz, r1.xxxx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 168: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 169: deriv_rtx_coarse r15.x, r1.x
    r15.x = (ddx_coarse(r1.xxxx)).x;
    // 170: deriv_rty_coarse r15.y, r1.x
    r15.y = (ddy_coarse(r1.xxxx)).y;
    // 171: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 172: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 173: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 174: mad_sat r15.y, r4.w, l(0.300000), r8.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 175: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 177: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 178: mul r16.xyz, r3.wwww, r16.xyzx
    r16.xyz = ((r3.wwww)*(r16.xyzx)).xyz;
    // 179: add r3.w, r2.z, l(1.000000)
    r3.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: add_sat r15.x, r1.x, -r3.w
    r15.x = (saturate((r1.xxxx)+(-(r3.wwww)))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 183: add r1.x, r0.y, r15.x
    r1.x = ((r0.yyyy)+(r15.xxxx)).x;
    // 184: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 185: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 186: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 187: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 188: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 189: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 190: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 191: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 193: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 194: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 195: mul r1.yzw, r1.yyzw, r12.xxyz
    r1.yzw = ((r1.yyzw)*(r12.xxyz)).yzw;
    // 196: mad r1.yzw, -r1.yyzw, r8.wwww, r1.yyzw
    r1.yzw = ((-(r1.yyzw))*(r8.wwww)+(r1.yyzw)).yzw;
    // 197: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 198: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 199: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 200: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 201: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 202: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 203: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 204: mul r1.x, r1.x, r3.y
    r1.x = ((r1.xxxx)*(r3.yyyy)).x;
    // 205: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 206: add r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)+(r1.xxxx)).x;
    // 207: mov o5.y, r0.y
    output.targets[5].y = (r0.yyyy).y;
    // 208: add_sat r0.y, r1.x, l(-1.000000)
    r0.y = (saturate((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 209: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 210: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t7.xyzw, s7, r3.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 211: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 212: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 213: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 214: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 215: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 216: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 217: mul r2.xyz, r5.xyzx, r5.xyzx
    r2.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 218: dp3 r1.x, r11.xyzx, r2.xyzx
    r1.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 219: add r2.x, -r1.x, r2.w
    r2.x = ((-(r1.xxxx))+(r2.wwww)).x;
    // 220: mad r1.x, r8.z, r2.x, r1.x
    r1.x = ((r8.zzzz)*(r2.xxxx)+(r1.xxxx)).x;
    // 221: mad r2.xyz, r10.xyzx, r1.xxxx, r7.xyzx
    r2.xyz = ((r10.xyzx)*(r1.xxxx)+(r7.xyzx)).xyz;
    // 222: mul r5.xyz, r1.xxxx, r10.xyzx
    r5.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 223: mad r1.x, r0.y, r13.x, r13.y
    r1.x = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).x;
    // 224: mad r1.x, r1.x, r0.y, r13.z
    r1.x = ((r1.xxxx)*(r0.yyyy)+(r13.zzzz)).x;
    // 225: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 226: max r0.y, r0.y, r1.x
    r0.y = (max(r0.yyyy,r1.xxxx)).y;
    // 227: mul r6.xyz, r0.yyyy, r2.xyzx
    r6.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 228: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 229: div r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)/(r2.xyzx)).xyz;
    // 230: dp3 r0.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 231: mul r2.xyz, r3.xyzx, r6.xyzx
    r2.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 232: mad r1.xyz, r2.xyzx, r15.xzwx, r1.yzwy
    r1.xyz = ((r2.xyzx)*(r15.xzwx)+(r1.yzwy)).xyz;
    // 233: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 234: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 235: dp3 r0.x, r0.xzwx, r14.xyzx
    r0.x = (dot((r0.xzwx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 236: add r0.z, -|r14.z|, l(1.000000)
    r0.z = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 237: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 238: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 239: lt r0.z, |r0.x|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 240: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 241: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 242: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 243: mul r2.xyz, r0.xxxx, cb0[4].xyzx
    r2.xyz = ((r0.xxxx)*(source[4].xyzx)).xyz;
    // 244: movc r0.xzw, r0.zzzz, l(0,0,0,0), r2.xxyz
    r0.xzw = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).xzw;
    // 245: add r0.xzw, r0.xxzw, cb0[1].xxyz
    r0.xzw = ((r0.xxzw)+(source[1].xxyz)).xzw;
    // 246: add r0.xzw, r1.xxyz, r0.xxzw
    r0.xzw = ((r1.xxyz)+(r0.xxzw)).xzw;
    // 247: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r0.xzwx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r0.xzwx)).xyz;
    // 248: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 249: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 250: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 251: mul r0.xzw, r0.xxxx, r9.xxyz
    r0.xzw = ((r0.xxxx)*(r9.xxyz)).xzw;
    // 252: ge r1.w, l(0.000000), r0.w
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 253: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 254: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 255: ge r2.xy, r0.xzxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xzxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 256: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 257: mad r2.xy, -|r0.zxzz|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.zxzz)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 258: movc r0.xz, r1.wwww, r2.xxyx, r0.xxzx
    r0.xz = ((asuint(r1.wwww) != 0u) ? (r2.xxyx) : (r0.xxzx)).xz;
    // 259: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 260: mul o4.z, r0.y, r1.x
    output.targets[4].z = ((r0.yyyy)*(r1.xxxx)).z;
    // 261: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 262: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 263: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 264: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 265: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 266: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 267: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 268: ret
    return output;
}

// source.character.static-map-native-1147.v1 / source program a0de2ebd81bd8d4d95e33a8ffb31bb90
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1147(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1147(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    source[10]=g_SourceCharacterBaseConstants[9];
    source[11]=g_SourceCharacterBaseConstants[10];
    source[12]=g_SourceCharacterBaseConstants[11];
    source[13]=g_SourceCharacterBaseConstants[12];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: mul r1.zw, r1.xxxy, cb0[8].xxxx
    r1.zw = ((r1.xxxy)*(source[8].xxxx)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.zw, r1.zzzw, cb0[8].yyyy
    r1.zw = ((r1.zzzw)*(source[8].yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mad r1.zw, cb0[7].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 17: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 20: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 24: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 27: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 28: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 29: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 30: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 31: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 32: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 33: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r0.x, r0.x, l(0.500000), cb0[9].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].wwww)).x;
    // 35: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[8].zzzz
    r0.zw = ((v4.xxxy)*(source[8].zzzz)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 43: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 44: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 45: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 46: max r1.w, cb0[9].x, l(0.000000)
    r1.w = (max(source[9].xxxx,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 47: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 48: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 49: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 50: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 51: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 53: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 54: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 55: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 56: mul r8.xyz, cb0[6].xyzx, cb0[10].yyyy
    r8.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 57: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 58: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 60: mad r7.xyz, cb0[10].wwww, r7.xyzx, r9.xyzx
    r7.xyz = ((source[10].wwww)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 61: mul r8.xyz, cb0[5].xyzx, cb0[10].xxxx
    r8.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 62: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 63: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 64: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r7.xyz, r4.xyzx, cb0[11].xxxx
    r7.xyz = ((r4.xyzx)*(source[11].xxxx)).xyz;
    // 67: mad r4.xyz, cb0[11].yyyy, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 68: mul r0.y, r1.z, cb0[11].z
    r0.y = ((r1.zzzz)*(source[11].zzzz)).y;
    // 69: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 70: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 71: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 72: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 73: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 74: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 75: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 76: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 77: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 79: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: mad r7.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r7.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 81: mad r9.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 82: mad r10.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 83: mul r0.y, r1.x, cb0[13].x
    r0.y = ((r1.xxxx)*(source[13].xxxx)).y;
    // 84: mul r1.x, r1.y, cb0[12].z
    r1.x = ((r1.yyyy)*(source[12].zzzz)).x;
    // 85: log r1.y, |r0.y|
    r1.y = (log2(abs(r0.yyyy))).y;
    // 86: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 87: mul r1.y, r1.y, cb0[13].y
    r1.y = ((r1.yyyy)*(source[13].yyyy)).y;
    // 88: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 89: min r1.y, r1.y, l(1.000000)
    r1.y = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 90: movc r0.y, r0.y, l(0), r1.y
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 91: mad r1.yzw, r0.yyyy, r9.xxyz, r10.xxyz
    r1.yzw = ((r0.yyyy)*(r9.xxyz)+(r10.xxyz)).yzw;
    // 92: mad r1.yzw, r1.yyzw, r0.yyyy, r7.xxyz
    r1.yzw = ((r1.yyzw)*(r0.yyyy)+(r7.xxyz)).yzw;
    // 93: mul r1.yzw, r0.yyyy, r1.yyzw
    r1.yzw = ((r0.yyyy)*(r1.yyzw)).yzw;
    // 94: max r1.yzw, r0.yyyy, r1.yyzw
    r1.yzw = (max(r0.yyyy,r1.yyzw)).yzw;
    // 95: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 96: mul r7.xy, r0.zwzz, cb0[8].wwww
    r7.xy = ((r0.zwzz)*(source[8].wwww)).xy;
    // 97: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 99: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 100: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 101: add r7.xyz, -r2.xyzx, r7.xyzx
    r7.xyz = ((-(r2.xyzx))+(r7.xyzx)).xyz;
    // 102: mad r0.xzw, r0.xxxx, r7.xxyz, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r7.xxyz)+(r2.xxyz)).xzw;
    // 103: dp3 r2.x, r0.xzwx, r0.xzwx
    r2.x = (dot((r0.xzwx).xyz,(r0.xzwx).xyz).xxxx).x;
    // 104: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 105: mul r2.xyz, r0.xzwx, r2.xxxx
    r2.xyz = ((r0.xzwx)*(r2.xxxx)).xyz;
    // 106: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 107: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 108: mul r7.xyz, r2.wwww, v6.xyzx
    r7.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 109: dp3 r2.w, r7.xyzx, r2.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 110: mad r8.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 111: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 112: mul r9.xyz, r8.yyyy, cb0[25].xyzx
    r9.xyz = ((r8.yyyy)*(source[25].xyzx)).xyz;
    // 113: mad r9.xyz, r8.xxxx, cb0[24].xyzx, r9.xyzx
    r9.xyz = ((r8.xxxx)*(source[24].xyzx)+(r9.xyzx)).xyz;
    // 114: mul r9.xyz, r9.xyzx, cb0[26].wwww
    r9.xyz = ((r9.xyzx)*(source[26].wwww)).xyz;
    // 115: mul r9.xyz, r4.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 116: mul r1.yzw, r1.yyzw, r9.xxyz
    r1.yzw = ((r1.yyzw)*(r9.xxyz)).yzw;
    // 117: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 118: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 119: dp2 r10.z, r9.xyxx, cb0[15].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 120: dp3 r10.y, r5.xyzx, r2.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 121: mul r8.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 122: dp2 r10.x, r9.xyxx, r8.xyxx
    r10.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 123: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 124: dp4 r11.x, cb0[16].xyzw, r10.xyzw
    r11.x = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 125: dp4 r11.y, cb0[17].xyzw, r10.xyzw
    r11.y = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 126: dp4 r11.z, cb0[18].xyzw, r10.xyzw
    r11.z = (dot((source[18].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 127: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 128: dp4 r13.x, cb0[19].xyzw, r12.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 129: dp4 r13.y, cb0[20].xyzw, r12.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 130: dp4 r13.z, cb0[21].xyzw, r12.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 131: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 132: mul r2.w, r10.y, r10.y
    r2.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 133: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 134: mad r2.w, r10.x, r10.x, -r2.w
    r2.w = ((r10.xxxx)*(r10.xxxx)+(-(r2.wwww))).w;
    // 135: mad r10.xyz, cb0[22].xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((source[22].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 136: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 137: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 138: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 139: log r2.w, |r1.x|
    r2.w = (log2(abs(r1.xxxx))).w;
    // 140: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 141: mul r2.w, r2.w, cb0[12].w
    r2.w = ((r2.wwww)*(source[12].wwww)).w;
    // 142: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 143: movc r1.x, r1.x, l(0), r2.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).x;
    // 144: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 145: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 147: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 148: mul r11.xyz, r1.xxxx, v5.xyzx
    r11.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 149: dp3 r1.x, r2.xyzx, r11.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 150: mul r2.xyz, r1.xxxx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r2.xyzx)).xyz;
    // 151: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 152: deriv_rtx_coarse r12.x, r1.x
    r12.x = (ddx_coarse(r1.xxxx)).x;
    // 153: deriv_rty_coarse r12.y, r1.x
    r12.y = (ddy_coarse(r1.xxxx)).y;
    // 154: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 155: dp2 r2.w, r12.xyxx, r12.xyxx
    r2.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: mad_sat r12.y, r2.w, l(0.300000), r8.z
    r12.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 158: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 159: add r2.w, r2.z, l(1.000000)
    r2.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: add_sat r12.x, r1.x, -r2.w
    r12.x = (saturate((r1.xxxx)+(-(r2.wwww)))).x;
    // 162: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t6.zwxy, s7
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 163: add r1.x, r0.y, r12.x
    r1.x = ((r0.yyyy)+(r12.xxxx)).x;
    // 164: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 165: add r2.w, -r12.y, l(1.000000)
    r2.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: mov_sat r4.w, cb0[12].x
    r4.w = (saturate(source[12].xxxx)).w;
    // 167: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 168: mul r3.w, r4.w, l(0.080000)
    r3.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 169: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 170: mad r13.xyz, r8.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r8.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 171: max r14.xyz, r2.wwww, r13.xyzx
    r14.xyz = (max(r2.wwww,r13.xyzx)).xyz;
    // 172: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 173: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 174: mul r14.xyz, r2.wwww, r14.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 175: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 176: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 177: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 178: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 179: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 180: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 184: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 185: mul r1.yzw, r1.yyzw, r10.xxyz
    r1.yzw = ((r1.yyzw)*(r10.xxyz)).yzw;
    // 186: mad r1.yzw, -r1.yyzw, r8.wwww, r1.yyzw
    r1.yzw = ((-(r1.yyzw))*(r8.wwww)+(r1.yyzw)).yzw;
    // 187: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 188: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 189: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 190: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 191: mul r2.w, r12.y, l(5.000000)
    r2.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 192: mul r3.x, r12.y, r12.y
    r3.x = ((r12.yyyy)*(r12.yyyy)).x;
    // 193: mul r1.x, r1.x, r3.x
    r1.x = ((r1.xxxx)*(r3.xxxx)).x;
    // 194: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 195: add r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)+(r1.xxxx)).x;
    // 196: mov o5.y, r0.y
    output.targets[5].y = (r0.yyyy).y;
    // 197: add_sat r0.y, r1.x, l(-1.000000)
    r0.y = (saturate((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 198: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 199: dp3 r1.x, r7.xyzx, r2.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 200: mad r2.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 201: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 202: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t7.xyzw, s6, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 203: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 204: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 205: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 206: mad r1.x, r0.y, r13.x, r13.y
    r1.x = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).x;
    // 207: mad r1.x, r1.x, r0.y, r13.z
    r1.x = ((r1.xxxx)*(r0.yyyy)+(r13.zzzz)).x;
    // 208: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 209: max r0.y, r0.y, r1.x
    r0.y = (max(r0.yyyy,r1.xxxx)).y;
    // 210: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 211: mad r2.xyz, cb0[24].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[24].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 212: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 213: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 214: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 215: mad r1.xyz, r2.xyzx, r12.xzwx, r1.yzwy
    r1.xyz = ((r2.xyzx)*(r12.xzwx)+(r1.yzwy)).xyz;
    // 216: mul r2.xyz, r12.xzwx, r2.xyzx
    r2.xyz = ((r12.xzwx)*(r2.xyzx)).xyz;
    // 217: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: dp3 r0.x, r0.xzwx, r11.xyzx
    r0.x = (dot((r0.xzwx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 219: add r0.y, -|r11.z|, l(1.000000)
    r0.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 220: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 221: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 222: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 223: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 224: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 225: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 226: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 227: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 228: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 229: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 230: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 231: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r0.xyzx)).xyz;
    // 232: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 233: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 234: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 235: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 236: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 237: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 238: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 239: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 240: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 241: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 242: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 243: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 244: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 245: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 246: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 247: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 248: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 249: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 250: ret
    return output;
}

// source.character.static-map-native-1148.v1 / source program 6431bca482cf3046a104a3e9fc8b48f0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1148(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[10];
    source[9].w=(g_SourceCharacterTime.xxxx).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[15]=g_SourceCharacterEnvironmentColor;source[16]=g_SourceCharacterEnvironmentRotation;}
    source[28]=1.f;
    source[29]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(-0.333300)
    r1.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 3: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 4: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 9: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
    // 10: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 14: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 16: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 17: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 18: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mul r2.xy, v4.xyxx, cb0[7].yyyy
    r2.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 26: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 27: max r1.w, cb0[7].w, l(0.000000)
    r1.w = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 29: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 30: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 32: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 33: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 35: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 36: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 37: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 38: mul r4.xyz, cb0[6].xyzx, cb0[11].xxxx
    r4.xyz = ((source[6].xyzx)*(source[11].xxxx)).xyz;
    // 39: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 40: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 42: mad r3.xyz, cb0[11].zzzz, r3.xyzx, r5.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 43: mul r4.xyz, cb0[5].xyzx, cb0[10].wwww
    r4.xyz = ((source[5].xyzx)*(source[10].wwww)).xyz;
    // 44: mad r3.xyz, -r0.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r0.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 46: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 47: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 48: mul r3.xyz, r0.xyzx, cb0[11].wwww
    r3.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 49: mad r0.xyz, cb0[12].xxxx, r0.xyzx, -r3.xyzx
    r0.xyz = ((source[12].xxxx)*(r0.xyzx)+(-(r3.xyzx))).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r1.w, r4.z, cb0[12].y
    r1.w = ((r4.zzzz)*(source[12].yyyy)).w;
    // 52: mul r2.zw, r4.yyyx, cb0[13].yyyw
    r2.zw = ((r4.yyyx)*(source[13].yyyw)).zw;
    // 53: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 54: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r3.w, r3.w, cb0[12].z
    r3.w = ((r3.wwww)*(source[12].zzzz)).w;
    // 56: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 57: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 58: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 60: mad r0.xyz, r3.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 61: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 63: mad_sat r3.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 64: mad r0.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 65: mad r5.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 66: mad r6.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r6.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 67: log r4.xy, |r2.zwzz|
    r4.xy = (log2(abs(r2.zwzz))).xy;
    // 68: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 69: mul r1.w, r4.y, cb0[14].x
    r1.w = ((r4.yyyy)*(source[14].xxxx)).w;
    // 70: mul r4.x, r4.x, cb0[13].z
    r4.x = ((r4.xxxx)*(source[13].zzzz)).x;
    // 71: exp r4.x, r4.x
    r4.x = (exp2(r4.xxxx)).x;
    // 72: movc r2.z, r2.z, l(0), r4.x
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).z;
    // 73: max r2.z, r2.z, cb0[0].x
    r2.z = (max(r2.zzzz,source[0].xxxx)).z;
    // 74: min r4.z, r2.z, l(1.000000)
    r4.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 75: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 76: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 78: mad r5.xyz, r1.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 79: mad r0.xyz, r5.xyzx, r1.wwww, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r1.wwww)+(r0.xyzx)).xyz;
    // 80: mul r0.xyz, r1.wwww, r0.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)).xyz;
    // 81: max r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 82: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 83: mul r5.xy, r2.xyxx, cb0[7].zzzz
    r5.xy = ((r2.xyxx)*(source[7].zzzz)).xy;
    // 84: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 86: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 87: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: add r2.xyz, -r1.xyzx, r5.xyzx
    r2.xyz = ((-(r1.xyzx))+(r5.xyzx)).xyz;
    // 89: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 90: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r2.xyz, r0.wwww, r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 93: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r0.w, r5.xyzx, r2.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 97: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 98: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 99: mul r5.xyz, r4.yyyy, cb0[26].xyzx
    r5.xyz = ((r4.yyyy)*(source[26].xyzx)).xyz;
    // 100: mad r5.xyz, r4.xxxx, cb0[25].xyzx, r5.xyzx
    r5.xyz = ((r4.xxxx)*(source[25].xyzx)+(r5.xyzx)).xyz;
    // 101: mul r5.xyz, r5.xyzx, cb0[27].wwww
    r5.xyz = ((r5.xyzx)*(source[27].wwww)).xyz;
    // 102: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 103: dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 104: dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 105: dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 106: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 107: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s5
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 108: mul r8.xyz, r8.xyzx, cb0[29].xyzx
    r8.xyz = ((r8.xyzx)*(source[29].xyzx)).xyz;
    // 109: dp3 r0.w, r8.xyzx, r7.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 110: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t7.xyzw, s5
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 111: mul r7.xyz, r7.xyzx, cb0[28].xyzx
    r7.xyz = ((r7.xyzx)*(source[28].xyzx)).xyz;
    // 112: mul r9.xyz, r0.wwww, r7.xyzx
    r9.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 113: mad r6.xyz, r3.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r3.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 114: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 115: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 116: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 117: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 118: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 119: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 120: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 121: mul r10.xyz, r2.wwww, v0.xyzx
    r10.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 122: mul r11.xyz, r6.zxyz, r10.yzxy
    r11.xyz = ((r6.zxyz)*(r10.yzxy)).xyz;
    // 123: mad r11.xyz, r6.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r6.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 124: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 125: dp3 r12.y, r11.xyzx, r2.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 126: dp3 r12.x, r10.xyzx, r2.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 127: dp2 r9.z, r12.xyxx, cb0[16].xyxx
    r9.z = (dot((r12.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 128: mul r4.xy, cb0[16].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[16].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 129: dp2 r9.x, r12.xyxx, r4.xyxx
    r9.x = (dot((r12.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 130: mov r9.w, l(1.000000)
    r9.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 131: dp4 r13.x, cb0[17].xyzw, r9.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 132: dp4 r13.y, cb0[18].xyzw, r9.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 133: dp4 r13.z, cb0[19].xyzw, r9.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 134: mul r14.xyzw, r9.yzzx, r9.xyzz
    r14.xyzw = ((r9.yzzx)*(r9.xyzz)).xyzw;
    // 135: dp4 r15.x, cb0[20].xyzw, r14.xyzw
    r15.x = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 136: dp4 r15.y, cb0[21].xyzw, r14.xyzw
    r15.y = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 137: dp4 r15.z, cb0[22].xyzw, r14.xyzw
    r15.z = (dot((source[22].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 138: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 139: mul r2.w, r9.y, r9.y
    r2.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 140: mov r12.z, r9.y
    r12.z = (r9.yyyy).z;
    // 141: mad r2.w, r9.x, r9.x, -r2.w
    r2.w = ((r9.xxxx)*(r9.xxxx)+(-(r2.wwww))).w;
    // 142: mad r9.xyz, cb0[23].xyzx, r2.wwww, r13.xyzx
    r9.xyz = ((source[23].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 143: max r9.xyz, r9.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 144: mul r9.xyz, r9.xyzx, cb0[15].xyzx
    r9.xyz = ((r9.xyzx)*(source[15].xyzx)).xyz;
    // 145: mad r9.xyz, r9.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r9.xyz = ((r9.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 146: mov_sat r3.w, cb0[12].w
    r3.w = (saturate(source[12].wwww)).w;
    // 147: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 148: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 149: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 150: mad r13.xyz, r4.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r4.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 151: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 152: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 153: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 154: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 155: dp3 r3.w, r2.xyzx, r14.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 156: mul r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // 157: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 158: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 159: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 160: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 162: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 163: mad_sat r15.y, r5.w, l(0.300000), r4.z
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 164: add r5.w, -r15.y, l(1.000000)
    r5.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: max r16.xyz, r13.xyzx, r5.wwww
    r16.xyz = (max(r13.xyzx,r5.wwww)).xyz;
    // 166: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 167: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 168: add r2.w, r2.z, l(1.000000)
    r2.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
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
    // 183: mul r9.xyz, r9.xyzx, r17.xyzx
    r9.xyz = ((r9.xyzx)*(r17.xyzx)).xyz;
    // 184: mul r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 185: mad r0.xyz, -r0.xyzx, r4.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r4.wwww)+(r0.xyzx)).xyz;
    // 186: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 187: dp2_sat r9.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 188: dp3_sat r9.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 189: dp3_sat r9.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 190: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 191: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 192: add r0.w, r0.w, -r3.w
    r0.w = ((r0.wwww)+(-(r3.wwww))).w;
    // 193: mad r0.w, r4.z, r0.w, r3.w
    r0.w = ((r4.zzzz)*(r0.wwww)+(r3.wwww)).w;
    // 194: mad r5.xyz, r7.xyzx, r0.wwww, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 195: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 196: mul r0.w, r15.y, r15.y
    r0.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 197: mul r3.w, r15.y, l(5.000000)
    r3.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 198: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 199: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 200: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 201: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 202: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 203: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 204: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 205: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 206: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 207: mul r8.xyz, r0.wwww, r5.xyzx
    r8.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 208: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 209: div r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)/(r5.xyzx)).xyz;
    // 210: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 211: dp3 r5.x, r10.xyzx, r2.xyzx
    r5.x = (dot((r10.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 212: dp3 r5.y, r11.xyzx, r2.xyzx
    r5.y = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 213: dp3 r2.y, r6.xyzx, r2.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 214: dp2 r2.x, r5.xyxx, r4.xyxx
    r2.x = (dot((r5.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 215: dp2 r2.z, r5.xyxx, cb0[16].xyxx
    r2.z = (dot((r5.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 216: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t6.xyzw, s6, r3.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 217: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 218: mul r2.xyz, r2.xyzx, cb0[15].xyzx
    r2.xyz = ((r2.xyzx)*(source[15].xyzx)).xyz;
    // 219: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 220: mul r2.xyz, r8.xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 221: mad r0.xyz, r2.xyzx, r15.xzwx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r15.xzwx)+(r0.xyzx)).xyz;
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
    // 228: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 229: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 230: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 231: mul r1.yzw, r1.yyyy, cb0[2].xxyz
    r1.yzw = ((r1.yyyy)*(source[2].xxyz)).yzw;
    // 232: lt r2.x, |r1.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 233: movc r1.yzw, r2.xxxx, l(0,0,0,0), r1.yyzw
    r1.yzw = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzw)).yzw;
    // 234: mad_sat r2.x, r1.x, cb0[8].z, -cb0[8].w
    r2.x = (saturate((r1.xxxx)*(source[8].zzzz)+(-(source[8].wwww)))).x;
    // 235: log r2.y, r2.x
    r2.y = (log2(r2.xxxx)).y;
    // 236: lt r2.x, r2.x, l(0.000001)
    r2.x = (asfloat((uint4)((r2.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 237: mul r2.y, r2.y, cb0[9].x
    r2.y = ((r2.yyyy)*(source[9].xxxx)).y;
    // 238: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 239: mul r2.yzw, r2.yyyy, cb0[3].xxyz
    r2.yzw = ((r2.yyyy)*(source[3].xxyz)).yzw;
    // 240: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 241: add r2.xyz, r2.xyzx, -cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[3].xyzx))).xyz;
    // 242: mad r2.xyz, cb0[3].wwww, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((source[3].wwww)*(r2.xyzx)+(source[3].xyzx)).xyz;
    // 243: mul r4.xyz, cb0[4].xyzx, cb0[9].zzzz
    r4.xyz = ((source[4].xyzx)*(source[9].zzzz)).xyz;
    // 244: mul r4.xyz, r4.xyzx, cb0[10].yyyy
    r4.xyz = ((r4.xyzx)*(source[10].yyyy)).xyz;
    // 245: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 246: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 247: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 248: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 249: mul r4.xyz, r4.xyzx, cb0[10].zzzz
    r4.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 250: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 251: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 252: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 253: mad r1.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.yzwy
    r1.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.yzwy)).xyz;
    // 254: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 255: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 256: mad o0.xyz, r3.xyzx, cb0[27].xyzx, r1.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 257: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 258: dp3 r1.x, r12.xyzx, r12.xyzx
    r1.x = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 259: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 260: mul r1.xyz, r1.xxxx, r12.xyzx
    r1.xyz = ((r1.xxxx)*(r12.xyzx)).xyz;
    // 261: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 262: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 263: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 264: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 265: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 266: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 267: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 268: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 269: mul o4.z, r0.w, r0.x
    output.targets[4].z = ((r0.wwww)*(r0.xxxx)).z;
    // 270: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 271: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 272: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
    // 273: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 274: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 275: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 276: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 277: ret
    return output;
}

// source.character.static-map-native-1148.v1 / source program 9e305986228dfb44b4e7b3dcc3d6c385
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1148(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1148(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[10];
    source[9].w=(g_SourceCharacterTime.xxxx).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[15]=g_SourceCharacterEnvironmentColor;source[16]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(-0.333300)
    r1.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 3: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 4: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 9: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
    // 10: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 12: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 13: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 14: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 16: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 17: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 18: mul r1.w, r1.z, r1.z
    r1.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mul r2.xy, v4.xyxx, cb0[7].yyyy
    r2.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 26: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 27: max r1.w, cb0[7].w, l(0.000000)
    r1.w = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 29: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 30: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 32: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 33: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 35: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 36: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 37: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 38: mul r4.xyz, cb0[6].xyzx, cb0[11].xxxx
    r4.xyz = ((source[6].xyzx)*(source[11].xxxx)).xyz;
    // 39: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 40: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 41: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 42: mad r3.xyz, cb0[11].zzzz, r3.xyzx, r5.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 43: mul r4.xyz, cb0[5].xyzx, cb0[10].wwww
    r4.xyz = ((source[5].xyzx)*(source[10].wwww)).xyz;
    // 44: mad r3.xyz, -r0.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r0.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 45: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 46: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 47: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 48: mul r3.xyz, r0.xyzx, cb0[11].wwww
    r3.xyz = ((r0.xyzx)*(source[11].wwww)).xyz;
    // 49: mad r0.xyz, cb0[12].xxxx, r0.xyzx, -r3.xyzx
    r0.xyz = ((source[12].xxxx)*(r0.xyzx)+(-(r3.xyzx))).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r1.w, r4.z, cb0[12].y
    r1.w = ((r4.zzzz)*(source[12].yyyy)).w;
    // 52: mul r2.zw, r4.yyyx, cb0[13].yyyw
    r2.zw = ((r4.yyyx)*(source[13].yyyw)).zw;
    // 53: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 54: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: mul r3.w, r3.w, cb0[12].z
    r3.w = ((r3.wwww)*(source[12].zzzz)).w;
    // 56: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 57: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 58: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 59: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 60: mad r0.xyz, r3.wwww, r0.xyzx, r3.xyzx
    r0.xyz = ((r3.wwww)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 61: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 63: mad_sat r3.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 64: dp2 r0.x, r2.xyxx, r2.xyxx
    r0.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 65: mul r5.xy, r2.xyxx, cb0[7].zzzz
    r5.xy = ((r2.xyxx)*(source[7].zzzz)).xy;
    // 66: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 67: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 68: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 69: add r5.z, r0.x, l(0.000010)
    r5.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 70: add r0.xyz, -r1.xyzx, r5.xyzx
    r0.xyz = ((-(r1.xyzx))+(r5.xyzx)).xyz;
    // 71: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 72: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 75: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 76: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 77: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 78: dp3 r0.w, r5.xyzx, r1.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 79: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 80: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 81: mul r6.xyz, r2.yyyy, cb0[26].xyzx
    r6.xyz = ((r2.yyyy)*(source[26].xyzx)).xyz;
    // 82: mad r6.xyz, r2.xxxx, cb0[25].xyzx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(source[25].xyzx)+(r6.xyzx)).xyz;
    // 83: mul r6.xyz, r6.xyzx, cb0[27].wwww
    r6.xyz = ((r6.xyzx)*(source[27].wwww)).xyz;
    // 84: mul r6.xyz, r3.xyzx, r6.xyzx
    r6.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 85: mad r7.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 86: mad r8.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 87: mad r9.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 88: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 89: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 90: mul r0.w, r2.y, cb0[14].x
    r0.w = ((r2.yyyy)*(source[14].xxxx)).w;
    // 91: mul r1.w, r2.x, cb0[13].z
    r1.w = ((r2.xxxx)*(source[13].zzzz)).w;
    // 92: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 93: movc r1.w, r2.z, l(0), r1.w
    r1.w = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 94: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 95: min r4.z, r1.w, l(1.000000)
    r4.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 97: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: movc r0.w, r2.w, l(0), r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 99: mad r2.xyz, r0.wwww, r8.xyzx, r9.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 100: mad r2.xyz, r2.xyzx, r0.wwww, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 101: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 102: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 103: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 104: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 105: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 106: mul r6.xyz, r1.wwww, v1.xyzx
    r6.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 107: dp3 r7.y, r6.xyzx, r1.xyzx
    r7.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 108: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 109: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 110: mul r8.xyz, r1.wwww, v0.xyzx
    r8.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 111: mul r9.xyz, r6.zxyz, r8.yzxy
    r9.xyz = ((r6.zxyz)*(r8.yzxy)).xyz;
    // 112: mad r9.xyz, r6.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r6.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 113: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 114: dp3 r10.y, r9.xyzx, r1.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 115: dp3 r10.x, r8.xyzx, r1.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 116: dp2 r7.z, r10.xyxx, cb0[16].xyxx
    r7.z = (dot((r10.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 117: mul r4.xy, cb0[16].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[16].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 118: dp2 r7.x, r10.xyxx, r4.xyxx
    r7.x = (dot((r10.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 119: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 120: dp4 r11.x, cb0[17].xyzw, r7.xyzw
    r11.x = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 121: dp4 r11.y, cb0[18].xyzw, r7.xyzw
    r11.y = (dot((source[18].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 122: dp4 r11.z, cb0[19].xyzw, r7.xyzw
    r11.z = (dot((source[19].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 123: mul r12.xyzw, r7.yzzx, r7.xyzz
    r12.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 124: dp4 r13.x, cb0[20].xyzw, r12.xyzw
    r13.x = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 125: dp4 r13.y, cb0[21].xyzw, r12.xyzw
    r13.y = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 126: dp4 r13.z, cb0[22].xyzw, r12.xyzw
    r13.z = (dot((source[22].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 127: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 128: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 129: mov r10.z, r7.y
    r10.z = (r7.yyyy).z;
    // 130: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 131: mad r7.xyz, cb0[23].xyzx, r1.wwww, r11.xyzx
    r7.xyz = ((source[23].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 132: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 133: mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // 134: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 135: mov_sat r3.w, cb0[12].w
    r3.w = (saturate(source[12].wwww)).w;
    // 136: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 137: mul r1.w, r3.w, l(0.080000)
    r1.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 138: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 139: mad r11.xyz, r4.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 140: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 141: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 142: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 143: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 144: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 145: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 146: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
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
    // 152: mad_sat r13.y, r3.w, l(0.300000), r4.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 153: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 154: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 156: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 157: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 158: add r1.w, r1.z, l(1.000000)
    r1.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 161: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 162: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 163: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 164: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 165: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 166: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
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
    // 173: mul r7.xyz, r7.xyzx, r15.xyzx
    r7.xyz = ((r7.xyzx)*(r15.xyzx)).xyz;
    // 174: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 175: mad r2.xyz, -r2.xyzx, r4.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r4.wwww)+(r2.xyzx)).xyz;
    // 176: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 177: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 178: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 179: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 180: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 181: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 182: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 183: dp3 r7.x, r8.xyzx, r1.xyzx
    r7.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 184: dp3 r7.y, r9.xyzx, r1.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 185: dp2 r4.x, r7.xyxx, r4.xyxx
    r4.x = (dot((r7.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 186: dp2 r4.z, r7.xyxx, cb0[16].xyxx
    r4.z = (dot((r7.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 187: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 188: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 189: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 190: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 191: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r4.xyzx, t6.xyzw, s5, r2.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r4.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 192: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 193: mul r4.xyz, r4.xyzx, cb0[15].xyzx
    r4.xyz = ((r4.xyzx)*(source[15].xyzx)).xyz;
    // 194: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 195: mad r1.z, r0.w, r11.x, r11.y
    r1.z = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).z;
    // 196: mad r1.z, r1.z, r0.w, r11.z
    r1.z = ((r1.zzzz)*(r0.wwww)+(r11.zzzz)).z;
    // 197: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 198: max r0.w, r0.w, r1.z
    r0.w = (max(r0.wwww,r1.zzzz)).w;
    // 199: mul r1.yzw, r1.yyyy, cb0[26].xxyz
    r1.yzw = ((r1.yyyy)*(source[26].xxyz)).yzw;
    // 200: mad r1.xyz, cb0[25].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[25].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 201: mul r1.xyz, r1.xyzx, cb0[27].wwww
    r1.xyz = ((r1.xyzx)*(source[27].wwww)).xyz;
    // 202: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 203: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 204: mad r2.xyz, r1.xyzx, r13.xzwx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r13.xzwx)+(r2.xyzx)).xyz;
    // 205: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 206: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 207: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 208: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 209: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 210: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 211: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 212: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 213: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 214: mul r0.yzw, r0.yyyy, cb0[2].xxyz
    r0.yzw = ((r0.yyyy)*(source[2].xxyz)).yzw;
    // 215: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 216: movc r0.yzw, r1.xxxx, l(0,0,0,0), r0.yyzw
    r0.yzw = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzw)).yzw;
    // 217: mad_sat r1.x, r0.x, cb0[8].z, -cb0[8].w
    r1.x = (saturate((r0.xxxx)*(source[8].zzzz)+(-(source[8].wwww)))).x;
    // 218: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 219: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 220: mul r1.y, r1.y, cb0[9].x
    r1.y = ((r1.yyyy)*(source[9].xxxx)).y;
    // 221: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 222: mul r1.yzw, r1.yyyy, cb0[3].xxyz
    r1.yzw = ((r1.yyyy)*(source[3].xxyz)).yzw;
    // 223: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 224: add r1.xyz, r1.xyzx, -cb0[3].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[3].xyzx))).xyz;
    // 225: mad r1.xyz, cb0[3].wwww, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((source[3].wwww)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 226: mul r4.xyz, cb0[4].xyzx, cb0[9].zzzz
    r4.xyz = ((source[4].xyzx)*(source[9].zzzz)).xyz;
    // 227: mul r4.xyz, r4.xyzx, cb0[10].yyyy
    r4.xyz = ((r4.xyzx)*(source[10].yyyy)).xyz;
    // 228: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 229: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 230: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 231: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 232: mul r4.xyz, r4.xyzx, cb0[10].zzzz
    r4.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 233: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 234: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 235: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 236: mad r0.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r0.yzwy
    r0.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r0.yzwy)).xyz;
    // 237: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 238: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 239: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 240: mad o0.xyz, r3.xyzx, cb0[27].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[27].xyzx)+(r0.xyzx)).xyz;
    // 241: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 242: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 243: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 244: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 245: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 246: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 247: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 248: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 249: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 250: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 251: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 252: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 253: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 254: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
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

// source.character.static-map-native-1149.v1 / source program 38de457e1cbeda46a0dbc85c2c5942e2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1149(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[10];
    source[9].w=(g_SourceCharacterTime.xxxx).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[27]=1.f;
    source[28]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 3: add r0.z, r1.w, l(-0.333300)
    r0.z = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 4: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 5: discard_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 9: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 11: mul r0.xy, r0.xyxx, cb0[7].wwww
    r0.xy = ((r0.xyxx)*(source[7].wwww)).xy;
    // 12: mul r3.xy, r0.xyxx, v2.wwww
    r3.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 13: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: add r3.z, r0.x, l(0.000010)
    r3.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 18: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 19: div r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 20: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 23: dp3 r0.w, r0.xyzx, r3.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 24: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: add r1.w, -|r3.z|, l(1.000000)
    r1.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 26: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 27: mad_sat r1.w, r0.w, cb0[8].z, -cb0[8].w
    r1.w = (saturate((r0.wwww)*(source[8].zzzz)+(-(source[8].wwww)))).w;
    // 28: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 29: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 30: mul r2.w, r2.w, cb0[9].x
    r2.w = ((r2.wwww)*(source[9].xxxx)).w;
    // 31: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 32: mul r4.xyz, r2.wwww, cb0[4].xyzx
    r4.xyz = ((r2.wwww)*(source[4].xyzx)).xyz;
    // 33: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 34: add r4.xyz, r4.xyzx, -cb0[4].xyzx
    r4.xyz = ((r4.xyzx)+(-(source[4].xyzx))).xyz;
    // 35: mad r4.xyz, cb0[4].wwww, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((source[4].wwww)*(r4.xyzx)+(source[4].xyzx)).xyz;
    // 36: mul r5.xyz, cb0[5].xyzx, cb0[9].zzzz
    r5.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 37: mul r5.xyz, r5.xyzx, cb0[10].yyyy
    r5.xyz = ((r5.xyzx)*(source[10].yyyy)).xyz;
    // 38: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 39: mul r5.xyz, r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 40: max r5.xyz, |r5.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r5.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 41: log r5.xyz, r5.xyzx
    r5.xyz = (log2(r5.xyzx)).xyz;
    // 42: mul r5.xyz, r5.xyzx, cb0[10].zzzz
    r5.xyz = ((r5.xyzx)*(source[10].zzzz)).xyz;
    // 43: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 44: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 46: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 47: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 48: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 49: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 50: mul r5.xyz, r1.wwww, cb0[3].xyzx
    r5.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 51: movc r5.xyz, r0.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 52: mad r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r5.xyzx
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r5.xyzx)).xyz;
    // 53: add r4.xyz, r4.xyzx, cb0[1].xyzx
    r4.xyz = ((r4.xyzx)+(source[1].xyzx)).xyz;
    // 54: mul r5.xyz, cb0[6].xyzx, cb0[10].wwww
    r5.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 55: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 56: mul r5.xyz, r1.xyzx, cb0[11].xxxx
    r5.xyz = ((r1.xyzx)*(source[11].xxxx)).xyz;
    // 57: mad r1.xyz, cb0[11].yyyy, r1.xyzx, -r5.xyzx
    r1.xyz = ((source[11].yyyy)*(r1.xyzx)+(-(r5.xyzx))).xyz;
    // 58: mul r0.w, r2.z, cb0[11].z
    r0.w = ((r2.zzzz)*(source[11].zzzz)).w;
    // 59: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 60: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 61: mul r1.w, r1.w, cb0[11].w
    r1.w = ((r1.wwww)*(source[11].wwww)).w;
    // 62: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 63: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 64: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 66: mad r1.xyz, r1.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 67: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 68: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 69: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 70: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 71: mad r6.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r6.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 72: mul r0.w, r2.x, cb0[13].x
    r0.w = ((r2.xxxx)*(source[13].xxxx)).w;
    // 73: mul r2.x, r2.y, cb0[12].z
    r2.x = ((r2.yyyy)*(source[12].zzzz)).x;
    // 74: log r2.y, |r0.w|
    r2.y = (log2(abs(r0.wwww))).y;
    // 75: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 76: mul r2.y, r2.y, cb0[13].y
    r2.y = ((r2.yyyy)*(source[13].yyyy)).y;
    // 77: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 78: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 79: movc r0.w, r0.w, l(0), r2.y
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yyyy)).w;
    // 80: mad r5.xyz, r0.wwww, r5.xyzx, r6.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 81: mad r6.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 82: mad r5.xyz, r5.xyzx, r0.wwww, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 83: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 84: max r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = (max(r0.wwww,r5.xyzx)).xyz;
    // 85: dp3 r2.y, v6.xyzx, v6.xyzx
    r2.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 86: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 87: mul r6.xyz, r2.yyyy, v6.xyzx
    r6.xyz = ((r2.yyyy)*(v6.xyzx)).xyz;
    // 88: dp3 r2.y, r0.xyzx, r0.xyzx
    r2.y = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 89: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 90: mul r0.xyz, r0.xyzx, r2.yyyy
    r0.xyz = ((r0.xyzx)*(r2.yyyy)).xyz;
    // 91: dp3 r2.y, r6.xyzx, r0.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 92: mad r6.xy, r2.yyyy, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r2.yyyy)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 93: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 94: mul r6.yzw, r6.yyyy, cb0[25].xxyz
    r6.yzw = ((r6.yyyy)*(source[25].xxyz)).yzw;
    // 95: mad r6.xyz, r6.xxxx, cb0[24].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[24].xyzx)+(r6.yzwy)).xyz;
    // 96: mul r6.xyz, r6.xyzx, cb0[26].wwww
    r6.xyz = ((r6.xyzx)*(source[26].wwww)).xyz;
    // 97: mul r7.xyz, r1.xyzx, r6.xyzx
    r7.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 98: dp2_sat r8.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 99: dp3_sat r8.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 100: dp3_sat r8.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 101: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 102: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t6.xyzw, s3
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 103: mul r9.xyz, r9.xyzx, cb0[28].xyzx
    r9.xyz = ((r9.xyzx)*(source[28].xyzx)).xyz;
    // 104: dp3 r2.y, r9.xyzx, r8.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 105: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t5.xyzw, s3
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 106: mul r8.xyz, r8.xyzx, cb0[27].xyzx
    r8.xyz = ((r8.xyzx)*(source[27].xyzx)).xyz;
    // 107: mul r10.xyz, r2.yyyy, r8.xyzx
    r10.xyz = ((r2.yyyy)*(r8.xyzx)).xyz;
    // 108: mad r7.xyz, r1.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 109: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 110: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 111: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 112: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 113: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 114: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 115: mul r10.xyz, r3.wwww, v0.xyzx
    r10.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 116: mul r11.xyz, r7.zxyz, r10.yzxy
    r11.xyz = ((r7.zxyz)*(r10.yzxy)).xyz;
    // 117: mad r11.xyz, r7.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r7.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 118: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 119: dp3 r12.y, r11.xyzx, r0.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 120: dp3 r12.x, r10.xyzx, r0.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 121: dp2 r13.z, r12.xyxx, cb0[15].xyxx
    r13.z = (dot((r12.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 122: mul r14.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r14.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 123: dp2 r13.x, r12.xyxx, r14.xyxx
    r13.x = (dot((r12.xyxx).xy,(r14.xyxx).xy).xxxx).x;
    // 124: dp3 r13.y, r7.xyzx, r0.xyzx
    r13.y = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 125: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 126: dp4 r15.x, cb0[16].xyzw, r13.xyzw
    r15.x = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 127: dp4 r15.y, cb0[17].xyzw, r13.xyzw
    r15.y = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 128: dp4 r15.z, cb0[18].xyzw, r13.xyzw
    r15.z = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 129: mul r16.xyzw, r13.yzzx, r13.xyzz
    r16.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 130: dp4 r17.x, cb0[19].xyzw, r16.xyzw
    r17.x = (dot((source[19].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).x;
    // 131: dp4 r17.y, cb0[20].xyzw, r16.xyzw
    r17.y = (dot((source[20].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).y;
    // 132: dp4 r17.z, cb0[21].xyzw, r16.xyzw
    r17.z = (dot((source[21].xyzw).xyzw,(r16.xyzw).xyzw).xxxx).z;
    // 133: add r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)+(r17.xyzx)).xyz;
    // 134: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 135: mov r12.z, r13.y
    r12.z = (r13.yyyy).z;
    // 136: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 137: mad r13.xyz, cb0[22].xyzx, r3.wwww, r15.xyzx
    r13.xyz = ((source[22].xyzx)*(r3.wwww)+(r15.xyzx)).xyz;
    // 138: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 139: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 140: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 141: log r3.w, |r2.x|
    r3.w = (log2(abs(r2.xxxx))).w;
    // 142: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 143: mul r3.w, r3.w, cb0[12].w
    r3.w = ((r3.wwww)*(source[12].wwww)).w;
    // 144: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 145: movc r2.x, r2.x, l(0), r3.w
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).x;
    // 146: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 147: min r2.z, r2.x, l(1.000000)
    r2.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 148: dp3 r2.x, r0.xyzx, r3.xyzx
    r2.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 149: mul r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = ((r0.xyzx)*(r2.xxxx)).xyz;
    // 150: mad r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r0.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 151: deriv_rtx_coarse r3.x, r2.x
    r3.x = (ddx_coarse(r2.xxxx)).x;
    // 152: deriv_rty_coarse r3.y, r2.x
    r3.y = (ddy_coarse(r2.xxxx)).y;
    // 153: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 154: dp2 r3.x, r3.xyxx, r3.xyxx
    r3.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 155: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 156: mad_sat r3.y, r3.x, l(0.300000), r2.z
    r3.y = (saturate((r3.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 157: add r3.z, -r3.y, l(1.000000)
    r3.z = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 158: mov_sat r1.w, cb0[12].x
    r1.w = (saturate(source[12].xxxx)).w;
    // 159: mad r15.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r15.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 160: mul r3.w, r1.w, l(0.080000)
    r3.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 161: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 162: mad r15.xyz, r2.wwww, r15.xyzx, r3.wwww
    r15.xyz = ((r2.wwww)*(r15.xyzx)+(r3.wwww)).xyz;
    // 163: max r16.xyz, r3.zzzz, r15.xyzx
    r16.xyz = (max(r3.zzzz,r15.xyzx)).xyz;
    // 164: add r16.xyz, -r15.xyzx, r16.xyzx
    r16.xyz = ((-(r15.xyzx))+(r16.xyzx)).xyz;
    // 165: mul_sat r1.w, r15.y, l(50.000000)
    r1.w = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 166: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 167: add r1.w, r0.z, l(1.000000)
    r1.w = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add_sat r3.x, -r1.w, r2.x
    r3.x = (saturate((-(r1.wwww))+(r2.xxxx))).x;
    // 170: sample_indexable(texture2d)(float,float,float,float) r3.zw, r3.xyxx, t3.zwxy, s5
    r3.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 171: add r1.w, r0.w, r3.x
    r1.w = ((r0.wwww)+(r3.xxxx)).w;
    // 172: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 173: mul r17.xyz, r3.wwww, r15.xyzx
    r17.xyz = ((r3.wwww)*(r15.xyzx)).xyz;
    // 174: mad r16.xyz, r16.xyzx, r3.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r3.zzzz)+(r17.xyzx)).xyz;
    // 175: div r2.x, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r2.x = r3.w != 0.f ? 1.f / r3.w : 0.f;
    // 176: add r2.x, r2.x, l(-1.000000)
    r2.x = ((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 177: mad r3.xzw, r15.xxyz, r2.xxxx, l(1.000000, 0.000000, 1.000000, 1.000000)
    r3.xzw = ((r15.xxyz)*(r2.xxxx)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 178: dp3 r2.x, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 179: mad r15.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 180: mad r17.xyz, -r16.xyzx, r3.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r3.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mul r3.xzw, r3.xxzw, r16.xxyz
    r3.xzw = ((r3.xxzw)*(r16.xxyz)).xzw;
    // 182: mul r13.xyz, r13.xyzx, r17.xyzx
    r13.xyz = ((r13.xyzx)*(r17.xyzx)).xyz;
    // 183: mul r5.xyz, r5.xyzx, r13.xyzx
    r5.xyz = ((r5.xyzx)*(r13.xyzx)).xyz;
    // 184: mad r5.xyz, -r5.xyzx, r2.wwww, r5.xyzx
    r5.xyz = ((-(r5.xyzx))*(r2.wwww)+(r5.xyzx)).xyz;
    // 185: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 186: mul r2.x, r3.y, l(5.000000)
    r2.x = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 187: mul r2.w, r3.y, r3.y
    r2.w = ((r3.yyyy)*(r3.yyyy)).w;
    // 188: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 189: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 190: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 191: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 192: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 193: dp3 r10.x, r10.xyzx, r0.xyzx
    r10.x = (dot((r10.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 194: dp3 r10.y, r11.xyzx, r0.xyzx
    r10.y = (dot((r11.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 195: dp2 r11.x, r10.xyxx, r14.xyxx
    r11.x = (dot((r10.xyxx).xy,(r14.xyxx).xy).xxxx).x;
    // 196: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 197: dp3 r11.y, r7.xyzx, r0.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 198: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r11.xyzx, t4.xyzw, s4, r2.x
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r11.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 199: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 200: mul r7.xyz, r7.xyzx, cb0[14].xyzx
    r7.xyz = ((r7.xyzx)*(source[14].xyzx)).xyz;
    // 201: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 202: dp2_sat r10.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 203: dp3_sat r10.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 204: dp3_sat r10.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 205: mul r0.xyz, r10.xyzx, r10.xyzx
    r0.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 206: dp3 r0.x, r9.xyzx, r0.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 207: add r0.y, -r0.x, r2.y
    r0.y = ((-(r0.xxxx))+(r2.yyyy)).y;
    // 208: mad r0.x, r2.z, r0.y, r0.x
    r0.x = ((r2.zzzz)*(r0.yyyy)+(r0.xxxx)).x;
    // 209: mad r2.xyz, r8.xyzx, r0.xxxx, r6.xyzx
    r2.xyz = ((r8.xyzx)*(r0.xxxx)+(r6.xyzx)).xyz;
    // 210: mul r0.xyz, r0.xxxx, r8.xyzx
    r0.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 211: mad r1.w, r0.w, r15.x, r15.y
    r1.w = ((r0.wwww)*(r15.xxxx)+(r15.yyyy)).w;
    // 212: mad r1.w, r1.w, r0.w, r15.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r15.zzzz)).w;
    // 213: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 214: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 215: mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 216: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 217: div r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)/(r2.xyzx)).xyz;
    // 218: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 219: mul r0.yzw, r6.xxyz, r7.xxyz
    r0.yzw = ((r6.xxyz)*(r7.xxyz)).yzw;
    // 220: mad r2.xyz, r0.yzwy, r3.xzwx, r5.xyzx
    r2.xyz = ((r0.yzwy)*(r3.xzwx)+(r5.xyzx)).xyz;
    // 221: mul r0.yzw, r3.xxzw, r0.yyzw
    r0.yzw = ((r3.xxzw)*(r0.yyzw)).yzw;
    // 222: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 223: add r0.yzw, r2.xxyz, r4.xxyz
    r0.yzw = ((r2.xxyz)+(r4.xxyz)).yzw;
    // 224: mad o0.xyz, r1.xyzx, cb0[26].xyzx, r0.yzwy
    output.targets[0].xyz = ((r1.xyzx)*(source[26].xyzx)+(r0.yzwy)).xyz;
    // 225: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 226: dp3 r0.y, r12.xyzx, r12.xyzx
    r0.y = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 227: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 228: mul r0.yzw, r0.yyyy, r12.xxyz
    r0.yzw = ((r0.yyyy)*(r12.xxyz)).yzw;
    // 229: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 230: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 231: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 232: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 233: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 234: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 235: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 236: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 237: mul o4.z, r0.x, r2.x
    output.targets[4].z = ((r0.xxxx)*(r2.xxxx)).z;
    // 238: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 239: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 240: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 241: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 242: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 243: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 244: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 245: ret
    return output;
}

// source.character.static-map-native-1149.v1 / source program 9cc99f8aece490438300452a1aee87d8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1149(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1149(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[5];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[9]=g_SourceCharacterBaseConstants[10];
    source[9].w=(g_SourceCharacterTime.xxxx).x;
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 3: add r0.z, r1.w, l(-0.333300)
    r0.z = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 4: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 5: discard_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mul r0.z, r2.z, cb0[11].z
    r0.z = ((r2.zzzz)*(source[11].zzzz)).z;
    // 11: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 12: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 13: mul r0.w, r0.w, cb0[11].w
    r0.w = ((r0.wwww)*(source[11].wwww)).w;
    // 14: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 15: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 16: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: mul_sat r2.w, r0.z, cb2[3].w
    r2.w = (saturate((r0.zzzz)*(passValues[3].wwww))).w;
    // 18: mul r3.xyz, cb0[6].xyzx, cb0[10].wwww
    r3.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 19: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 20: mul r3.xyz, r1.xyzx, cb0[11].xxxx
    r3.xyz = ((r1.xyzx)*(source[11].xxxx)).xyz;
    // 21: mad r1.xyz, cb0[11].yyyy, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[11].yyyy)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 22: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 23: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 24: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 25: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 26: mad r3.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 27: mad r4.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 28: mul r0.z, r2.x, cb0[13].x
    r0.z = ((r2.xxxx)*(source[13].xxxx)).z;
    // 29: mul r0.w, r2.y, cb0[12].z
    r0.w = ((r2.yyyy)*(source[12].zzzz)).w;
    // 30: log r2.x, |r0.z|
    r2.x = (log2(abs(r0.zzzz))).x;
    // 31: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 32: mul r2.x, r2.x, cb0[13].y
    r2.x = ((r2.xxxx)*(source[13].yyyy)).x;
    // 33: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 34: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: movc r0.z, r0.z, l(0), r2.x
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).z;
    // 36: mad r3.xyz, r0.zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 37: mad r4.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 38: mad r3.xyz, r3.xyzx, r0.zzzz, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r0.zzzz)+(r4.xyzx)).xyz;
    // 39: mul r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = ((r0.zzzz)*(r3.xyzx)).xyz;
    // 40: max r3.xyz, r0.zzzz, r3.xyzx
    r3.xyz = (max(r0.zzzz,r3.xyzx)).xyz;
    // 41: dp2 r2.x, r0.xyxx, r0.xyxx
    r2.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 42: mul r0.xy, r0.xyxx, cb0[7].wwww
    r0.xy = ((r0.xyxx)*(source[7].wwww)).xy;
    // 43: mul r4.xy, r0.xyxx, v2.wwww
    r4.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 44: add r0.x, -r2.x, l(1.000000)
    r0.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 46: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 47: add r4.z, r0.x, l(0.000010)
    r4.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 49: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 50: div r4.xyz, r4.xyzx, r0.xxxx
    r4.xyz = ((r4.xyzx)/(r0.xxxx)).xyz;
    // 51: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 52: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 53: mul r5.xyz, r0.xxxx, r4.xyzx
    r5.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 54: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 55: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 56: mul r6.xyz, r0.xxxx, v6.xyzx
    r6.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 57: dp3 r0.x, r6.xyzx, r5.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 58: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 60: mul r7.xyz, r0.yyyy, cb0[25].xyzx
    r7.xyz = ((r0.yyyy)*(source[25].xyzx)).xyz;
    // 61: mad r7.xyz, r0.xxxx, cb0[24].xyzx, r7.xyzx
    r7.xyz = ((r0.xxxx)*(source[24].xyzx)+(r7.xyzx)).xyz;
    // 62: mul r7.xyz, r7.xyzx, cb0[26].wwww
    r7.xyz = ((r7.xyzx)*(source[26].wwww)).xyz;
    // 63: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 64: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 65: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 66: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 67: mul r7.xyz, r0.xxxx, v1.xyzx
    r7.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 68: dp3 r0.x, v0.xyzx, v0.xyzx
    r0.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 69: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 70: mul r8.xyz, r0.xxxx, v0.xyzx
    r8.xyz = ((r0.xxxx)*(v0.xyzx)).xyz;
    // 71: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 72: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 73: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 74: dp3 r10.y, r9.xyzx, r5.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 75: dp3 r10.x, r8.xyzx, r5.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 76: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 77: mul r0.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 78: dp2 r11.x, r10.xyxx, r0.xyxx
    r11.x = (dot((r10.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 79: dp3 r11.y, r7.xyzx, r5.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 80: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 81: dp4 r12.x, cb0[16].xyzw, r11.xyzw
    r12.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 82: dp4 r12.y, cb0[17].xyzw, r11.xyzw
    r12.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 83: dp4 r12.z, cb0[18].xyzw, r11.xyzw
    r12.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 84: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 85: dp4 r14.x, cb0[19].xyzw, r13.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 86: dp4 r14.y, cb0[20].xyzw, r13.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 87: dp4 r14.z, cb0[21].xyzw, r13.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 88: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 89: mul r2.x, r11.y, r11.y
    r2.x = ((r11.yyyy)*(r11.yyyy)).x;
    // 90: mov r10.z, r11.y
    r10.z = (r11.yyyy).z;
    // 91: mad r2.x, r11.x, r11.x, -r2.x
    r2.x = ((r11.xxxx)*(r11.xxxx)+(-(r2.xxxx))).x;
    // 92: mad r11.xyz, cb0[22].xyzx, r2.xxxx, r12.xyzx
    r11.xyz = ((source[22].xyzx)*(r2.xxxx)+(r12.xyzx)).xyz;
    // 93: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 94: mul r11.xyz, r11.xyzx, cb0[14].xyzx
    r11.xyz = ((r11.xyzx)*(source[14].xyzx)).xyz;
    // 95: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 96: log r2.x, |r0.w|
    r2.x = (log2(abs(r0.wwww))).x;
    // 97: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 98: mul r2.x, r2.x, cb0[12].w
    r2.x = ((r2.xxxx)*(source[12].wwww)).x;
    // 99: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 100: movc r0.w, r0.w, l(0), r2.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).w;
    // 101: max r0.w, r0.w, cb0[0].x
    r0.w = (max(r0.wwww,source[0].xxxx)).w;
    // 102: min r2.z, r0.w, l(1.000000)
    r2.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 103: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 104: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 105: mul r12.xyz, r0.wwww, v5.xyzx
    r12.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 106: dp3 r0.w, r5.xyzx, r12.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 107: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 108: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 109: deriv_rtx_coarse r2.x, r0.w
    r2.x = (ddx_coarse(r0.wwww)).x;
    // 110: deriv_rty_coarse r2.y, r0.w
    r2.y = (ddy_coarse(r0.wwww)).y;
    // 111: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 113: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 114: mad_sat r2.y, r2.x, l(0.300000), r2.z
    r2.y = (saturate((r2.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 115: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 116: add r2.z, -r2.y, l(1.000000)
    r2.z = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: mov_sat r1.w, cb0[12].x
    r1.w = (saturate(source[12].xxxx)).w;
    // 118: mad r13.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r13.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 119: mul r3.w, r1.w, l(0.080000)
    r3.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 120: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 121: mad r13.xyz, r2.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r2.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 122: max r14.xyz, r2.zzzz, r13.xyzx
    r14.xyz = (max(r2.zzzz,r13.xyzx)).xyz;
    // 123: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 124: mul_sat r1.w, r13.y, l(50.000000)
    r1.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 125: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 126: add r1.w, r5.z, l(1.000000)
    r1.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: add_sat r2.x, r0.w, -r1.w
    r2.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 129: sample_indexable(texture2d)(float,float,float,float) r15.xy, r2.xyxx, t3.xyzw, s4
    r15.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 130: add r0.w, r0.z, r2.x
    r0.w = ((r0.zzzz)+(r2.xxxx)).w;
    // 131: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 132: mul r16.xyz, r13.xyzx, r15.yyyy
    r16.xyz = ((r13.xyzx)*(r15.yyyy)).xyz;
    // 133: mad r14.xyz, r14.xyzx, r15.xxxx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xxxx)+(r16.xyzx)).xyz;
    // 134: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.y
    r1.w = r15.y != 0.f ? 1.f / r15.y : 0.f;
    // 135: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 136: mad r15.xyz, r13.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r13.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 137: dp3 r1.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 138: mad r13.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 139: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 140: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 141: mul r11.xyz, r11.xyzx, r16.xyzx
    r11.xyz = ((r11.xyzx)*(r16.xyzx)).xyz;
    // 142: mul r3.xyz, r3.xyzx, r11.xyzx
    r3.xyz = ((r3.xyzx)*(r11.xyzx)).xyz;
    // 143: mad r2.xzw, -r3.xxyz, r2.wwww, r3.xxyz
    r2.xzw = ((-(r3.xxyz))*(r2.wwww)+(r3.xxyz)).xzw;
    // 144: mul r1.w, r2.y, l(5.000000)
    r1.w = ((r2.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 145: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 146: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 147: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 148: add r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)+(r0.wwww)).w;
    // 149: mov o5.y, r0.z
    output.targets[5].y = (r0.zzzz).y;
    // 150: add_sat r0.z, r0.w, l(-1.000000)
    r0.z = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).z;
    // 151: dp3 r3.x, r8.xyzx, r5.xyzx
    r3.x = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 152: dp3 r3.y, r9.xyzx, r5.xyzx
    r3.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 153: dp2 r8.x, r3.xyxx, r0.xyxx
    r8.x = (dot((r3.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 154: dp2 r8.z, r3.xyxx, cb0[15].xyxx
    r8.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 155: dp3 r8.y, r7.xyzx, r5.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 156: dp3 r0.x, r6.xyzx, r5.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 157: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 158: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 159: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r8.xyzx, t4.xyzw, s3, r1.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r8.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 160: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 161: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 162: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 163: mad r0.w, r0.z, r13.x, r13.y
    r0.w = ((r0.zzzz)*(r13.xxxx)+(r13.yyyy)).w;
    // 164: mad r0.w, r0.w, r0.z, r13.z
    r0.w = ((r0.wwww)*(r0.zzzz)+(r13.zzzz)).w;
    // 165: mul r0.w, r0.z, r0.w
    r0.w = ((r0.zzzz)*(r0.wwww)).w;
    // 166: max r0.z, r0.w, r0.z
    r0.z = (max(r0.wwww,r0.zzzz)).z;
    // 167: mul r5.xyz, r0.yyyy, cb0[25].xyzx
    r5.xyz = ((r0.yyyy)*(source[25].xyzx)).xyz;
    // 168: mad r0.xyw, cb0[24].xyxz, r0.xxxx, r5.xyxz
    r0.xyw = ((source[24].xyxz)*(r0.xxxx)+(r5.xyxz)).xyw;
    // 169: mul r0.xyw, r0.xyxw, cb0[26].wwww
    r0.xyw = ((r0.xyxw)*(source[26].wwww)).xyw;
    // 170: mul r0.xyz, r0.zzzz, r0.xywx
    r0.xyz = ((r0.zzzz)*(r0.xywx)).xyz;
    // 171: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 172: mad r2.xyz, r0.xyzx, r14.xyzx, r2.xzwx
    r2.xyz = ((r0.xyzx)*(r14.xyzx)+(r2.xzwx)).xyz;
    // 173: mul r0.xyz, r14.xyzx, r0.xyzx
    r0.xyz = ((r14.xyzx)*(r0.xyzx)).xyz;
    // 174: dp3 o4.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 175: dp3 r0.x, r4.xyzx, r12.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 176: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 178: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 179: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 180: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 181: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 182: mul r0.yzw, r0.yyyy, cb0[3].xxyz
    r0.yzw = ((r0.yyyy)*(source[3].xxyz)).yzw;
    // 183: lt r1.w, |r0.x|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 184: movc r0.yzw, r1.wwww, l(0,0,0,0), r0.yyzw
    r0.yzw = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzw)).yzw;
    // 185: mad_sat r1.w, r0.x, cb0[8].z, -cb0[8].w
    r1.w = (saturate((r0.xxxx)*(source[8].zzzz)+(-(source[8].wwww)))).w;
    // 186: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 187: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 188: mul r2.w, r2.w, cb0[9].x
    r2.w = ((r2.wwww)*(source[9].xxxx)).w;
    // 189: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 190: mul r3.xyz, r2.wwww, cb0[4].xyzx
    r3.xyz = ((r2.wwww)*(source[4].xyzx)).xyz;
    // 191: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 192: add r3.xyz, r3.xyzx, -cb0[4].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[4].xyzx))).xyz;
    // 193: mad r3.xyz, cb0[4].wwww, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((source[4].wwww)*(r3.xyzx)+(source[4].xyzx)).xyz;
    // 194: mul r4.xyz, cb0[5].xyzx, cb0[9].zzzz
    r4.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 195: mul r4.xyz, r4.xyzx, cb0[10].yyyy
    r4.xyz = ((r4.xyzx)*(source[10].yyyy)).xyz;
    // 196: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 197: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 198: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 199: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 200: mul r4.xyz, r4.xyzx, cb0[10].zzzz
    r4.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 201: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 202: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 203: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 204: mad r0.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r0.yzwy
    r0.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r0.yzwy)).xyz;
    // 205: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 206: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 207: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 208: mad o0.xyz, r1.xyzx, cb0[26].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[26].xyzx)+(r0.xyzx)).xyz;
    // 209: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 210: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 211: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 212: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 213: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 214: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 215: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 216: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 217: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 218: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 219: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 220: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 221: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 222: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 223: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 224: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 225: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 226: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 227: ret
    return output;
}

// source.character.static-map-native-1150.v1 / source program b743d80328550249bd5f49dab27e51cd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1150(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[8];
    source[9]=g_SourceCharacterBaseConstants[9];
    source[14]=1.f;
    source[15]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 3: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 4: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((float4(0.0,0.0,0.0,0.0)).yxzw).y;
    // 5: min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // 6: mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // 7: mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 8: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 9: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 10: add r0.z, -cb0[9].y, l(1.000000)
    r0.z = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // 12: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 13: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 14: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 15: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 16: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 17: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 18: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 19: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 20: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 21: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 22: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 23: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 24: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 25: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 26: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 27: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 28: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 29: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 33: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 37: mul r5.xy, r4.xyxx, cb0[7].xxxx
    r5.xy = ((r4.xyxx)*(source[7].xxxx)).xy;
    // 38: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 39: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 40: mul r4.xyz, r1.wwww, r5.xyzx
    r4.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 41: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 42: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 43: mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 44: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: add r6.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r6.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 46: dp2 r7.x, cb0[2].xyxx, r6.xyxx
    r7.x = (dot((source[2].xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 47: dp2 r7.y, cb0[3].xyxx, r6.xyxx
    r7.y = (dot((source[3].xyxx).xy,(r6.xyxx).xy).xxxx).y;
    // 48: add r6.xy, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r7.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 49: mul r6.xy, r6.xyxx, cb0[4].xyxx
    r6.xy = ((r6.xyxx)*(source[4].xyxx)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t3.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 52: add r7.xyz, -r6.xyzx, r1.wwww
    r7.xyz = ((-(r6.xyzx))+(r1.wwww)).xyz;
    // 53: mad r6.xyz, cb0[8].yyyy, r7.xyzx, r6.xyzx
    r6.xyz = ((source[8].yyyy)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 54: mul r7.xyz, r6.xyzx, cb0[8].zzzz
    r7.xyz = ((r6.xyzx)*(source[8].zzzz)).xyz;
    // 55: mul r7.xyz, r7.xyzx, cb0[5].xyzx
    r7.xyz = ((r7.xyzx)*(source[5].xyzx)).xyz;
    // 56: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 57: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 58: mul r6.xyz, r6.xyzx, cb0[8].wwww
    r6.xyz = ((r6.xyzx)*(source[8].wwww)).xyz;
    // 59: mul r6.xyz, r6.xyzx, cb0[6].xyzx
    r6.xyz = ((r6.xyzx)*(source[6].xyzx)).xyz;
    // 60: mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 61: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 62: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 63: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 64: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 65: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 66: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 67: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 68: add r1.w, cb0[9].x, l(1.000000)
    r1.w = ((source[9].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 70: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 71: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 72: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 73: mul r8.xyz, r8.xyzx, cb0[14].xyzx
    r8.xyz = ((r8.xyzx)*(source[14].xyzx)).xyz;
    // 74: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 75: mul r9.xyz, r9.xyzx, cb0[15].xyzx
    r9.xyz = ((r9.xyzx)*(source[15].xyzx)).xyz;
    // 76: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 77: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 78: mul r10.xyz, r6.xyzx, r8.xyzx
    r10.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 79: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 80: mul r7.xyz, r2.wwww, r10.xyzx
    r7.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 81: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 82: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 83: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 84: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 85: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 86: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 87: mul r9.yzw, r9.yyyy, cb0[12].xxyz
    r9.yzw = ((r9.yyyy)*(source[12].xxyz)).yzw;
    // 88: mad r9.xyz, r9.xxxx, cb0[11].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[11].xyzx)+(r9.yzwy)).xyz;
    // 89: mul r9.xyz, r9.xyzx, cb0[13].wwww
    r9.xyz = ((r9.xyzx)*(source[13].wwww)).xyz;
    // 90: mul r11.xyz, r5.xyzx, r9.xyzx
    r11.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 91: mad r11.xyz, r5.xyzx, r3.xyzx, r11.xyzx
    r11.xyz = ((r5.xyzx)*(r3.xyzx)+(r11.xyzx)).xyz;
    // 92: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 93: mad r9.xyz, r10.xyzx, r2.wwww, r11.xyzx
    r9.xyz = ((r10.xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 94: add r10.xyz, r9.xyzx, cb0[1].xyzx
    r10.xyz = ((r9.xyzx)+(source[1].xyzx)).xyz;
    // 95: mad r10.xyz, r5.xyzx, cb0[13].xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(source[13].xyzx)+(r10.xyzx)).xyz;
    // 96: mad o0.xyz, r10.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r10.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 97: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 98: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 99: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 100: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 101: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 102: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 103: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 104: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 105: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 106: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 107: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 108: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 109: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 110: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 111: mul_sat o3.w, cb0[9].x, l(0.002000)
    output.targets[3].w = (saturate((source[9].xxxx)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 112: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 113: dp3 o4.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 114: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 115: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 116: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 117: mul o4.z, r0.y, r9.x
    output.targets[4].z = ((r0.yyyy)*(r9.xxxx)).z;
    // 118: ftou r0.y, cb0[10].z
    r0.y = (asfloat((uint4)(source[10].zzzz))).y;
    // 119: bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // 120: utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // 121: mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 122: mul_sat r0.yzw, r6.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r6.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // 123: sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // 124: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 125: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 126: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 127: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 128: ret
    return output;
}

// source.character.static-map-native-1150.v1 / source program fff4f25164212f47a9c7d39bbaf4148b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1150(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1150(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[8];
    source[9]=g_SourceCharacterBaseConstants[9];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 3: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 4: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t2.yxzw, s0, l(0.000000)
    r0.y = ((float4(0.0,0.0,0.0,0.0)).yxzw).y;
    // 5: min r0.y, r0.y, l(0.999000)
    r0.y = (min(r0.yyyy,float4(0.999000,0.999000,0.999000,0.999000))).y;
    // 6: mad r0.z, r0.y, cb2[1].x, cb2[1].y
    r0.z = ((r0.yyyy)*(passValues[1].xxxx)+(passValues[1].yyyy)).z;
    // 7: mad r0.y, r0.y, cb2[1].z, -cb2[1].w
    r0.y = ((r0.yyyy)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 8: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.yyyy)).y;
    // 9: add r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)+(r0.zzzz)).y;
    // 10: add r0.z, -cb0[9].y, l(1.000000)
    r0.z = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: add r0.y, r0.y, -v8.w
    r0.y = ((r0.yyyy)+(-(v8.wwww))).y;
    // 12: max r0.z, r0.z, l(0.001000)
    r0.z = (max(r0.zzzz,float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 13: div_sat r0.y, r0.y, r0.z
    r0.y = (saturate((r0.yyyy)/(r0.zzzz))).y;
    // 14: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 15: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 16: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 17: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 18: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 19: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 20: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 21: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 22: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 23: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 24: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 25: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 26: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 28: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 29: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 30: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 34: mul r4.xy, r3.xyxx, cb0[7].xxxx
    r4.xy = ((r3.xyxx)*(source[7].xxxx)).xy;
    // 35: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 36: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 37: mul r3.xyz, r1.wwww, r4.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 38: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 39: add r5.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r5.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 40: dp2 r6.x, cb0[2].xyxx, r5.xyxx
    r6.x = (dot((source[2].xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 41: dp2 r6.y, cb0[3].xyxx, r5.xyxx
    r6.y = (dot((source[3].xyxx).xy,(r5.xyxx).xy).xxxx).y;
    // 42: add r5.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 43: mul r5.xy, r5.xyxx, cb0[4].xyxx
    r5.xy = ((r5.xyxx)*(source[4].xyxx)).xy;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 45: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 47: mad r5.xyz, cb0[8].yyyy, r6.xyzx, r5.xyzx
    r5.xyz = ((source[8].yyyy)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 48: mul r6.xyz, r5.xyzx, cb0[8].zzzz
    r6.xyz = ((r5.xyzx)*(source[8].zzzz)).xyz;
    // 49: mul r6.xyz, r6.xyzx, cb0[5].xyzx
    r6.xyz = ((r6.xyzx)*(source[5].xyzx)).xyz;
    // 50: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 51: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 52: mul r5.xyz, r5.xyzx, cb0[8].wwww
    r5.xyz = ((r5.xyzx)*(source[8].wwww)).xyz;
    // 53: mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // 54: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 55: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 56: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 57: mul r6.xyz, r1.wwww, v7.xyzx
    r6.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 58: dp3 r1.w, r6.xyzx, r3.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 59: mad r6.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 60: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 61: mul r6.yzw, r6.yyyy, cb0[12].xxyz
    r6.yzw = ((r6.yyyy)*(source[12].xxyz)).yzw;
    // 62: mad r6.xyz, r6.xxxx, cb0[11].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[11].xyzx)+(r6.yzwy)).xyz;
    // 63: mul r6.xyz, r6.xyzx, cb0[13].wwww
    r6.xyz = ((r6.xyzx)*(source[13].wwww)).xyz;
    // 64: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 65: mad r6.xyz, r6.xyzx, r4.xyzx, cb0[1].xyzx
    r6.xyz = ((r6.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 66: mad r6.xyz, r4.xyzx, cb0[13].xyzx, r6.xyzx
    r6.xyz = ((r4.xyzx)*(source[13].xyzx)+(r6.xyzx)).xyz;
    // 67: mad o0.xyz, r6.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r6.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 68: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 69: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 70: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 71: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 72: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 73: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 74: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 75: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 76: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 77: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 78: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 79: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 80: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 81: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul_sat o3.w, cb0[9].x, l(0.002000)
    output.targets[3].w = (saturate((source[9].xxxx)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 83: dp3 o4.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 84: ftou r0.y, cb0[10].z
    r0.y = (asfloat((uint4)(source[10].zzzz))).y;
    // 85: bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // 86: utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // 87: mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 88: mul_sat r0.yzw, r5.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r5.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // 89: sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // 90: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 91: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 92: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 93: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 94: ret
    return output;
}

// source.character.static-map-native-1151.v1 / source program ebc86d3b6c59c7499d0e92bea0787826
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1151(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
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
    source[14]=g_SourceCharacterBaseConstants[15];
    source[15]=g_SourceCharacterBaseConstants[16];
    source[16]=g_SourceCharacterBaseConstants[17];
    source[17]=g_SourceCharacterBaseConstants[18];
    source[18]=g_SourceCharacterBaseConstants[19];
    source[19]=g_SourceCharacterBaseConstants[20];
    source[20]=g_SourceCharacterBaseConstants[21];
    source[24]=1.f;
    source[25]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: mul r1.xy, v4.xyxx, cb0[1].xyxx
    r1.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 5: mul r1.xy, r1.xyxx, cb0[17].xxxx
    r1.xy = ((r1.xyxx)*(source[17].xxxx)).xy;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t8.xyzw, s8, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: mul r1.zw, v4.xxxy, cb0[5].xxxy
    r1.zw = ((v4.xxxy)*(source[5].xxxy)).zw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r1.zwzz, t7.xyzw, s7, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 11: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 12: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 13: mul r2.xy, r1.zwzz, cb0[16].zzzz
    r2.xy = ((r1.zwzz)*(source[16].zzzz)).xy;
    // 14: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 15: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r1.zw, v4.xxxy, cb0[4].xxxy
    r1.zw = ((v4.xxxy)*(source[4].xxxy)).zw;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r1.zwzz, t6.xyzw, s6, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r1.zwzz, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mad r1.zw, r4.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r4.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 22: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 23: mul r4.xy, r1.zwzz, cb0[16].yyyy
    r4.xy = ((r1.zwzz)*(source[16].yyyy)).xy;
    // 24: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 27: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: mul r1.zw, v4.xxxy, cb0[3].xxxy
    r1.zw = ((v4.xxxy)*(source[3].xxxy)).zw;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r1.zwzz, t1.xyzw, s1, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r1.zwzz, t2.wxyz, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 31: mad r1.zw, r6.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r6.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 32: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 33: mul r6.xy, r1.zwzz, cb0[14].zzzz
    r6.xy = ((r1.zwzz)*(source[14].zzzz)).xy;
    // 34: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 37: add r6.z, r0.w, l(0.000010)
    r6.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: mul r1.zw, v4.xxxy, cb0[2].xxxy
    r1.zw = ((v4.xxxy)*(source[2].xxxy)).zw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r8.xy, r1.zwzz, t0.xyzw, s0, l(0.000000)
    r8.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r1.zwzz, t9.xyzw, s9, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture9.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: mad r1.zw, r8.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r8.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 42: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 43: mul r8.xy, r1.zwzz, cb0[14].xxxx
    r8.xy = ((r1.zwzz)*(source[14].xxxx)).xy;
    // 44: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: add r8.z, r0.w, l(0.000010)
    r8.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: add r6.xyz, r6.xyzx, -r8.xyzx
    r6.xyz = ((r6.xyzx)+(-(r8.xyzx))).xyz;
    // 49: add r10.xyz, r7.yzwy, -r9.xyzx
    r10.xyz = ((r7.yzwy)+(-(r9.xyzx))).xyz;
    // 50: mov r7.y, r5.w
    r7.y = (r5.wwww).y;
    // 51: mov r7.z, r3.w
    r7.z = (r3.wwww).z;
    // 52: mad_sat r11.xyz, r7.xyzx, v2.xyzx, v2.xyzx
    r11.xyz = (saturate((r7.xyzx)*(v2.xyzx)+(v2.xyzx))).xyz;
    // 53: add r12.xyz, r11.xyzx, -cb0[15].xxxx
    r12.xyz = ((r11.xyzx)+(-(source[15].xxxx))).xyz;
    // 54: mul_sat r12.xyz, r12.xyzx, cb0[16].xxxx
    r12.xyz = (saturate((r12.xyzx)*(source[16].xxxx))).xyz;
    // 55: add r11.xyz, r11.xyzx, -r12.xyzx
    r11.xyz = ((r11.xyzx)+(-(r12.xyzx))).xyz;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r13.xyz, v4.xyxx, t5.xywz, s5, l(0.000000)
    r13.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 57: mul r7.xyz, r7.xyzx, r13.zzzz
    r7.xyz = ((r7.xyzx)*(r13.zzzz)).xyz;
    // 58: mad r1.zw, r13.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r13.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 59: mul r13.xy, r1.zwzz, cb0[16].wwww
    r13.xy = ((r1.zwzz)*(source[16].wwww)).xy;
    // 60: mad r7.xyz, r7.xyzx, r11.xyzx, r12.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 61: mad r6.xyz, r7.xxxx, r6.xyzx, r8.xyzx
    r6.xyz = ((r7.xxxx)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 62: add r4.xyz, r4.xyzx, -r6.xyzx
    r4.xyz = ((r4.xyzx)+(-(r6.xyzx))).xyz;
    // 63: mad r4.xyz, r7.yyyy, r4.xyzx, r6.xyzx
    r4.xyz = ((r7.yyyy)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 64: add r2.xyz, r2.xyzx, -r4.xyzx
    r2.xyz = ((r2.xyzx)+(-(r4.xyzx))).xyz;
    // 65: mad r2.xyz, r7.zzzz, r2.xyzx, r4.xyzx
    r2.xyz = ((r7.zzzz)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 66: mov r13.z, l(0)
    r13.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 67: add r2.xyz, r2.xyzx, r13.xyzx
    r2.xyz = ((r2.xyzx)+(r13.xyzx)).xyz;
    // 68: mad r2.xy, cb0[17].yyyy, r1.xyxx, r2.xyxx
    r2.xy = ((source[17].yyyy)*(r1.xyxx)+(r2.xyxx)).xy;
    // 69: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 70: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 71: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 72: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 75: dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 76: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 78: mul r0.yzw, r0.yyyy, cb0[22].xxyz
    r0.yzw = ((r0.yyyy)*(source[22].xxyz)).yzw;
    // 79: mad r0.xyz, r0.xxxx, cb0[21].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[21].xyzx)+(r0.yzwy)).xyz;
    // 80: mul r0.xyz, r0.xyzx, cb0[23].wwww
    r0.xyz = ((r0.xyzx)*(source[23].wwww)).xyz;
    // 81: mad r2.xyz, r7.xxxx, r10.xyzx, r9.xyzx
    r2.xyz = ((r7.xxxx)*(r10.xyzx)+(r9.xyzx)).xyz;
    // 82: add r4.xyz, -r2.xyzx, r5.xyzx
    r4.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 83: mad r2.xyz, r7.yyyy, r4.xyzx, r2.xyzx
    r2.xyz = ((r7.yyyy)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 84: add r3.xyz, -r2.xyzx, r3.xyzx
    r3.xyz = ((-(r2.xyzx))+(r3.xyzx)).xyz;
    // 85: mad r2.xyz, r7.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r7.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 86: mul r3.xyz, cb0[6].xyzx, cb0[17].zzzz
    r3.xyz = ((source[6].xyzx)*(source[17].zzzz)).xyz;
    // 87: mad r4.xyz, cb0[17].wwww, cb0[7].xyzx, -r3.xyzx
    r4.xyz = ((source[17].wwww)*(source[7].xyzx)+(-(r3.xyzx))).xyz;
    // 88: mad r3.xyz, r7.xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 89: mad r4.xyz, cb0[18].xxxx, cb0[8].xyzx, -r3.xyzx
    r4.xyz = ((source[18].xxxx)*(source[8].xyzx)+(-(r3.xyzx))).xyz;
    // 90: mad r3.xyz, r7.yyyy, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 91: mad r4.xyz, cb0[18].yyyy, cb0[9].xyzx, -r3.xyzx
    r4.xyz = ((source[18].yyyy)*(source[9].xyzx)+(-(r3.xyzx))).xyz;
    // 92: mad r3.xyz, r7.zzzz, r4.xyzx, r3.xyzx
    r3.xyz = ((r7.zzzz)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 93: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 94: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 95: mul r4.xyz, r0.xyzx, r3.xyzx
    r4.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 96: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 97: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 98: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 99: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 100: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t11.xyzw, s10
    r6.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 101: mul r6.xyz, r6.xyzx, cb0[25].xyzx
    r6.xyz = ((r6.xyzx)*(source[25].xyzx)).xyz;
    // 102: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 103: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t10.xyzw, s10
    r5.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 104: mul r5.xyz, r5.xyzx, cb0[24].xyzx
    r5.xyz = ((r5.xyzx)*(source[24].xyzx)).xyz;
    // 105: mul r8.xyz, r0.wwww, r5.xyzx
    r8.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 106: mad r0.xyz, r5.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 107: add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 108: div r0.xyz, r8.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)/(r0.xyzx)).xyz;
    // 109: mad r4.xyz, r3.xyzx, r8.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 110: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 112: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 113: mul r0.yzw, r0.yyyy, v5.xxyz
    r0.yzw = ((r0.yyyy)*(v5.xxyz)).yzw;
    // 114: dp3 r1.w, r1.xyzx, r0.yzwy
    r1.w = (dot((r1.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 115: mul r8.xyz, r1.wwww, r1.xyzx
    r8.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 116: mad r0.yzw, r8.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), -r0.yyzw
    r0.yzw = ((r8.xxyz)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r0.yyzw))).yzw;
    // 117: dp2_sat r8.x, r0.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r0.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 118: dp3_sat r8.y, r0.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r0.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 119: dp3_sat r8.z, r0.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r0.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 120: log r0.yzw, r8.xxyz
    r0.yzw = (log2(r8.xxyz)).yzw;
    // 121: add r1.w, -cb0[19].w, cb0[19].z
    r1.w = ((-(source[19].wwww))+(source[19].zzzz)).w;
    // 122: mad r1.w, r7.x, r1.w, cb0[19].w
    r1.w = ((r7.xxxx)*(r1.wwww)+(source[19].wwww)).w;
    // 123: add r2.w, -r1.w, cb0[20].x
    r2.w = ((-(r1.wwww))+(source[20].xxxx)).w;
    // 124: mad r1.w, r7.y, r2.w, r1.w
    r1.w = ((r7.yyyy)*(r2.wwww)+(r1.wwww)).w;
    // 125: add r2.w, -r1.w, cb0[20].y
    r2.w = ((-(r1.wwww))+(source[20].yyyy)).w;
    // 126: mad r1.w, r7.z, r2.w, r1.w
    r1.w = ((r7.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 127: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 128: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 129: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 130: dp3 r0.y, r6.xyzx, r0.yzwy
    r0.y = (dot((r6.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 131: add r6.xyz, -cb0[10].xyzx, cb0[11].xyzx
    r6.xyz = ((-(source[10].xyzx))+(source[11].xyzx)).xyz;
    // 132: mad r6.xyz, r7.xxxx, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r7.xxxx)*(r6.xyzx)+(source[10].xyzx)).xyz;
    // 133: add r8.xyz, -r6.xyzx, cb0[12].xyzx
    r8.xyz = ((-(r6.xyzx))+(source[12].xyzx)).xyz;
    // 134: mad r6.xyz, r7.yyyy, r8.xyzx, r6.xyzx
    r6.xyz = ((r7.yyyy)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 135: add r8.xyz, -r6.xyzx, cb0[13].xyzx
    r8.xyz = ((-(r6.xyzx))+(source[13].xyzx)).xyz;
    // 136: mad r6.xyz, r7.zzzz, r8.xyzx, r6.xyzx
    r6.xyz = ((r7.zzzz)*(r8.xyzx)+(r6.xyzx)).xyz;
    // 137: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 138: add r0.z, -cb0[18].w, cb0[18].z
    r0.z = ((-(source[18].wwww))+(source[18].zzzz)).z;
    // 139: mad r0.z, r7.x, r0.z, cb0[18].w
    r0.z = ((r7.xxxx)*(r0.zzzz)+(source[18].wwww)).z;
    // 140: add r0.w, -r0.z, cb0[19].x
    r0.w = ((-(r0.zzzz))+(source[19].xxxx)).w;
    // 141: mad r0.z, r7.y, r0.w, r0.z
    r0.z = ((r7.yyyy)*(r0.wwww)+(r0.zzzz)).z;
    // 142: add r0.w, -r0.z, cb0[19].y
    r0.w = ((-(r0.zzzz))+(source[19].yyyy)).w;
    // 143: mad r0.z, r7.z, r0.w, r0.z
    r0.z = ((r7.zzzz)*(r0.wwww)+(r0.zzzz)).z;
    // 144: mul r2.xyz, r2.xyzx, r0.zzzz
    r2.xyz = ((r2.xyzx)*(r0.zzzz)).xyz;
    // 145: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 146: mul r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r2.xyzx)).xyz;
    // 147: mad r4.xyz, r2.xyzx, r0.yyyy, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r0.yyyy)+(r4.xyzx)).xyz;
    // 148: mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // 149: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 150: add r0.yzw, r4.xxyz, cb0[0].xxyz
    r0.yzw = ((r4.xxyz)+(source[0].xxyz)).yzw;
    // 151: mad o0.xyz, r3.xyzx, cb0[23].xyzx, r0.yzwy
    output.targets[0].xyz = ((r3.xyzx)*(source[23].xyzx)+(r0.yzwy)).xyz;
    // 152: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 153: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 154: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 155: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 156: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 157: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 158: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 159: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 160: mul r3.xyz, r0.wyzw, r2.yzxy
    r3.xyz = ((r0.wyzw)*(r2.yzxy)).xyz;
    // 161: mad r3.xyz, r0.zwyz, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.zwyz)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 162: dp3 r5.z, r0.yzwy, r1.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r1.xyzx).xyz).xxxx).z;
    // 163: dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 164: mul r0.yzw, r3.xxyz, v1.wwww
    r0.yzw = ((r3.xxyz)*(v1.wwww)).yzw;
    // 165: dp3 r5.y, r0.yzwy, r1.xyzx
    r5.y = (dot((r0.yzwy).xyz,(r1.xyzx).xyz).xxxx).y;
    // 166: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 167: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 168: mul r0.yzw, r0.yyyy, r5.xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)).yzw;
    // 169: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 170: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 171: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 172: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 173: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 174: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 175: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 176: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 177: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 178: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 179: mul o4.z, r0.x, r4.x
    output.targets[4].z = ((r0.xxxx)*(r4.xxxx)).z;
    // 180: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 181: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 182: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 183: ret
    return output;
}

// source.character.static-map-native-1151.v1 / source program 361cf60a7dc21740a89908df0543c57a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1151(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1151(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=g_SourceCharacterBaseConstants[8];
    source[9]=g_SourceCharacterBaseConstants[9];
    source[10]=g_SourceCharacterBaseConstants[15];
    source[11]=g_SourceCharacterBaseConstants[16];
    source[12]=g_SourceCharacterBaseConstants[17];
    source[13]=g_SourceCharacterBaseConstants[18];
    source[14]=g_SourceCharacterBaseConstants[19];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[4].xyxx
    r0.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t6.zwxy, s6, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 6: mul r2.xy, r0.xyxx, cb0[12].yyyy
    r2.xy = ((r0.xyxx)*(source[12].yyyy)).xy;
    // 7: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: mul r0.xy, v4.xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r0.xyxx, t2.wxyz, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).xyzw;
    // 14: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 16: mul r4.xy, r0.xyxx, cb0[10].zzzz
    r4.xy = ((r0.xyxx)*(source[10].zzzz)).xy;
    // 17: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 19: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 20: add r4.z, r0.x, l(0.000010)
    r4.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 21: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.xyxx, t0.zwxy, s0, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.xyxx, t9.xyzw, s9, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture9.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 24: mad r0.xy, r0.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 26: mul r6.xy, r0.xyxx, cb0[10].xxxx
    r6.xy = ((r0.xyxx)*(source[10].xxxx)).xy;
    // 27: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 29: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 30: add r6.z, r0.x, l(0.000010)
    r6.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 31: add r0.xyz, r4.xyzx, -r6.xyzx
    r0.xyz = ((r4.xyzx)+(-(r6.xyzx))).xyz;
    // 32: add r4.xyz, r3.yzwy, -r5.xyzx
    r4.xyz = ((r3.yzwy)+(-(r5.xyzx))).xyz;
    // 33: mov r3.y, r1.w
    r3.y = (r1.wwww).y;
    // 34: mul r7.xy, v4.xyxx, cb0[5].xyxx
    r7.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r7.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t7.xyzw, s7, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 37: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 38: mov r3.z, r8.w
    r3.z = (r8.wwww).z;
    // 39: mad_sat r9.xyz, r3.xyzx, v2.xyzx, v2.xyzx
    r9.xyz = (saturate((r3.xyzx)*(v2.xyzx)+(v2.xyzx))).xyz;
    // 40: add r10.xyz, r9.xyzx, -cb0[11].xxxx
    r10.xyz = ((r9.xyzx)+(-(source[11].xxxx))).xyz;
    // 41: mul_sat r10.xyz, r10.xyzx, cb0[12].xxxx
    r10.xyz = (saturate((r10.xyzx)*(source[12].xxxx))).xyz;
    // 42: add r9.xyz, r9.xyzx, -r10.xyzx
    r9.xyz = ((r9.xyzx)+(-(r10.xyzx))).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r11.xyz, v4.xyxx, t5.xywz, s5, l(0.000000)
    r11.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 44: mul r3.xyz, r3.xyzx, r11.zzzz
    r3.xyz = ((r3.xyzx)*(r11.zzzz)).xyz;
    // 45: mad r7.zw, r11.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r7.zw = ((r11.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 46: mul r11.xy, r7.zwzz, cb0[12].wwww
    r11.xy = ((r7.zwzz)*(source[12].wwww)).xy;
    // 47: mad r3.xyz, r3.xyzx, r9.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 48: mad r0.xyz, r3.xxxx, r0.xyzx, r6.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 49: add r2.xyz, -r0.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xyzx))+(r2.xyzx)).xyz;
    // 50: mad r0.xyz, r3.yyyy, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.yyyy)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 51: dp2 r0.w, r7.xyxx, r7.xyxx
    r0.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 52: mul r2.xy, r7.xyxx, cb0[12].zzzz
    r2.xy = ((r7.xyxx)*(source[12].zzzz)).xy;
    // 53: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 55: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 56: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 57: add r2.xyz, -r0.xyzx, r2.xyzx
    r2.xyz = ((-(r0.xyzx))+(r2.xyzx)).xyz;
    // 58: mad r0.xyz, r3.zzzz, r2.xyzx, r0.xyzx
    r0.xyz = ((r3.zzzz)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 59: mov r11.z, l(0)
    r11.z = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).z;
    // 60: add r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)+(r11.xyzx)).xyz;
    // 61: mul r2.xy, v4.xyxx, cb0[1].xyxx
    r2.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 62: mul r2.xy, r2.xyxx, cb0[13].xxxx
    r2.xy = ((r2.xyxx)*(source[13].xxxx)).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t8.xyzw, s8, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 64: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 65: mad r0.xy, cb0[13].yyyy, r2.xyxx, r0.xyxx
    r0.xy = ((source[13].yyyy)*(r2.xyxx)+(r0.xyxx)).xy;
    // 66: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 67: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 68: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 69: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 71: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 72: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 73: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 74: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 75: dp3 r0.w, r2.xyzx, r0.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 76: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 78: mul r2.yzw, r2.yyyy, cb0[16].xxyz
    r2.yzw = ((r2.yyyy)*(source[16].xxyz)).yzw;
    // 79: mad r2.xyz, r2.xxxx, cb0[15].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[15].xyzx)+(r2.yzwy)).xyz;
    // 80: mul r2.xyz, r2.xyzx, cb0[17].wwww
    r2.xyz = ((r2.xyzx)*(source[17].wwww)).xyz;
    // 81: mad r4.xyz, r3.xxxx, r4.xyzx, r5.xyzx
    r4.xyz = ((r3.xxxx)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 82: add r1.xyz, r1.xyzx, -r4.xyzx
    r1.xyz = ((r1.xyzx)+(-(r4.xyzx))).xyz;
    // 83: mad r1.xyz, r3.yyyy, r1.xyzx, r4.xyzx
    r1.xyz = ((r3.yyyy)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 84: add r4.xyz, -r1.xyzx, r8.xyzx
    r4.xyz = ((-(r1.xyzx))+(r8.xyzx)).xyz;
    // 85: mad r1.xyz, r3.zzzz, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.zzzz)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 86: mul r4.xyz, cb0[6].xyzx, cb0[13].zzzz
    r4.xyz = ((source[6].xyzx)*(source[13].zzzz)).xyz;
    // 87: mad r5.xyz, cb0[13].wwww, cb0[7].xyzx, -r4.xyzx
    r5.xyz = ((source[13].wwww)*(source[7].xyzx)+(-(r4.xyzx))).xyz;
    // 88: mad r4.xyz, r3.xxxx, r5.xyzx, r4.xyzx
    r4.xyz = ((r3.xxxx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 89: mad r5.xyz, cb0[14].xxxx, cb0[8].xyzx, -r4.xyzx
    r5.xyz = ((source[14].xxxx)*(source[8].xyzx)+(-(r4.xyzx))).xyz;
    // 90: mad r3.xyw, r3.yyyy, r5.xyxz, r4.xyxz
    r3.xyw = ((r3.yyyy)*(r5.xyxz)+(r4.xyxz)).xyw;
    // 91: mad r4.xyz, cb0[14].yyyy, cb0[9].xyzx, -r3.xywx
    r4.xyz = ((source[14].yyyy)*(source[9].xyzx)+(-(r3.xywx))).xyz;
    // 92: mad r3.xyz, r3.zzzz, r4.xyzx, r3.xywx
    r3.xyz = ((r3.zzzz)*(r4.xyzx)+(r3.xywx)).xyz;
    // 93: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 94: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 95: mad r3.xyz, r2.xyzx, r1.xyzx, cb0[0].xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(source[0].xyzx)).xyz;
    // 96: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 97: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 98: mad o0.xyz, r1.xyzx, cb0[17].xyzx, r3.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[17].xyzx)+(r3.xyzx)).xyz;
    // 99: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 100: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 101: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 102: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 103: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 104: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 105: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 106: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 107: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 108: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 109: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 110: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 111: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 112: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 113: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 114: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 115: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 116: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 117: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 118: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 119: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 120: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 121: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 122: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 123: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 124: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 125: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 126: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 127: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 128: ret
    return output;
}

// source.character.static-map-native-1152.v1 / source program b02f165d758ea14eb5e5be4b3b872fe4
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 1136u: return SourceCharacterBase1136(input);
    case 1137u: return SourceCharacterBase1137(input);
    case 1138u: return SourceCharacterBase1138(input);
    case 1139u: return SourceCharacterBase1139(input);
    case 1140u: return SourceCharacterBase1140(input);
    case 1141u: return SourceCharacterBase1141(input);
    case 1142u: return SourceCharacterBase1142(input);
    case 1143u: return SourceCharacterBase1143(input);
    case 1144u: return SourceCharacterBase1144(input);
    case 1145u: return SourceCharacterBase1145(input);
    case 1146u: return SourceCharacterBase1146(input);
    case 1147u: return SourceCharacterBase1147(input);
    case 1148u: return SourceCharacterBase1148(input);
    case 1149u: return SourceCharacterBase1149(input);
    case 1150u: return SourceCharacterBase1150(input);
    case 1151u: return SourceCharacterBase1151(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
