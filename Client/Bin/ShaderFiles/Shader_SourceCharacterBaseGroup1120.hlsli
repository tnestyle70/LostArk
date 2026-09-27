#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1120(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[5].yyww
    r2.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[5].zzzz
    r0.zw = ((r0.zzzw)*(source[5].zzzz)).zw;
    // 12: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: mul r4.xyz, cb0[4].xyzx, cb0[6].wwww
    r4.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 38: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 39: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 41: mad r3.xyz, cb0[7].yyyy, r3.xyzx, r5.xyzx
    r3.xyz = ((source[7].yyyy)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 42: mul r4.xyz, cb0[3].xyzx, cb0[6].zzzz
    r4.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 43: mad r3.xyz, -r1.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 44: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 45: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 46: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 47: mul r3.xyz, r1.xyzx, cb0[7].zzzz
    r3.xyz = ((r1.xyzx)*(source[7].zzzz)).xyz;
    // 48: mad r1.xyz, cb0[7].wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[7].wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: mul r1.w, r4.z, cb0[8].x
    r1.w = ((r4.zzzz)*(source[8].xxxx)).w;
    // 51: mul r2.zw, r4.yyyx, cb0[9].xxxz
    r2.zw = ((r4.yyyx)*(source[9].xxxz)).zw;
    // 52: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 53: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: mul r3.w, r3.w, cb0[8].y
    r3.w = ((r3.wwww)*(source[8].yyyy)).w;
    // 55: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 56: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 57: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 59: mad r1.xyz, r3.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 60: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 62: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 63: dp2 r3.x, r2.xyxx, r2.xyxx
    r3.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 64: mul r5.xy, r2.xyxx, cb0[6].xxxx
    r5.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 65: add r2.x, -r3.x, l(1.000000)
    r2.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 67: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 68: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 69: add r3.xyz, -r0.xyzx, r5.xyzx
    r3.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 70: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 71: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 73: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 74: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 75: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 76: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 77: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 78: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 79: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 80: mul r5.xyz, r2.yyyy, cb0[21].xyzx
    r5.xyz = ((r2.yyyy)*(source[21].xyzx)).xyz;
    // 81: mad r5.xyz, r2.xxxx, cb0[20].xyzx, r5.xyzx
    r5.xyz = ((r2.xxxx)*(source[20].xyzx)+(r5.xyzx)).xyz;
    // 82: mul r5.xyz, r5.xyzx, cb0[22].wwww
    r5.xyz = ((r5.xyzx)*(source[22].wwww)).xyz;
    // 83: mul r6.xyz, r1.xyzx, r5.xyzx
    r6.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 84: dp2_sat r7.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 85: dp3_sat r7.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 86: dp3_sat r7.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 87: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 88: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t9.xyzw, s6
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 89: mul r8.xyz, r8.xyzx, cb0[24].xyzx
    r8.xyz = ((r8.xyzx)*(source[24].xyzx)).xyz;
    // 90: dp3 r0.w, r8.xyzx, r7.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 91: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s6
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 92: mul r7.xyz, r7.xyzx, cb0[23].xyzx
    r7.xyz = ((r7.xyzx)*(source[23].xyzx)).xyz;
    // 93: mul r9.xyz, r0.wwww, r7.xyzx
    r9.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 94: mad r6.xyz, r1.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 95: mad r9.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 96: mad r10.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 97: mad r11.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 98: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 99: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 100: mul r2.xy, r2.xyxx, cb0[9].ywyy
    r2.xy = ((r2.xyxx)*(source[9].ywyy)).xy;
    // 101: exp r2.xy, r2.xyxx
    r2.xy = (exp2(r2.xyxx)).xy;
    // 102: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 103: movc r2.xy, r2.zwzz, l(0,0,0,0), r2.xyxx
    r2.xy = ((asuint(r2.zwzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyxx)).xy;
    // 104: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 105: min r4.z, r2.x, l(1.000000)
    r4.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mad r2.xzw, r2.yyyy, r10.xxyz, r11.xxyz
    r2.xzw = ((r2.yyyy)*(r10.xxyz)+(r11.xxyz)).xzw;
    // 107: mad r2.xzw, r2.xxzw, r2.yyyy, r9.xxyz
    r2.xzw = ((r2.xxzw)*(r2.yyyy)+(r9.xxyz)).xzw;
    // 108: mul r2.xzw, r2.yyyy, r2.xxzw
    r2.xzw = ((r2.yyyy)*(r2.xxzw)).xzw;
    // 109: max r2.xzw, r2.xxzw, r2.yyyy
    r2.xzw = (max(r2.xxzw,r2.yyyy)).xzw;
    // 110: mul r2.xzw, r2.xxzw, r6.xxyz
    r2.xzw = ((r2.xxzw)*(r6.xxyz)).xzw;
    // 111: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 112: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 113: mul r6.xyz, r3.wwww, v1.xyzx
    r6.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 114: dp3 r9.y, r6.xyzx, r3.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 115: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 116: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 117: mul r10.xyz, r3.wwww, v0.xyzx
    r10.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 118: mul r11.xyz, r6.zxyz, r10.yzxy
    r11.xyz = ((r6.zxyz)*(r10.yzxy)).xyz;
    // 119: mad r11.xyz, r6.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r6.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 120: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 121: dp3 r12.y, r11.xyzx, r3.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 122: dp3 r12.x, r10.xyzx, r3.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 123: dp2 r9.z, r12.xyxx, cb0[11].xyxx
    r9.z = (dot((r12.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 124: mul r4.xy, cb0[11].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[11].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 125: dp2 r9.x, r12.xyxx, r4.xyxx
    r9.x = (dot((r12.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 126: mov r9.w, l(1.000000)
    r9.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 127: dp4 r13.x, cb0[12].xyzw, r9.xyzw
    r13.x = (dot((source[12].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 128: dp4 r13.y, cb0[13].xyzw, r9.xyzw
    r13.y = (dot((source[13].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 129: dp4 r13.z, cb0[14].xyzw, r9.xyzw
    r13.z = (dot((source[14].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 130: mul r14.xyzw, r9.yzzx, r9.xyzz
    r14.xyzw = ((r9.yzzx)*(r9.xyzz)).xyzw;
    // 131: dp4 r15.x, cb0[15].xyzw, r14.xyzw
    r15.x = (dot((source[15].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 132: dp4 r15.y, cb0[16].xyzw, r14.xyzw
    r15.y = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 133: dp4 r15.z, cb0[17].xyzw, r14.xyzw
    r15.z = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 134: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 135: mul r3.w, r9.y, r9.y
    r3.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 136: mov r12.z, r9.y
    r12.z = (r9.yyyy).z;
    // 137: mad r3.w, r9.x, r9.x, -r3.w
    r3.w = ((r9.xxxx)*(r9.xxxx)+(-(r3.wwww))).w;
    // 138: mad r9.xyz, cb0[18].xyzx, r3.wwww, r13.xyzx
    r9.xyz = ((source[18].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 139: max r9.xyz, r9.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 140: mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // 141: mad r9.xyz, r9.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r9.xyz = ((r9.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 142: mov_sat r1.w, cb0[8].z
    r1.w = (saturate(source[8].zzzz)).w;
    // 143: mad r13.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r13.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 144: mul r3.w, r1.w, l(0.080000)
    r3.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 145: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 146: mad r13.xyz, r4.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r4.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 147: mul_sat r1.w, r13.y, l(50.000000)
    r1.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 148: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 149: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 150: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 151: dp3 r3.w, r3.xyzx, r14.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 152: mul r3.xyz, r3.wwww, r3.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)).xyz;
    // 153: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 154: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 155: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 156: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 158: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 159: mad_sat r15.y, r5.w, l(0.300000), r4.z
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 160: add r5.w, -r15.y, l(1.000000)
    r5.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: max r16.xyz, r13.xyzx, r5.wwww
    r16.xyz = (max(r13.xyzx,r5.wwww)).xyz;
    // 162: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 163: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 164: add r1.w, r3.z, l(1.000000)
    r1.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add_sat r15.x, -r1.w, r3.w
    r15.x = (saturate((-(r1.wwww))+(r3.wwww))).x;
    // 167: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 168: add r1.w, r2.y, r15.x
    r1.w = ((r2.yyyy)+(r15.xxxx)).w;
    // 169: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 170: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 171: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 172: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 173: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 174: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 175: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 176: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 177: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 179: mul r9.xyz, r9.xyzx, r17.xyzx
    r9.xyz = ((r9.xyzx)*(r17.xyzx)).xyz;
    // 180: mul r2.xzw, r2.xxzw, r9.xxyz
    r2.xzw = ((r2.xxzw)*(r9.xxyz)).xzw;
    // 181: mad r2.xzw, -r2.xxzw, r4.wwww, r2.xxzw
    r2.xzw = ((-(r2.xxzw))*(r4.wwww)+(r2.xxzw)).xzw;
    // 182: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 183: dp2_sat r9.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 184: dp3_sat r9.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 185: dp3_sat r9.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 186: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 187: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 188: add r0.w, r0.w, -r3.w
    r0.w = ((r0.wwww)+(-(r3.wwww))).w;
    // 189: mad r0.w, r4.z, r0.w, r3.w
    r0.w = ((r4.zzzz)*(r0.wwww)+(r3.wwww)).w;
    // 190: mad r5.xyz, r7.xyzx, r0.wwww, r5.xyzx
    r5.xyz = ((r7.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 191: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 192: mul r0.w, r15.y, r15.y
    r0.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 193: mul r3.w, r15.y, l(5.000000)
    r3.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 194: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 195: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 196: add r0.w, r2.y, r0.w
    r0.w = ((r2.yyyy)+(r0.wwww)).w;
    // 197: mov o5.y, r2.y
    output.targets[5].y = (r2.yyyy).y;
    // 198: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 199: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 200: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 201: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 202: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 203: mul r8.xyz, r0.wwww, r5.xyzx
    r8.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 204: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 205: div r5.xyz, r7.xyzx, r5.xyzx
    r5.xyz = ((r7.xyzx)/(r5.xyzx)).xyz;
    // 206: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 207: dp3 r5.x, r10.xyzx, r3.xyzx
    r5.x = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 208: dp3 r5.y, r11.xyzx, r3.xyzx
    r5.y = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 209: dp3 r3.y, r6.xyzx, r3.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 210: dp2 r3.x, r5.xyxx, r4.xyxx
    r3.x = (dot((r5.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 211: dp2 r3.z, r5.xyxx, cb0[11].xyxx
    r3.z = (dot((r5.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 212: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t7.xyzw, s7, r3.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 213: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 214: mul r3.xyz, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((r3.xyzx)*(source[10].xyzx)).xyz;
    // 215: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 216: mul r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)).xyz;
    // 217: mad r2.xyz, r3.xyzx, r15.xzwx, r2.xzwx
    r2.xyz = ((r3.xyzx)*(r15.xzwx)+(r2.xzwx)).xyz;
    // 218: mul r3.xyz, r15.xzwx, r3.xyzx
    r3.xyz = ((r15.xzwx)*(r3.xyzx)).xyz;
    // 219: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 220: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 221: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 222: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 223: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 224: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 225: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 226: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 227: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 228: mul r3.xyz, r0.xxxx, cb0[2].xyzx
    r3.xyz = ((r0.xxxx)*(source[2].xyzx)).xyz;
    // 229: movc r0.xyz, r0.yyyy, l(0,0,0,0), r3.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 230: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 231: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 232: mad o0.xyz, r1.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 233: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 234: dp3 r0.x, r12.xyzx, r12.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 235: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 236: mul r0.xyz, r0.xxxx, r12.xyzx
    r0.xyz = ((r0.xxxx)*(r12.xyzx)).xyz;
    // 237: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 238: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 239: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 240: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 241: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 242: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 243: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 244: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 245: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 246: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 247: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 248: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 249: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 250: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 251: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 252: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 253: ret
    return output;
}

// source.character.static-map-native-1120.v1 / source program 516ebb147160fd47a78869b404e02241
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1120(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1120(input);
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[5].yyww
    r2.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[5].zzzz
    r0.zw = ((r0.zzzw)*(source[5].zzzz)).zw;
    // 12: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: mul r4.xyz, cb0[4].xyzx, cb0[6].wwww
    r4.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 38: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 39: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 41: mad r3.xyz, cb0[7].yyyy, r3.xyzx, r5.xyzx
    r3.xyz = ((source[7].yyyy)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 42: mul r4.xyz, cb0[3].xyzx, cb0[6].zzzz
    r4.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 43: mad r3.xyz, -r1.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 44: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 45: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 46: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 47: mul r3.xyz, r1.xyzx, cb0[7].zzzz
    r3.xyz = ((r1.xyzx)*(source[7].zzzz)).xyz;
    // 48: mad r1.xyz, cb0[7].wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[7].wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: mul r1.w, r4.z, cb0[8].x
    r1.w = ((r4.zzzz)*(source[8].xxxx)).w;
    // 51: mul r2.zw, r4.yyyx, cb0[9].xxxz
    r2.zw = ((r4.yyyx)*(source[9].xxxz)).zw;
    // 52: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 53: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 54: mul r3.w, r3.w, cb0[8].y
    r3.w = ((r3.wwww)*(source[8].yyyy)).w;
    // 55: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 56: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 57: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 59: mad r1.xyz, r3.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 60: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 62: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 63: dp2 r3.x, r2.xyxx, r2.xyxx
    r3.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 64: mul r5.xy, r2.xyxx, cb0[6].xxxx
    r5.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 65: add r2.x, -r3.x, l(1.000000)
    r2.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 67: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 68: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 69: add r3.xyz, -r0.xyzx, r5.xyzx
    r3.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 70: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 71: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 72: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 73: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 74: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 75: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 76: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 77: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 78: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 79: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 80: mul r6.xyz, r2.yyyy, cb0[21].xyzx
    r6.xyz = ((r2.yyyy)*(source[21].xyzx)).xyz;
    // 81: mad r6.xyz, r2.xxxx, cb0[20].xyzx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(source[20].xyzx)+(r6.xyzx)).xyz;
    // 82: mul r6.xyz, r6.xyzx, cb0[22].wwww
    r6.xyz = ((r6.xyzx)*(source[22].wwww)).xyz;
    // 83: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 84: mad r7.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r7.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 85: mad r8.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 86: mad r9.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 87: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 88: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 89: mul r2.xy, r2.xyxx, cb0[9].ywyy
    r2.xy = ((r2.xyxx)*(source[9].ywyy)).xy;
    // 90: exp r2.xy, r2.xyxx
    r2.xy = (exp2(r2.xyxx)).xy;
    // 91: min r0.w, r2.y, l(1.000000)
    r0.w = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 92: movc r2.x, r2.z, l(0), r2.x
    r2.x = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 93: movc r0.w, r2.w, l(0), r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 94: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 95: min r4.z, r2.x, l(1.000000)
    r4.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: mad r2.xyz, r0.wwww, r8.xyzx, r9.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 97: mad r2.xyz, r2.xyzx, r0.wwww, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 98: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 99: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 100: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 101: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 102: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 103: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 104: dp3 r7.y, r6.xyzx, r3.xyzx
    r7.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 105: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 106: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 107: mul r8.xyz, r2.wwww, v0.xyzx
    r8.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 108: mul r9.xyz, r6.zxyz, r8.yzxy
    r9.xyz = ((r6.zxyz)*(r8.yzxy)).xyz;
    // 109: mad r9.xyz, r6.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r6.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 110: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 111: dp3 r10.y, r9.xyzx, r3.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 112: dp3 r10.x, r8.xyzx, r3.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 113: dp2 r7.z, r10.xyxx, cb0[11].xyxx
    r7.z = (dot((r10.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 114: mul r4.xy, cb0[11].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[11].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 115: dp2 r7.x, r10.xyxx, r4.xyxx
    r7.x = (dot((r10.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 116: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 117: dp4 r11.x, cb0[12].xyzw, r7.xyzw
    r11.x = (dot((source[12].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 118: dp4 r11.y, cb0[13].xyzw, r7.xyzw
    r11.y = (dot((source[13].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 119: dp4 r11.z, cb0[14].xyzw, r7.xyzw
    r11.z = (dot((source[14].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 120: mul r12.xyzw, r7.yzzx, r7.xyzz
    r12.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 121: dp4 r13.x, cb0[15].xyzw, r12.xyzw
    r13.x = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 122: dp4 r13.y, cb0[16].xyzw, r12.xyzw
    r13.y = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 123: dp4 r13.z, cb0[17].xyzw, r12.xyzw
    r13.z = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 124: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 125: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 126: mov r10.z, r7.y
    r10.z = (r7.yyyy).z;
    // 127: mad r2.w, r7.x, r7.x, -r2.w
    r2.w = ((r7.xxxx)*(r7.xxxx)+(-(r2.wwww))).w;
    // 128: mad r7.xyz, cb0[18].xyzx, r2.wwww, r11.xyzx
    r7.xyz = ((source[18].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 129: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 130: mul r7.xyz, r7.xyzx, cb0[10].xyzx
    r7.xyz = ((r7.xyzx)*(source[10].xyzx)).xyz;
    // 131: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[10].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[10].wwww)).xyz;
    // 132: mov_sat r1.w, cb0[8].z
    r1.w = (saturate(source[8].zzzz)).w;
    // 133: mad r11.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r11.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 134: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 135: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 136: mad r11.xyz, r4.wwww, r11.xyzx, r2.wwww
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r2.wwww)).xyz;
    // 137: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 138: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 139: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 140: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 141: dp3 r2.w, r3.xyzx, r12.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 142: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 143: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 144: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 145: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 146: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 148: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 149: mad_sat r13.y, r3.w, l(0.300000), r4.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 150: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 151: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 153: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 154: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 155: add r1.w, r3.z, l(1.000000)
    r1.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 158: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 159: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 160: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 161: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 162: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 163: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 164: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 165: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 166: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 167: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 168: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 170: mul r7.xyz, r7.xyzx, r15.xyzx
    r7.xyz = ((r7.xyzx)*(r15.xyzx)).xyz;
    // 171: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 172: mad r2.xyz, -r2.xyzx, r4.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r4.wwww)+(r2.xyzx)).xyz;
    // 173: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 174: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 175: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 176: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 177: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 178: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 179: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 180: dp3 r7.x, r8.xyzx, r3.xyzx
    r7.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 181: dp3 r7.y, r9.xyzx, r3.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 182: dp2 r4.x, r7.xyxx, r4.xyxx
    r4.x = (dot((r7.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 183: dp2 r4.z, r7.xyxx, cb0[11].xyxx
    r4.z = (dot((r7.xyxx).xy,(source[11].xyxx).xy).xxxx).z;
    // 184: dp3 r4.y, r6.xyzx, r3.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 185: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 186: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 187: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 188: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r4.xyzx, t7.xyzw, s6, r2.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r4.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 189: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 190: mul r4.xyz, r4.xyzx, cb0[10].xyzx
    r4.xyz = ((r4.xyzx)*(source[10].xyzx)).xyz;
    // 191: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[10].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[10].wwww)).xyz;
    // 192: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 193: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 194: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 195: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 196: mul r3.yzw, r3.yyyy, cb0[21].xxyz
    r3.yzw = ((r3.yyyy)*(source[21].xxyz)).yzw;
    // 197: mad r3.xyz, cb0[20].xyzx, r3.xxxx, r3.yzwy
    r3.xyz = ((source[20].xyzx)*(r3.xxxx)+(r3.yzwy)).xyz;
    // 198: mul r3.xyz, r3.xyzx, cb0[22].wwww
    r3.xyz = ((r3.xyzx)*(source[22].wwww)).xyz;
    // 199: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 200: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 201: mad r2.xyz, r3.xyzx, r13.xzwx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r13.xzwx)+(r2.xyzx)).xyz;
    // 202: mul r3.xyz, r13.xzwx, r3.xyzx
    r3.xyz = ((r13.xzwx)*(r3.xyzx)).xyz;
    // 203: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 204: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 205: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 206: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 207: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 208: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 209: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 210: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 211: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 212: mul r0.xzw, r0.xxxx, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[2].xxyz)).xzw;
    // 213: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 214: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 215: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 216: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 217: mad o0.xyz, r1.xyzx, cb0[22].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[22].xyzx)+(r0.xyzx)).xyz;
    // 218: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 219: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 220: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 221: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 222: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 223: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 224: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 225: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 226: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 227: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 228: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 229: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 230: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 231: ftou r0.x, cb0[19].z
    r0.x = (asfloat((uint4)(source[19].zzzz))).x;
    // 232: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 233: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 234: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 235: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 236: ret
    return output;
}

// source.character.static-map-native-1121.v1 / source program 320d8a3c280c9848862049aec2713f82
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1121(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 12: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: mul r1.zw, r1.zzzw, cb0[6].zzzz
    r1.zw = ((r1.zzzw)*(source[6].zzzz)).zw;
    // 14: mad r1.xy, cb0[6].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
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
    // 22: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 28: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 40: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 41: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 42: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 43: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 44: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 45: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 46: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 47: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 48: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 50: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 51: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 52: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 53: mul r3.xyz, cb0[5].xyzx, cb0[8].zzzz
    r3.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 54: mul r8.xyz, r7.xyzx, r3.xyzx
    r8.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 55: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: mad r3.xyz, -r3.xyzx, r7.xyzx, r0.yyyy
    r3.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 57: mad r3.xyz, cb0[9].xxxx, r3.xyzx, r8.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[4].xyzx, cb0[8].yyyy
    r7.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 59: mad r3.xyz, -r4.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r3.xyzx)).xyz;
    // 60: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 61: mad r3.xyz, r0.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mul r4.xyz, r3.xyzx, cb0[9].yyyy
    r4.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 64: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r4.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    // 74: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 75: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 77: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 78: mad r4.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
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
    // 90: mad r4.xyz, r8.xyzx, r0.yyyy, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r0.yyyy)+(r4.xyzx)).xyz;
    // 91: mul r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)).xyz;
    // 92: max r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = (max(r0.yyyy,r4.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r8.xy, r0.zwzz, cb0[7].xxxx
    r8.xy = ((r0.zwzz)*(source[7].xxxx)).xy;
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
    // 118: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t9.xyzw, s6
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 119: mul r11.xyz, r11.xyzx, cb0[26].xyzx
    r11.xyz = ((r11.xyzx)*(source[26].xyzx)).xyz;
    // 120: dp3 r2.w, r11.xyzx, r10.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 121: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s6
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[25].xyzx
    r10.xyz = ((r10.xyzx)*(source[25].xyzx)).xyz;
    // 123: mul r12.xyz, r2.wwww, r10.xyzx
    r12.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 124: mad r9.xyz, r3.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 125: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 126: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 127: dp3 r9.y, r6.xyzx, r1.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 128: dp2 r12.z, r9.xyxx, cb0[13].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 129: dp3 r12.y, r5.xyzx, r1.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
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
    // 180: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
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
    // 193: mul r4.xyz, r4.xyzx, r12.xyzx
    r4.xyz = ((r4.xyzx)*(r12.xyzx)).xyz;
    // 194: mad r4.xyz, -r4.xyzx, r7.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r7.wwww)+(r4.xyzx)).xyz;
    // 195: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 196: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 197: dp3 r2.y, r6.xyzx, r1.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 198: dp2 r6.x, r2.xyxx, r7.xyxx
    r6.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 199: dp2 r6.z, r2.xyxx, cb0[13].xyxx
    r6.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
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
    // 207: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r6.xyzx, t7.xyzw, s7, r2.x
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r2.xyz, r5.xyzx, r5.wwww
    r2.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 211: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 212: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 213: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 214: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 215: mul r1.xyz, r5.xyzx, r5.xyzx
    r1.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 216: dp3 r1.x, r11.xyzx, r1.xyzx
    r1.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 217: add r1.y, -r1.x, r2.w
    r1.y = ((-(r1.xxxx))+(r2.wwww)).y;
    // 218: mad r1.x, r7.z, r1.y, r1.x
    r1.x = ((r7.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 219: mad r1.yzw, r10.xxyz, r1.xxxx, r8.xxyz
    r1.yzw = ((r10.xxyz)*(r1.xxxx)+(r8.xxyz)).yzw;
    // 220: mul r5.xyz, r1.xxxx, r10.xyzx
    r5.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 221: mad r1.x, r0.y, r13.x, r13.y
    r1.x = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).x;
    // 222: mad r1.x, r1.x, r0.y, r13.z
    r1.x = ((r1.xxxx)*(r0.yyyy)+(r13.zzzz)).x;
    // 223: mul r1.x, r0.y, r1.x
    r1.x = ((r0.yyyy)*(r1.xxxx)).x;
    // 224: max r0.y, r0.y, r1.x
    r0.y = (max(r0.yyyy,r1.xxxx)).y;
    // 225: mul r6.xyz, r0.yyyy, r1.yzwy
    r6.xyz = ((r0.yyyy)*(r1.yzwy)).xyz;
    // 226: add r1.xyz, r1.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 227: div r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)/(r1.xyzx)).xyz;
    // 228: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 229: mul r1.xyz, r2.xyzx, r6.xyzx
    r1.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 230: mad r2.xyz, r1.xyzx, r15.xzwx, r4.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r4.xyzx)).xyz;
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

// source.character.static-map-native-1121.v1 / source program 22432f265eb264418ce2f96754e7842f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1121(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1121(input);
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
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 12: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: mul r1.zw, r1.zzzw, cb0[6].zzzz
    r1.zw = ((r1.zzzw)*(source[6].zzzz)).zw;
    // 14: mad r1.xy, cb0[6].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
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
    // 22: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 28: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 40: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 41: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 42: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 43: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 44: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 45: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 46: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 47: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 48: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 50: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 51: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 52: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 53: mul r3.xyz, cb0[5].xyzx, cb0[8].zzzz
    r3.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 54: mul r8.xyz, r7.xyzx, r3.xyzx
    r8.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 55: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 56: mad r3.xyz, -r3.xyzx, r7.xyzx, r0.yyyy
    r3.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 57: mad r3.xyz, cb0[9].xxxx, r3.xyzx, r8.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r7.xyz, cb0[4].xyzx, cb0[8].yyyy
    r7.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 59: mad r3.xyz, -r4.xyzx, r7.xyzx, r3.xyzx
    r3.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r3.xyzx)).xyz;
    // 60: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 61: mad r3.xyz, r0.xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mul r4.xyz, r3.xyzx, cb0[9].yyyy
    r4.xyz = ((r3.xyzx)*(source[9].yyyy)).xyz;
    // 64: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r4.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
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
    // 74: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 75: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 77: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 78: mad r4.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
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
    // 90: mad r4.xyz, r8.xyzx, r0.yyyy, r4.xyzx
    r4.xyz = ((r8.xyzx)*(r0.yyyy)+(r4.xyzx)).xyz;
    // 91: mul r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = ((r0.yyyy)*(r4.xyzx)).xyz;
    // 92: max r4.xyz, r0.yyyy, r4.xyzx
    r4.xyz = (max(r0.yyyy,r4.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r8.xy, r0.zwzz, cb0[7].xxxx
    r8.xy = ((r0.zwzz)*(source[7].xxxx)).xy;
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
    // 114: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 115: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 116: dp3 r9.y, r6.xyzx, r1.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 117: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 118: dp3 r10.y, r5.xyzx, r1.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
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
    // 137: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 138: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r2.w, r2.w, cb0[11].x
    r2.w = ((r2.wwww)*(source[11].xxxx)).w;
    // 140: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 141: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 142: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 143: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 144: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 145: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 146: mul r11.xyz, r1.wwww, v5.xyzx
    r11.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 147: dp3 r1.w, r1.xyzx, r11.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 148: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 149: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 150: deriv_rtx_coarse r12.x, r1.w
    r12.x = (ddx_coarse(r1.wwww)).x;
    // 151: deriv_rty_coarse r12.y, r1.w
    r12.y = (ddy_coarse(r1.wwww)).y;
    // 152: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: dp2 r2.w, r12.xyxx, r12.xyxx
    r2.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 154: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 155: mad_sat r12.y, r2.w, l(0.300000), r7.z
    r12.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 156: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 157: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: add_sat r12.x, r1.w, -r2.w
    r12.x = (saturate((r1.wwww)+(-(r2.wwww)))).x;
    // 160: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t6.zwxy, s7
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 161: add r1.w, r0.y, r12.x
    r1.w = ((r0.yyyy)+(r12.xxxx)).w;
    // 162: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 163: add r2.w, -r12.y, l(1.000000)
    r2.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: mov_sat r3.w, cb0[10].y
    r3.w = (saturate(source[10].yyyy)).w;
    // 165: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 166: mul r4.w, r3.w, l(0.080000)
    r4.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 167: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 168: mad r13.xyz, r7.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 169: max r14.xyz, r2.wwww, r13.xyzx
    r14.xyz = (max(r2.wwww,r13.xyzx)).xyz;
    // 170: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 171: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 172: mul r14.xyz, r2.wwww, r14.xyzx
    r14.xyz = ((r2.wwww)*(r14.xyzx)).xyz;
    // 173: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 174: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 175: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 176: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 177: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 178: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 179: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 180: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 181: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 182: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 183: mul r4.xyz, r4.xyzx, r10.xyzx
    r4.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 184: mad r4.xyz, -r4.xyzx, r7.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r7.wwww)+(r4.xyzx)).xyz;
    // 185: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 186: dp3 r2.y, r6.xyzx, r1.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 187: dp2 r6.x, r2.xyxx, r7.xyxx
    r6.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 188: dp2 r6.z, r2.xyxx, cb0[13].xyxx
    r6.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 189: mul r2.x, r12.y, l(5.000000)
    r2.x = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 190: mul r2.y, r12.y, r12.y
    r2.y = ((r12.yyyy)*(r12.yyyy)).y;
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
    // 196: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 197: dp3 r1.x, r8.xyzx, r1.xyzx
    r1.x = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 198: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r6.xyzx, t7.xyzw, s6, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 202: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 203: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 204: mad r1.z, r0.y, r13.x, r13.y
    r1.z = ((r0.yyyy)*(r13.xxxx)+(r13.yyyy)).z;
    // 205: mad r1.z, r1.z, r0.y, r13.z
    r1.z = ((r1.zzzz)*(r0.yyyy)+(r13.zzzz)).z;
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
    // 213: mad r2.xyz, r1.xyzx, r12.xzwx, r4.xyzx
    r2.xyz = ((r1.xyzx)*(r12.xzwx)+(r4.xyzx)).xyz;
    // 214: mul r1.xyz, r12.xzwx, r1.xyzx
    r1.xyz = ((r12.xzwx)*(r1.xyzx)).xyz;
    // 215: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: dp3 r0.x, r0.xzwx, r11.xyzx
    r0.x = (dot((r0.xzwx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 217: add r0.y, -|r11.z|, l(1.000000)
    r0.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
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

// source.character.static-map-native-1122.v1 / source program c40f4e7bd55e1f4b974aceb3cfd2838e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1122(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterBaseConstants[11];
    source[11]=g_SourceCharacterBaseConstants[12];
    source[11].z=(g_SourceCharacterTime.xxxx).x;
    source[12]=g_SourceCharacterBaseConstants[14];
    source[12].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    source[30]=1.f;
    source[31]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
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
    // 6: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 7: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 8: mad r0.xyz, cb0[12].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[12].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r1.xyz, cb0[6].xyzx, cb0[13].xxxx
    r1.xyz = ((source[6].xyzx)*(source[13].xxxx)).xyz;
    // 10: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 11: mul r3.xyz, cb0[7].xyzx, cb0[13].yyyy
    r3.xyz = ((source[7].xyzx)*(source[13].yyyy)).xyz;
    // 12: mul r4.xy, v4.xyxx, cb0[8].yyyy
    r4.xy = ((v4.xyxx)*(source[8].yyyy)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 17: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 18: mad r3.xyz, -r3.xyzx, r5.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 19: mul r1.w, r5.w, r5.w
    r1.w = ((r5.wwww)*(r5.wwww)).w;
    // 20: mad r3.xyz, cb0[13].wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 21: mad r0.xyz, -r0.xyzx, r1.xyzx, r3.xyzx
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r3.xyzx)).xyz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 25: mul r1.xy, r1.xyxx, cb0[8].xxxx
    r1.xy = ((r1.xyxx)*(source[8].xxxx)).xy;
    // 26: mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 27: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 29: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 30: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 31: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 32: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 33: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 34: mul r2.w, r1.z, r1.z
    r2.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 35: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 36: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 38: max r1.w, cb0[8].w, l(0.000000)
    r1.w = (max(source[8].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 39: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 40: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 41: max r3.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 44: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 45: mul r5.xyz, r3.wwww, v0.xyzx
    r5.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 46: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 47: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 48: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 49: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 50: dp3 r6.z, r7.xyzx, r1.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 51: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 52: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 53: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 54: dp3 r6.y, r8.xyzx, r1.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 55: dp3 r3.x, r6.xyzx, r3.xyzx
    r3.x = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 56: add r3.x, r3.x, l(1.000000)
    r3.x = ((r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: mad r3.x, r3.x, l(0.500000), cb0[9].z
    r3.x = ((r3.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 58: mad r2.w, r3.x, r2.w, r3.x
    r2.w = ((r3.xxxx)*(r2.wwww)+(r3.xxxx)).w;
    // 59: add r3.x, -r1.w, r2.w
    r3.x = ((-(r1.wwww))+(r2.wwww)).x;
    // 60: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 62: mad r2.w, -r1.w, r3.x, r2.w
    r2.w = ((-(r1.wwww))*(r3.xxxx)+(r2.wwww)).w;
    // 63: mul r1.w, r3.x, r1.w
    r1.w = ((r3.xxxx)*(r1.wwww)).w;
    // 64: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 65: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 67: mul r2.xyz, r0.xyzx, cb0[14].xxxx
    r2.xyz = ((r0.xyzx)*(source[14].xxxx)).xyz;
    // 68: mad r0.xyz, cb0[14].yyyy, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[14].yyyy)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mul r1.w, r3.z, cb0[14].z
    r1.w = ((r3.zzzz)*(source[14].zzzz)).w;
    // 71: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 72: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 73: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 74: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 75: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 76: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul_sat r3.w, r1.w, cb2[3].w
    r3.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 78: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 79: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 81: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: mad r0.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 83: mad r6.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r6.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 84: mad r9.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 85: mul r1.w, r3.x, cb0[16].x
    r1.w = ((r3.xxxx)*(source[16].xxxx)).w;
    // 86: mul r3.x, r3.y, cb0[15].z
    r3.x = ((r3.yyyy)*(source[15].zzzz)).x;
    // 87: log r3.y, |r1.w|
    r3.y = (log2(abs(r1.wwww))).y;
    // 88: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: mul r3.y, r3.y, cb0[16].y
    r3.y = ((r3.yyyy)*(source[16].yyyy)).y;
    // 90: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 91: min r3.y, r3.y, l(1.000000)
    r3.y = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 92: movc r1.w, r1.w, l(0), r3.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).w;
    // 93: mad r6.xyz, r1.wwww, r6.xyzx, r9.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 94: mad r0.xyz, r6.xyzx, r1.wwww, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r1.wwww)+(r0.xyzx)).xyz;
    // 95: mul r0.xyz, r1.wwww, r0.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)).xyz;
    // 96: max r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 97: dp2 r3.y, r4.xyxx, r4.xyxx
    r3.y = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 98: mul r4.xy, r4.xyxx, cb0[8].zzzz
    r4.xy = ((r4.xyxx)*(source[8].zzzz)).xy;
    // 99: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 100: max r3.y, r3.y, l(0.000000)
    r3.y = (max(r3.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 101: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 102: add r4.z, r3.y, l(0.000010)
    r4.z = ((r3.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 103: add r4.xyz, -r1.xyzx, r4.xyzx
    r4.xyz = ((-(r1.xyzx))+(r4.xyzx)).xyz;
    // 104: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 105: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 106: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 107: mul r4.xyz, r0.wwww, r1.xyzx
    r4.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 108: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 109: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 110: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 111: dp3 r0.w, r6.xyzx, r4.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 112: mad r6.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 113: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 114: mul r6.yzw, r6.yyyy, cb0[28].xxyz
    r6.yzw = ((r6.yyyy)*(source[28].xxyz)).yzw;
    // 115: mad r6.xyz, r6.xxxx, cb0[27].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[27].xyzx)+(r6.yzwy)).xyz;
    // 116: mul r6.xyz, r6.xyzx, cb0[29].wwww
    r6.xyz = ((r6.xyzx)*(source[29].wwww)).xyz;
    // 117: mul r9.xyz, r2.xyzx, r6.xyzx
    r9.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 118: dp2_sat r10.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 119: dp3_sat r10.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 120: dp3_sat r10.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 121: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 122: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 123: mul r11.xyz, r11.xyzx, cb0[31].xyzx
    r11.xyz = ((r11.xyzx)*(source[31].xyzx)).xyz;
    // 124: dp3 r0.w, r11.xyzx, r10.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 125: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 126: mul r10.xyz, r10.xyzx, cb0[30].xyzx
    r10.xyz = ((r10.xyzx)*(source[30].xyzx)).xyz;
    // 127: mul r12.xyz, r0.wwww, r10.xyzx
    r12.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 128: mad r9.xyz, r2.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 129: mul r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 130: dp3 r9.x, r5.xyzx, r4.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 131: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 132: dp2 r12.z, r9.xyxx, cb0[18].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 133: dp3 r12.y, r7.xyzx, r4.xyzx
    r12.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 134: mul r13.xy, cb0[18].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r13.xy = ((source[18].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 135: dp2 r12.x, r9.xyxx, r13.xyxx
    r12.x = (dot((r9.xyxx).xy,(r13.xyxx).xy).xxxx).x;
    // 136: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 137: dp4 r14.x, cb0[19].xyzw, r12.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 138: dp4 r14.y, cb0[20].xyzw, r12.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 139: dp4 r14.z, cb0[21].xyzw, r12.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 140: mul r15.xyzw, r12.yzzx, r12.xyzz
    r15.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 141: dp4 r16.x, cb0[22].xyzw, r15.xyzw
    r16.x = (dot((source[22].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 142: dp4 r16.y, cb0[23].xyzw, r15.xyzw
    r16.y = (dot((source[23].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 143: dp4 r16.z, cb0[24].xyzw, r15.xyzw
    r16.z = (dot((source[24].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 144: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 145: mul r3.y, r12.y, r12.y
    r3.y = ((r12.yyyy)*(r12.yyyy)).y;
    // 146: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 147: mad r3.y, r12.x, r12.x, -r3.y
    r3.y = ((r12.xxxx)*(r12.xxxx)+(-(r3.yyyy))).y;
    // 148: mad r12.xyz, cb0[25].xyzx, r3.yyyy, r14.xyzx
    r12.xyz = ((source[25].xyzx)*(r3.yyyy)+(r14.xyzx)).xyz;
    // 149: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 150: mul r12.xyz, r12.xyzx, cb0[17].xyzx
    r12.xyz = ((r12.xyzx)*(source[17].xyzx)).xyz;
    // 151: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 152: mov_sat r2.w, cb0[15].x
    r2.w = (saturate(source[15].xxxx)).w;
    // 153: mad r14.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r14.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 154: mul r3.y, r2.w, l(0.080000)
    r3.y = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 155: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 156: mad r14.xyz, r3.wwww, r14.xyzx, r3.yyyy
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r3.yyyy)).xyz;
    // 157: mul_sat r2.w, r14.y, l(50.000000)
    r2.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 158: log r3.y, |r3.x|
    r3.y = (log2(abs(r3.xxxx))).y;
    // 159: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 160: mul r3.y, r3.y, cb0[15].w
    r3.y = ((r3.yyyy)*(source[15].wwww)).y;
    // 161: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 162: movc r3.x, r3.x, l(0), r3.y
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).x;
    // 163: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 164: min r3.z, r3.x, l(1.000000)
    r3.z = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 165: dp3 r3.x, v5.xyzx, v5.xyzx
    r3.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 166: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 167: mul r15.xyz, r3.xxxx, v5.xyzx
    r15.xyz = ((r3.xxxx)*(v5.xyzx)).xyz;
    // 168: dp3 r3.x, r4.xyzx, r15.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 169: mul r4.xyz, r3.xxxx, r4.xyzx
    r4.xyz = ((r3.xxxx)*(r4.xyzx)).xyz;
    // 170: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 171: deriv_rtx_coarse r16.x, r3.x
    r16.x = (ddx_coarse(r3.xxxx)).x;
    // 172: deriv_rty_coarse r16.y, r3.x
    r16.y = (ddy_coarse(r3.xxxx)).y;
    // 173: add r3.x, r3.x, l(1.000000)
    r3.x = ((r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 174: dp2 r3.y, r16.xyxx, r16.xyxx
    r3.y = (dot((r16.xyxx).xy,(r16.xyxx).xy).xxxx).y;
    // 175: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 176: mad_sat r16.y, r3.y, l(0.300000), r3.z
    r16.y = (saturate((r3.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 177: add r3.y, -r16.y, l(1.000000)
    r3.y = ((-(r16.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 178: max r17.xyz, r14.xyzx, r3.yyyy
    r17.xyz = (max(r14.xyzx,r3.yyyy)).xyz;
    // 179: add r17.xyz, -r14.xyzx, r17.xyzx
    r17.xyz = ((-(r14.xyzx))+(r17.xyzx)).xyz;
    // 180: mul r17.xyz, r2.wwww, r17.xyzx
    r17.xyz = ((r2.wwww)*(r17.xyzx)).xyz;
    // 181: add r2.w, r4.z, l(1.000000)
    r2.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 183: add_sat r16.x, -r2.w, r3.x
    r16.x = (saturate((-(r2.wwww))+(r3.xxxx))).x;
    // 184: sample_indexable(texture2d)(float,float,float,float) r3.xy, r16.xyxx, t5.xyzw, s7
    r3.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 185: add r2.w, r1.w, r16.x
    r2.w = ((r1.wwww)+(r16.xxxx)).w;
    // 186: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 187: mul r16.xzw, r3.yyyy, r14.xxyz
    r16.xzw = ((r3.yyyy)*(r14.xxyz)).xzw;
    // 188: mad r16.xzw, r17.xxyz, r3.xxxx, r16.xxzw
    r16.xzw = ((r17.xxyz)*(r3.xxxx)+(r16.xxzw)).xzw;
    // 189: div r3.x, l(1.000000, 1.000000, 1.000000, 1.000000), r3.y
    r3.x = r3.y != 0.f ? 1.f / r3.y : 0.f;
    // 190: add r3.x, r3.x, l(-1.000000)
    r3.x = ((r3.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 191: mad r17.xyz, r14.xyzx, r3.xxxx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r14.xyzx)*(r3.xxxx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 192: dp3 r3.x, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.x = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 193: mad r14.xyz, r3.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r3.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 194: mad r18.xyz, -r16.xzwx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xzwx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: mul r16.xzw, r16.xxzw, r17.xxyz
    r16.xzw = ((r16.xxzw)*(r17.xxyz)).xzw;
    // 196: mul r12.xyz, r12.xyzx, r18.xyzx
    r12.xyz = ((r12.xyzx)*(r18.xyzx)).xyz;
    // 197: mul r0.xyz, r0.xyzx, r12.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)).xyz;
    // 198: mad r0.xyz, -r0.xyzx, r3.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r3.wwww)+(r0.xyzx)).xyz;
    // 199: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 200: dp3 r3.x, r5.xyzx, r4.xyzx
    r3.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 201: dp3 r3.y, r8.xyzx, r4.xyzx
    r3.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 202: dp2 r5.x, r3.xyxx, r13.xyxx
    r5.x = (dot((r3.xyxx).xy,(r13.xyxx).xy).xxxx).x;
    // 203: dp2 r5.z, r3.xyxx, cb0[18].xyxx
    r5.z = (dot((r3.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 204: mul r3.x, r16.y, l(5.000000)
    r3.x = ((r16.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 205: mul r3.y, r16.y, r16.y
    r3.y = ((r16.yyyy)*(r16.yyyy)).y;
    // 206: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 207: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 208: add r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)+(r2.wwww)).w;
    // 209: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 210: add_sat r1.w, r2.w, l(-1.000000)
    r1.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 211: dp3 r5.y, r7.xyzx, r4.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 212: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r5.xyzx, t6.xyzw, s6, r3.x
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 213: mul r3.xyw, r5.xyxz, r5.wwww
    r3.xyw = ((r5.xyxz)*(r5.wwww)).xyw;
    // 214: mul r3.xyw, r3.xyxw, cb0[17].xyxz
    r3.xyw = ((r3.xyxw)*(source[17].xyxz)).xyw;
    // 215: mad r3.xyw, r3.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[17].wwww
    r3.xyw = ((r3.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[17].wwww)).xyw;
    // 216: dp2_sat r5.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 217: dp3_sat r5.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 218: dp3_sat r5.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 219: mul r4.xyz, r5.xyzx, r5.xyzx
    r4.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 220: dp3 r2.w, r11.xyzx, r4.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 221: add r0.w, r0.w, -r2.w
    r0.w = ((r0.wwww)+(-(r2.wwww))).w;
    // 222: mad r0.w, r3.z, r0.w, r2.w
    r0.w = ((r3.zzzz)*(r0.wwww)+(r2.wwww)).w;
    // 223: mad r4.xyz, r10.xyzx, r0.wwww, r6.xyzx
    r4.xyz = ((r10.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 224: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 225: mad r0.w, r1.w, r14.x, r14.y
    r0.w = ((r1.wwww)*(r14.xxxx)+(r14.yyyy)).w;
    // 226: mad r0.w, r0.w, r1.w, r14.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r14.zzzz)).w;
    // 227: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 228: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 229: mul r6.xyz, r0.wwww, r4.xyzx
    r6.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 230: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 231: div r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)/(r4.xyzx)).xyz;
    // 232: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 233: mul r3.xyz, r3.xywx, r6.xyzx
    r3.xyz = ((r3.xywx)*(r6.xyzx)).xyz;
    // 234: mad r0.xyz, r3.xyzx, r16.xzwx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r16.xzwx)+(r0.xyzx)).xyz;
    // 235: mul r3.xyz, r16.xzwx, r3.xyzx
    r3.xyz = ((r16.xzwx)*(r3.xyzx)).xyz;
    // 236: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 237: dp3 r1.x, r1.xyzx, r15.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 238: add r1.y, -|r15.z|, l(1.000000)
    r1.y = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 239: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 240: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 241: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 242: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 243: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 244: mul r1.yzw, r1.yyyy, cb0[3].xxyz
    r1.yzw = ((r1.yyyy)*(source[3].xxyz)).yzw;
    // 245: lt r2.w, |r1.x|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 246: movc r1.yzw, r2.wwww, l(0,0,0,0), r1.yyzw
    r1.yzw = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzw)).yzw;
    // 247: mad_sat r2.w, r1.x, cb0[10].y, -cb0[10].z
    r2.w = (saturate((r1.xxxx)*(source[10].yyyy)+(-(source[10].zzzz)))).w;
    // 248: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 249: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 250: mul r3.x, r3.x, cb0[10].w
    r3.x = ((r3.xxxx)*(source[10].wwww)).x;
    // 251: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 252: mul r3.xyz, r3.xxxx, cb0[4].xyzx
    r3.xyz = ((r3.xxxx)*(source[4].xyzx)).xyz;
    // 253: movc r3.xyz, r2.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 254: add r3.xyz, r3.xyzx, -cb0[4].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[4].xyzx))).xyz;
    // 255: mad r3.xyz, cb0[4].wwww, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((source[4].wwww)*(r3.xyzx)+(source[4].xyzx)).xyz;
    // 256: mul r4.xyz, cb0[5].xyzx, cb0[11].yyyy
    r4.xyz = ((source[5].xyzx)*(source[11].yyyy)).xyz;
    // 257: mul r4.xyz, r4.xyzx, cb0[12].xxxx
    r4.xyz = ((r4.xyzx)*(source[12].xxxx)).xyz;
    // 258: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 259: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 260: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 261: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 262: mul r4.xyz, r4.xyzx, cb0[12].yyyy
    r4.xyz = ((r4.xyzx)*(source[12].yyyy)).xyz;
    // 263: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 264: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 265: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 266: mad r1.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.yzwy
    r1.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.yzwy)).xyz;
    // 267: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 268: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 269: mad o0.xyz, r2.xyzx, cb0[29].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 270: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 271: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 272: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 273: mul r1.xyz, r1.xxxx, r9.xyzx
    r1.xyz = ((r1.xxxx)*(r9.xyzx)).xyz;
    // 274: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 275: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 276: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 277: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 278: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 279: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 280: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 281: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 282: mul o4.z, r0.w, r0.x
    output.targets[4].z = ((r0.wwww)*(r0.xxxx)).z;
    // 283: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 284: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 285: ftou r0.x, cb0[26].z
    r0.x = (asfloat((uint4)(source[26].zzzz))).x;
    // 286: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 287: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 288: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 289: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 290: ret
    return output;
}

// source.character.static-map-native-1122.v1 / source program c6bed953635cdb45a1dcc51ba6fccf00
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1122(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1122(input);
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
    source[10]=g_SourceCharacterBaseConstants[11];
    source[11]=g_SourceCharacterBaseConstants[12];
    source[11].z=(g_SourceCharacterTime.xxxx).x;
    source[12]=g_SourceCharacterBaseConstants[14];
    source[12].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
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
    // 6: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 7: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 8: mad r0.xyz, cb0[12].wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((source[12].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r1.xyz, cb0[6].xyzx, cb0[13].xxxx
    r1.xyz = ((source[6].xyzx)*(source[13].xxxx)).xyz;
    // 10: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 11: mul r3.xyz, cb0[7].xyzx, cb0[13].yyyy
    r3.xyz = ((source[7].xyzx)*(source[13].yyyy)).xyz;
    // 12: mul r4.xy, v4.xyxx, cb0[8].yyyy
    r4.xy = ((v4.xyxx)*(source[8].yyyy)).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 17: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 18: mad r3.xyz, -r3.xyzx, r5.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 19: mul r1.w, r5.w, r5.w
    r1.w = ((r5.wwww)*(r5.wwww)).w;
    // 20: mad r3.xyz, cb0[13].wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((source[13].wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 21: mad r0.xyz, -r0.xyzx, r1.xyzx, r3.xyzx
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r3.xyzx)).xyz;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 25: mul r1.xy, r1.xyxx, cb0[8].xxxx
    r1.xy = ((r1.xyxx)*(source[8].xxxx)).xy;
    // 26: mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 27: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 28: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 29: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 30: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 31: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 32: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 33: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 34: mul r2.w, r1.z, r1.z
    r2.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 35: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 36: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 38: max r1.w, cb0[8].w, l(0.000000)
    r1.w = (max(source[8].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 39: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 40: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 41: max r3.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r3.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 44: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 45: mul r5.xyz, r3.wwww, v0.xyzx
    r5.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 46: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 47: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 48: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 49: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 50: dp3 r6.z, r7.xyzx, r1.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 51: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 52: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 53: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 54: dp3 r6.y, r8.xyzx, r1.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 55: dp3 r3.x, r6.xyzx, r3.xyzx
    r3.x = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 56: add r3.x, r3.x, l(1.000000)
    r3.x = ((r3.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 57: mad r3.x, r3.x, l(0.500000), cb0[9].z
    r3.x = ((r3.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].zzzz)).x;
    // 58: mad r2.w, r3.x, r2.w, r3.x
    r2.w = ((r3.xxxx)*(r2.wwww)+(r3.xxxx)).w;
    // 59: add r3.x, -r1.w, r2.w
    r3.x = ((-(r1.wwww))+(r2.wwww)).x;
    // 60: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 61: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 62: mad r2.w, -r1.w, r3.x, r2.w
    r2.w = ((-(r1.wwww))*(r3.xxxx)+(r2.wwww)).w;
    // 63: mul r1.w, r3.x, r1.w
    r1.w = ((r3.xxxx)*(r1.wwww)).w;
    // 64: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 65: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 66: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 67: mul r2.xyz, r0.xyzx, cb0[14].xxxx
    r2.xyz = ((r0.xyzx)*(source[14].xxxx)).xyz;
    // 68: mad r0.xyz, cb0[14].yyyy, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[14].yyyy)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mul r1.w, r3.z, cb0[14].z
    r1.w = ((r3.zzzz)*(source[14].zzzz)).w;
    // 71: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 72: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 73: mul r2.w, r2.w, cb0[14].w
    r2.w = ((r2.wwww)*(source[14].wwww)).w;
    // 74: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 75: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 76: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul_sat r3.w, r1.w, cb2[3].w
    r3.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 78: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 79: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 81: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: mad r0.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 83: mad r6.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r6.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 84: mad r9.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 85: mul r1.w, r3.x, cb0[16].x
    r1.w = ((r3.xxxx)*(source[16].xxxx)).w;
    // 86: mul r3.x, r3.y, cb0[15].z
    r3.x = ((r3.yyyy)*(source[15].zzzz)).x;
    // 87: log r3.y, |r1.w|
    r3.y = (log2(abs(r1.wwww))).y;
    // 88: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: mul r3.y, r3.y, cb0[16].y
    r3.y = ((r3.yyyy)*(source[16].yyyy)).y;
    // 90: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 91: min r3.y, r3.y, l(1.000000)
    r3.y = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 92: movc r1.w, r1.w, l(0), r3.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).w;
    // 93: mad r6.xyz, r1.wwww, r6.xyzx, r9.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 94: mad r0.xyz, r6.xyzx, r1.wwww, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r1.wwww)+(r0.xyzx)).xyz;
    // 95: mul r0.xyz, r1.wwww, r0.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)).xyz;
    // 96: max r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 97: dp2 r3.y, r4.xyxx, r4.xyxx
    r3.y = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 98: mul r4.xy, r4.xyxx, cb0[8].zzzz
    r4.xy = ((r4.xyxx)*(source[8].zzzz)).xy;
    // 99: add r3.y, -r3.y, l(1.000000)
    r3.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 100: max r3.y, r3.y, l(0.000000)
    r3.y = (max(r3.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 101: sqrt r3.y, r3.y
    r3.y = (sqrt(r3.yyyy)).y;
    // 102: add r4.z, r3.y, l(0.000010)
    r4.z = ((r3.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 103: add r4.xyz, -r1.xyzx, r4.xyzx
    r4.xyz = ((-(r1.xyzx))+(r4.xyzx)).xyz;
    // 104: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 105: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 106: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 107: mul r4.xyz, r0.wwww, r1.xyzx
    r4.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 108: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 109: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 110: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 111: dp3 r0.w, r6.xyzx, r4.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 112: mad r9.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 113: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 114: mul r9.yzw, r9.yyyy, cb0[28].xxyz
    r9.yzw = ((r9.yyyy)*(source[28].xxyz)).yzw;
    // 115: mad r9.xyz, r9.xxxx, cb0[27].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[27].xyzx)+(r9.yzwy)).xyz;
    // 116: mul r9.xyz, r9.xyzx, cb0[29].wwww
    r9.xyz = ((r9.xyzx)*(source[29].wwww)).xyz;
    // 117: mul r9.xyz, r2.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r9.xyzx)).xyz;
    // 118: mul r0.xyz, r0.xyzx, r9.xyzx
    r0.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 119: dp3 r9.x, r5.xyzx, r4.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 120: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 121: dp2 r10.z, r9.xyxx, cb0[18].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 122: dp3 r10.y, r7.xyzx, r4.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 123: mul r11.xy, cb0[18].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r11.xy = ((source[18].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 124: dp2 r10.x, r9.xyxx, r11.xyxx
    r10.x = (dot((r9.xyxx).xy,(r11.xyxx).xy).xxxx).x;
    // 125: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 126: dp4 r12.x, cb0[19].xyzw, r10.xyzw
    r12.x = (dot((source[19].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 127: dp4 r12.y, cb0[20].xyzw, r10.xyzw
    r12.y = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 128: dp4 r12.z, cb0[21].xyzw, r10.xyzw
    r12.z = (dot((source[21].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 129: mul r13.xyzw, r10.yzzx, r10.xyzz
    r13.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 130: dp4 r14.x, cb0[22].xyzw, r13.xyzw
    r14.x = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 131: dp4 r14.y, cb0[23].xyzw, r13.xyzw
    r14.y = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 132: dp4 r14.z, cb0[24].xyzw, r13.xyzw
    r14.z = (dot((source[24].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 133: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 134: mul r0.w, r10.y, r10.y
    r0.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 135: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 136: mad r0.w, r10.x, r10.x, -r0.w
    r0.w = ((r10.xxxx)*(r10.xxxx)+(-(r0.wwww))).w;
    // 137: mad r10.xyz, cb0[25].xyzx, r0.wwww, r12.xyzx
    r10.xyz = ((source[25].xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 138: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 139: mul r10.xyz, r10.xyzx, cb0[17].xyzx
    r10.xyz = ((r10.xyzx)*(source[17].xyzx)).xyz;
    // 140: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 141: mov_sat r2.w, cb0[15].x
    r2.w = (saturate(source[15].xxxx)).w;
    // 142: mad r12.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r12.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 143: mul r0.w, r2.w, l(0.080000)
    r0.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 144: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 145: mad r12.xyz, r3.wwww, r12.xyzx, r0.wwww
    r12.xyz = ((r3.wwww)*(r12.xyzx)+(r0.wwww)).xyz;
    // 146: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 147: log r2.w, |r3.x|
    r2.w = (log2(abs(r3.xxxx))).w;
    // 148: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 149: mul r2.w, r2.w, cb0[15].w
    r2.w = ((r2.wwww)*(source[15].wwww)).w;
    // 150: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 151: movc r2.w, r3.x, l(0), r2.w
    r2.w = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 152: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 153: min r3.z, r2.w, l(1.000000)
    r3.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 154: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 156: mul r13.xyz, r2.wwww, v5.xyzx
    r13.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r2.w, r4.xyzx, r13.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 158: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 159: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 160: deriv_rtx_coarse r3.x, r2.w
    r3.x = (ddx_coarse(r2.wwww)).x;
    // 161: deriv_rty_coarse r3.y, r2.w
    r3.y = (ddy_coarse(r2.wwww)).y;
    // 162: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: dp2 r3.x, r3.xyxx, r3.xyxx
    r3.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 164: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 165: mad_sat r3.y, r3.x, l(0.300000), r3.z
    r3.y = (saturate((r3.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 166: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 167: add r3.z, -r3.y, l(1.000000)
    r3.z = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 168: max r14.xyz, r12.xyzx, r3.zzzz
    r14.xyz = (max(r12.xyzx,r3.zzzz)).xyz;
    // 169: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 170: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 171: add r0.w, r4.z, l(1.000000)
    r0.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: add_sat r3.x, -r0.w, r2.w
    r3.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 174: sample_indexable(texture2d)(float,float,float,float) r11.zw, r3.xyxx, t5.zwxy, s6
    r11.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 175: add r0.w, r1.w, r3.x
    r0.w = ((r1.wwww)+(r3.xxxx)).w;
    // 176: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 177: mul r15.xyz, r11.wwww, r12.xyzx
    r15.xyz = ((r11.wwww)*(r12.xyzx)).xyz;
    // 178: mad r14.xyz, r14.xyzx, r11.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r11.zzzz)+(r15.xyzx)).xyz;
    // 179: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.w
    r2.w = r11.w != 0.f ? 1.f / r11.w : 0.f;
    // 180: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 181: mad r15.xyz, r12.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r12.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 182: dp3 r2.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: mad r12.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 184: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 186: mul r10.xyz, r10.xyzx, r16.xyzx
    r10.xyz = ((r10.xyzx)*(r16.xyzx)).xyz;
    // 187: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 188: mad r0.xyz, -r0.xyzx, r3.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r3.wwww)+(r0.xyzx)).xyz;
    // 189: dp3 r5.x, r5.xyzx, r4.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 190: dp3 r5.y, r8.xyzx, r4.xyzx
    r5.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 191: dp2 r8.x, r5.xyxx, r11.xyxx
    r8.x = (dot((r5.xyxx).xy,(r11.xyxx).xy).xxxx).x;
    // 192: dp2 r8.z, r5.xyxx, cb0[18].xyxx
    r8.z = (dot((r5.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 193: mul r2.w, r3.y, l(5.000000)
    r2.w = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 194: mul r3.x, r3.y, r3.y
    r3.x = ((r3.yyyy)*(r3.yyyy)).x;
    // 195: mul r0.w, r0.w, r3.x
    r0.w = ((r0.wwww)*(r3.xxxx)).w;
    // 196: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 197: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 198: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 199: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 200: dp3 r8.y, r7.xyzx, r4.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 201: dp3 r1.w, r6.xyzx, r4.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 202: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 203: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 204: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r8.xyzx, t6.xyzw, s5, r2.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r8.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 205: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 206: mul r4.xyz, r4.xyzx, cb0[17].xyzx
    r4.xyz = ((r4.xyzx)*(source[17].xyzx)).xyz;
    // 207: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[17].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[17].wwww)).xyz;
    // 208: mad r1.w, r0.w, r12.x, r12.y
    r1.w = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).w;
    // 209: mad r1.w, r1.w, r0.w, r12.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r12.zzzz)).w;
    // 210: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 211: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 212: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 213: mad r3.xyz, cb0[27].xyzx, r3.xxxx, r3.yzwy
    r3.xyz = ((source[27].xyzx)*(r3.xxxx)+(r3.yzwy)).xyz;
    // 214: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 215: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 216: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 217: mad r0.xyz, r3.xyzx, r14.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r14.xyzx)+(r0.xyzx)).xyz;
    // 218: mul r3.xyz, r14.xyzx, r3.xyzx
    r3.xyz = ((r14.xyzx)*(r3.xyzx)).xyz;
    // 219: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 220: dp3 r0.w, r1.xyzx, r13.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 221: add r1.x, -|r13.z|, l(1.000000)
    r1.x = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 222: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 223: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 224: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 225: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 226: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 227: mul r1.xyz, r1.xxxx, cb0[3].xyzx
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)).xyz;
    // 228: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 229: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 230: mad_sat r1.w, r0.w, cb0[10].y, -cb0[10].z
    r1.w = (saturate((r0.wwww)*(source[10].yyyy)+(-(source[10].zzzz)))).w;
    // 231: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 232: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 233: mul r2.w, r2.w, cb0[10].w
    r2.w = ((r2.wwww)*(source[10].wwww)).w;
    // 234: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 235: mul r3.xyz, r2.wwww, cb0[4].xyzx
    r3.xyz = ((r2.wwww)*(source[4].xyzx)).xyz;
    // 236: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 237: add r3.xyz, r3.xyzx, -cb0[4].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[4].xyzx))).xyz;
    // 238: mad r3.xyz, cb0[4].wwww, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((source[4].wwww)*(r3.xyzx)+(source[4].xyzx)).xyz;
    // 239: mul r4.xyz, cb0[5].xyzx, cb0[11].yyyy
    r4.xyz = ((source[5].xyzx)*(source[11].yyyy)).xyz;
    // 240: mul r4.xyz, r4.xyzx, cb0[12].xxxx
    r4.xyz = ((r4.xyzx)*(source[12].xxxx)).xyz;
    // 241: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 242: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 243: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 244: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 245: mul r4.xyz, r4.xyzx, cb0[12].yyyy
    r4.xyz = ((r4.xyzx)*(source[12].yyyy)).xyz;
    // 246: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 247: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 248: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 249: mad r1.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.xyzx
    r1.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.xyzx)).xyz;
    // 250: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 251: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 252: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 253: mad o0.xyz, r2.xyzx, cb0[29].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[29].xyzx)+(r1.xyzx)).xyz;
    // 254: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 255: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 256: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 257: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 258: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 259: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 260: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 261: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 262: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 263: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 264: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 265: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 266: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 267: ftou r0.x, cb0[26].z
    r0.x = (asfloat((uint4)(source[26].zzzz))).x;
    // 268: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 269: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 270: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 271: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 272: ret
    return output;
}

// source.character.static-map-native-1123.v1 / source program 953c082ee39421429220921892a51b81
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1123(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterBaseConstants[11];
    source[11]=g_SourceCharacterBaseConstants[12];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[12].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[12].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[12].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    source[30]=1.f;
    source[31]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(-0.333300)
    r1.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 3: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 4: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 7: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 8: mad r0.xyz, cb0[13].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r1.xyz, cb0[6].xyzx, cb0[13].zzzz
    r1.xyz = ((source[6].xyzx)*(source[13].zzzz)).xyz;
    // 10: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 11: mul r3.xyz, cb0[7].xyzx, cb0[13].wwww
    r3.xyz = ((source[7].xyzx)*(source[13].wwww)).xyz;
    // 12: mul r4.xyzw, v4.xyxy, cb0[8].yyww
    r4.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r4.zwzz, t4.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 15: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 16: mad r3.xyz, -r3.xyzx, r5.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 17: mul r1.w, r5.w, r5.w
    r1.w = ((r5.wwww)*(r5.wwww)).w;
    // 18: mad r3.xyz, cb0[14].yyyy, r3.xyzx, r6.xyzx
    r3.xyz = ((source[14].yyyy)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 19: mad r0.xyz, -r0.xyzx, r1.xyzx, r3.xyzx
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r3.xyzx)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r4.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 22: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.xy, r1.xyxx, cb0[8].zzzz
    r1.xy = ((r1.xyxx)*(source[8].zzzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 26: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 27: mad r1.xy, cb0[8].xxxx, r3.zwzz, r1.xyxx
    r1.xy = ((source[8].xxxx)*(r3.zwzz)+(r1.xyxx)).xy;
    // 28: dp2 r1.z, r3.zwzz, r3.zwzz
    r1.z = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).z;
    // 29: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 31: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 32: add r4.z, r1.z, l(0.000010)
    r4.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: mul r4.xy, r1.xyxx, v2.wwww
    r4.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 34: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 35: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 36: div r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 37: mul r2.w, r1.z, r1.z
    r2.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 38: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 39: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 41: max r1.w, cb0[9].y, l(0.000000)
    r1.w = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 43: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 44: max r4.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 45: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 47: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 48: mul r5.xyz, r3.zzzz, v0.xyzx
    r5.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 49: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 51: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 52: mul r7.xyz, r3.zzzz, v1.xyzx
    r7.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 53: dp3 r6.z, r7.xyzx, r1.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 54: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 55: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 56: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 57: dp3 r6.y, r8.xyzx, r1.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 58: dp3 r3.z, r6.xyzx, r4.xyzx
    r3.z = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 59: add r3.z, r3.z, l(1.000000)
    r3.z = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: mad r3.z, r3.z, l(0.500000), cb0[10].x
    r3.z = ((r3.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].xxxx)).z;
    // 61: mad r2.w, r3.z, r2.w, r3.z
    r2.w = ((r3.zzzz)*(r2.wwww)+(r3.zzzz)).w;
    // 62: add r3.z, -r1.w, r2.w
    r3.z = ((-(r1.wwww))+(r2.wwww)).z;
    // 63: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 65: mad r2.w, -r1.w, r3.z, r2.w
    r2.w = ((-(r1.wwww))*(r3.zzzz)+(r2.wwww)).w;
    // 66: mul r1.w, r3.z, r1.w
    r1.w = ((r3.zzzz)*(r1.wwww)).w;
    // 67: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 68: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 69: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 70: mul r2.xyz, r0.xyzx, cb0[14].zzzz
    r2.xyz = ((r0.xyzx)*(source[14].zzzz)).xyz;
    // 71: mad r0.xyz, cb0[14].wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[14].wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r1.w, r4.z, cb0[15].x
    r1.w = ((r4.zzzz)*(source[15].xxxx)).w;
    // 74: mul r3.zw, r4.yyyx, cb0[16].xxxz
    r3.zw = ((r4.yyyx)*(source[16].xxxz)).zw;
    // 75: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 76: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 77: mul r2.w, r2.w, cb0[15].y
    r2.w = ((r2.wwww)*(source[15].yyyy)).w;
    // 78: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 79: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 80: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 82: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 83: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 85: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 86: dp2 r0.x, r3.xyxx, r3.xyxx
    r0.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 87: mul r6.xy, r3.xyxx, cb0[9].xxxx
    r6.xy = ((r3.xyxx)*(source[9].xxxx)).xy;
    // 88: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 90: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 91: add r6.z, r0.x, l(0.000010)
    r6.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 92: add r0.xyz, -r1.xyzx, r6.xyzx
    r0.xyz = ((-(r1.xyzx))+(r6.xyzx)).xyz;
    // 93: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 94: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 97: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 98: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 99: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 100: dp3 r0.w, r6.xyzx, r1.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 101: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 103: mul r6.xyz, r3.yyyy, cb0[28].xyzx
    r6.xyz = ((r3.yyyy)*(source[28].xyzx)).xyz;
    // 104: mad r6.xyz, r3.xxxx, cb0[27].xyzx, r6.xyzx
    r6.xyz = ((r3.xxxx)*(source[27].xyzx)+(r6.xyzx)).xyz;
    // 105: mul r6.xyz, r6.xyzx, cb0[29].wwww
    r6.xyz = ((r6.xyzx)*(source[29].wwww)).xyz;
    // 106: mul r9.xyz, r2.xyzx, r6.xyzx
    r9.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 107: dp2_sat r10.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 108: dp3_sat r10.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 109: dp3_sat r10.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 110: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 111: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t9.xyzw, s6
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 112: mul r11.xyz, r11.xyzx, cb0[31].xyzx
    r11.xyz = ((r11.xyzx)*(source[31].xyzx)).xyz;
    // 113: dp3 r0.w, r11.xyzx, r10.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 114: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s6
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 115: mul r10.xyz, r10.xyzx, cb0[30].xyzx
    r10.xyz = ((r10.xyzx)*(source[30].xyzx)).xyz;
    // 116: mul r12.xyz, r0.wwww, r10.xyzx
    r12.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 117: mad r9.xyz, r2.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 118: mad r12.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r12.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 119: mad r13.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 120: mad r14.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 121: log r3.xy, |r3.zwzz|
    r3.xy = (log2(abs(r3.zwzz))).xy;
    // 122: lt r3.zw, |r3.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r3.zw = (asfloat((uint4)((abs(r3.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 123: mul r3.xy, r3.xyxx, cb0[16].ywyy
    r3.xy = ((r3.xyxx)*(source[16].ywyy)).xy;
    // 124: exp r3.xy, r3.xyxx
    r3.xy = (exp2(r3.xyxx)).xy;
    // 125: min r1.w, r3.y, l(1.000000)
    r1.w = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: movc r3.x, r3.z, l(0), r3.x
    r3.x = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).x;
    // 127: movc r1.w, r3.w, l(0), r1.w
    r1.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 128: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 129: min r4.z, r3.x, l(1.000000)
    r4.z = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 130: mad r3.xyz, r1.wwww, r13.xyzx, r14.xyzx
    r3.xyz = ((r1.wwww)*(r13.xyzx)+(r14.xyzx)).xyz;
    // 131: mad r3.xyz, r3.xyzx, r1.wwww, r12.xyzx
    r3.xyz = ((r3.xyzx)*(r1.wwww)+(r12.xyzx)).xyz;
    // 132: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 133: max r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = (max(r1.wwww,r3.xyzx)).xyz;
    // 134: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 135: dp3 r9.x, r5.xyzx, r1.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 136: dp3 r9.y, r8.xyzx, r1.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 137: dp2 r12.z, r9.xyxx, cb0[18].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 138: dp3 r12.y, r7.xyzx, r1.xyzx
    r12.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 139: mul r4.xy, cb0[18].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[18].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 140: dp2 r12.x, r9.xyxx, r4.xyxx
    r12.x = (dot((r9.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 141: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 142: dp4 r13.x, cb0[19].xyzw, r12.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 143: dp4 r13.y, cb0[20].xyzw, r12.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 144: dp4 r13.z, cb0[21].xyzw, r12.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 145: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 146: dp4 r15.x, cb0[22].xyzw, r14.xyzw
    r15.x = (dot((source[22].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 147: dp4 r15.y, cb0[23].xyzw, r14.xyzw
    r15.y = (dot((source[23].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 148: dp4 r15.z, cb0[24].xyzw, r14.xyzw
    r15.z = (dot((source[24].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 149: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 150: mul r3.w, r12.y, r12.y
    r3.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 151: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 152: mad r3.w, r12.x, r12.x, -r3.w
    r3.w = ((r12.xxxx)*(r12.xxxx)+(-(r3.wwww))).w;
    // 153: mad r12.xyz, cb0[25].xyzx, r3.wwww, r13.xyzx
    r12.xyz = ((source[25].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 154: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 155: mul r12.xyz, r12.xyzx, cb0[17].xyzx
    r12.xyz = ((r12.xyzx)*(source[17].xyzx)).xyz;
    // 156: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 157: mov_sat r2.w, cb0[15].z
    r2.w = (saturate(source[15].zzzz)).w;
    // 158: mad r13.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r13.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 159: mul r3.w, r2.w, l(0.080000)
    r3.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 160: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 161: mad r13.xyz, r4.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r4.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 162: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 163: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 164: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 165: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 166: dp3 r3.w, r1.xyzx, r14.xyzx
    r3.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 167: mul r1.xyz, r1.xyzx, r3.wwww
    r1.xyz = ((r1.xyzx)*(r3.wwww)).xyz;
    // 168: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 169: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 170: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 171: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 173: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 174: mad_sat r15.y, r5.w, l(0.300000), r4.z
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 175: add r5.w, -r15.y, l(1.000000)
    r5.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: max r16.xyz, r13.xyzx, r5.wwww
    r16.xyz = (max(r13.xyzx,r5.wwww)).xyz;
    // 177: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 178: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 179: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 180: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 182: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 183: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 184: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
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
    // 195: mul r3.xyz, r3.xyzx, r12.xyzx
    r3.xyz = ((r3.xyzx)*(r12.xyzx)).xyz;
    // 196: mad r3.xyz, -r3.xyzx, r4.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r4.wwww)+(r3.xyzx)).xyz;
    // 197: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 198: dp3 r5.x, r5.xyzx, r1.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 199: dp3 r5.y, r8.xyzx, r1.xyzx
    r5.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 200: dp2 r8.x, r5.xyxx, r4.xyxx
    r8.x = (dot((r5.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 201: dp2 r8.z, r5.xyxx, cb0[18].xyxx
    r8.z = (dot((r5.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 202: mul r3.w, r15.y, l(5.000000)
    r3.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 203: mul r4.x, r15.y, r15.y
    r4.x = ((r15.yyyy)*(r15.yyyy)).x;
    // 204: mul r2.w, r2.w, r4.x
    r2.w = ((r2.wwww)*(r4.xxxx)).w;
    // 205: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 206: add r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)+(r2.wwww)).w;
    // 207: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 208: add_sat r1.w, r2.w, l(-1.000000)
    r1.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 209: dp3 r8.y, r7.xyzx, r1.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 210: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r8.xyzx, t7.xyzw, s7, r3.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r8.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 211: mul r4.xyw, r5.xyxz, r5.wwww
    r4.xyw = ((r5.xyxz)*(r5.wwww)).xyw;
    // 212: mul r4.xyw, r4.xyxw, cb0[17].xyxz
    r4.xyw = ((r4.xyxw)*(source[17].xyxz)).xyw;
    // 213: mad r4.xyw, r4.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[17].wwww
    r4.xyw = ((r4.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[17].wwww)).xyw;
    // 214: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 215: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 216: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 217: mul r1.xyz, r5.xyzx, r5.xyzx
    r1.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 218: dp3 r1.x, r11.xyzx, r1.xyzx
    r1.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 219: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 220: mad r0.w, r4.z, r0.w, r1.x
    r0.w = ((r4.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 221: mad r1.xyz, r10.xyzx, r0.wwww, r6.xyzx
    r1.xyz = ((r10.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 222: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 223: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 224: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 225: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 226: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 227: mul r6.xyz, r0.wwww, r1.xyzx
    r6.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 228: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 229: div r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)/(r1.xyzx)).xyz;
    // 230: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 231: mul r1.xyz, r4.xywx, r6.xyzx
    r1.xyz = ((r4.xywx)*(r6.xyzx)).xyz;
    // 232: mad r3.xyz, r1.xyzx, r15.xzwx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r15.xzwx)+(r3.xyzx)).xyz;
    // 233: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 234: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 235: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 236: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 237: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 238: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 239: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 240: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 241: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 242: mul r1.xyz, r0.yyyy, cb0[3].xyzx
    r1.xyz = ((r0.yyyy)*(source[3].xyzx)).xyz;
    // 243: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 244: movc r1.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 245: mad_sat r0.y, r0.x, cb0[10].w, -cb0[11].x
    r0.y = (saturate((r0.xxxx)*(source[10].wwww)+(-(source[11].xxxx)))).y;
    // 246: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 247: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 248: mul r0.z, r0.z, cb0[11].y
    r0.z = ((r0.zzzz)*(source[11].yyyy)).z;
    // 249: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 250: mul r4.xyz, r0.zzzz, cb0[4].xyzx
    r4.xyz = ((r0.zzzz)*(source[4].xyzx)).xyz;
    // 251: movc r4.xyz, r0.yyyy, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 252: add r4.xyz, r4.xyzx, -cb0[4].xyzx
    r4.xyz = ((r4.xyzx)+(-(source[4].xyzx))).xyz;
    // 253: mad r4.xyz, cb0[4].wwww, r4.xyzx, cb0[4].xyzx
    r4.xyz = ((source[4].wwww)*(r4.xyzx)+(source[4].xyzx)).xyz;
    // 254: mul r5.xyz, cb0[5].xyzx, cb0[11].wwww
    r5.xyz = ((source[5].xyzx)*(source[11].wwww)).xyz;
    // 255: mul r5.xyz, r5.xyzx, cb0[12].zzzz
    r5.xyz = ((r5.xyzx)*(source[12].zzzz)).xyz;
    // 256: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 257: mul r0.xyz, r0.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 258: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 259: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 260: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 261: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 262: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 263: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 264: mad r0.xyz, r0.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.xyzx)).xyz;
    // 265: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 266: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 267: mad o0.xyz, r2.xyzx, cb0[29].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[29].xyzx)+(r0.xyzx)).xyz;
    // 268: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 269: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 270: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 271: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 272: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 273: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 274: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 275: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 276: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 277: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 278: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 279: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 280: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 281: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 282: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 283: ftou r0.x, cb0[26].z
    r0.x = (asfloat((uint4)(source[26].zzzz))).x;
    // 284: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 285: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 286: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 287: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 288: ret
    return output;
}

// source.character.static-map-native-1123.v1 / source program 219e463f0c891a4ca0e60dc3b53214b2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1123(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1123(input);
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
    source[10]=g_SourceCharacterBaseConstants[11];
    source[11]=g_SourceCharacterBaseConstants[12];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[12].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[12].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[12].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[17]=g_SourceCharacterEnvironmentColor;source[18]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r1.x, r0.w, l(-0.333300)
    r1.x = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).x;
    // 3: lt r1.x, r1.x, l(0.000000)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 4: discard_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 7: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 8: mad r0.xyz, cb0[13].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[13].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r1.xyz, cb0[6].xyzx, cb0[13].zzzz
    r1.xyz = ((source[6].xyzx)*(source[13].zzzz)).xyz;
    // 10: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 11: mul r3.xyz, cb0[7].xyzx, cb0[13].wwww
    r3.xyz = ((source[7].xyzx)*(source[13].wwww)).xyz;
    // 12: mul r4.xyzw, v4.xyxy, cb0[8].yyww
    r4.xyzw = ((v4.xyxy)*(source[8].yyww)).xyzw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r4.zwzz, t4.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 15: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 16: mad r3.xyz, -r3.xyzx, r5.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 17: mul r1.w, r5.w, r5.w
    r1.w = ((r5.wwww)*(r5.wwww)).w;
    // 18: mad r3.xyz, cb0[14].yyyy, r3.xyzx, r6.xyzx
    r3.xyz = ((source[14].yyyy)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 19: mad r0.xyz, -r0.xyzx, r1.xyzx, r3.xyzx
    r0.xyz = ((-(r0.xyzx))*(r1.xyzx)+(r3.xyzx)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r4.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r4.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 22: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.xy, r1.xyxx, cb0[8].zzzz
    r1.xy = ((r1.xyxx)*(source[8].zzzz)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 26: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 27: mad r1.xy, cb0[8].xxxx, r3.zwzz, r1.xyxx
    r1.xy = ((source[8].xxxx)*(r3.zwzz)+(r1.xyxx)).xy;
    // 28: dp2 r1.z, r3.zwzz, r3.zwzz
    r1.z = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).z;
    // 29: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 30: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 31: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 32: add r4.z, r1.z, l(0.000010)
    r4.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: mul r4.xy, r1.xyxx, v2.wwww
    r4.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 34: dp3 r1.x, r4.xyzx, r4.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 35: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 36: div r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)/(r1.xxxx)).xyz;
    // 37: mul r2.w, r1.z, r1.z
    r2.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 38: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 39: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 41: max r1.w, cb0[9].y, l(0.000000)
    r1.w = (max(source[9].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 42: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 43: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 44: max r4.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 45: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: dp3 r3.z, v0.xyzx, v0.xyzx
    r3.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 47: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 48: mul r5.xyz, r3.zzzz, v0.xyzx
    r5.xyz = ((r3.zzzz)*(v0.xyzx)).xyz;
    // 49: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 50: dp3 r3.z, v1.xyzx, v1.xyzx
    r3.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 51: rsq r3.z, r3.z
    r3.z = (rsqrt(r3.zzzz)).z;
    // 52: mul r7.xyz, r3.zzzz, v1.xyzx
    r7.xyz = ((r3.zzzz)*(v1.xyzx)).xyz;
    // 53: dp3 r6.z, r7.xyzx, r1.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 54: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 55: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 56: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 57: dp3 r6.y, r8.xyzx, r1.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 58: dp3 r3.z, r6.xyzx, r4.xyzx
    r3.z = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 59: add r3.z, r3.z, l(1.000000)
    r3.z = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: mad r3.z, r3.z, l(0.500000), cb0[10].x
    r3.z = ((r3.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].xxxx)).z;
    // 61: mad r2.w, r3.z, r2.w, r3.z
    r2.w = ((r3.zzzz)*(r2.wwww)+(r3.zzzz)).w;
    // 62: add r3.z, -r1.w, r2.w
    r3.z = ((-(r1.wwww))+(r2.wwww)).z;
    // 63: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 64: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 65: mad r2.w, -r1.w, r3.z, r2.w
    r2.w = ((-(r1.wwww))*(r3.zzzz)+(r2.wwww)).w;
    // 66: mul r1.w, r3.z, r1.w
    r1.w = ((r3.zzzz)*(r1.wwww)).w;
    // 67: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 68: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 69: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 70: mul r2.xyz, r0.xyzx, cb0[14].zzzz
    r2.xyz = ((r0.xyzx)*(source[14].zzzz)).xyz;
    // 71: mad r0.xyz, cb0[14].wwww, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[14].wwww)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r1.w, r4.z, cb0[15].x
    r1.w = ((r4.zzzz)*(source[15].xxxx)).w;
    // 74: mul r3.zw, r4.yyyx, cb0[16].xxxz
    r3.zw = ((r4.yyyx)*(source[16].xxxz)).zw;
    // 75: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 76: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 77: mul r2.w, r2.w, cb0[15].y
    r2.w = ((r2.wwww)*(source[15].yyyy)).w;
    // 78: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 79: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 80: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 82: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 83: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 85: mad_sat r2.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 86: dp2 r0.x, r3.xyxx, r3.xyxx
    r0.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 87: mul r6.xy, r3.xyxx, cb0[9].xxxx
    r6.xy = ((r3.xyxx)*(source[9].xxxx)).xy;
    // 88: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 90: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 91: add r6.z, r0.x, l(0.000010)
    r6.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 92: add r0.xyz, -r1.xyzx, r6.xyzx
    r0.xyz = ((-(r1.xyzx))+(r6.xyzx)).xyz;
    // 93: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 94: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 97: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 98: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 99: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 100: dp3 r0.w, r6.xyzx, r1.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 101: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 103: mul r9.xyz, r3.yyyy, cb0[28].xyzx
    r9.xyz = ((r3.yyyy)*(source[28].xyzx)).xyz;
    // 104: mad r9.xyz, r3.xxxx, cb0[27].xyzx, r9.xyzx
    r9.xyz = ((r3.xxxx)*(source[27].xyzx)+(r9.xyzx)).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[29].wwww
    r9.xyz = ((r9.xyzx)*(source[29].wwww)).xyz;
    // 106: mul r9.xyz, r2.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r9.xyzx)).xyz;
    // 107: mad r10.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 108: mad r11.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 109: mad r12.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 110: log r3.xy, |r3.zwzz|
    r3.xy = (log2(abs(r3.zwzz))).xy;
    // 111: lt r3.zw, |r3.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r3.zw = (asfloat((uint4)((abs(r3.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 112: mul r3.xy, r3.xyxx, cb0[16].ywyy
    r3.xy = ((r3.xyxx)*(source[16].ywyy)).xy;
    // 113: exp r3.xy, r3.xyxx
    r3.xy = (exp2(r3.xyxx)).xy;
    // 114: min r0.w, r3.y, l(1.000000)
    r0.w = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: movc r1.w, r3.z, l(0), r3.x
    r1.w = ((asuint(r3.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 116: movc r0.w, r3.w, l(0), r0.w
    r0.w = ((asuint(r3.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 117: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 118: min r4.z, r1.w, l(1.000000)
    r4.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 119: mad r3.xyz, r0.wwww, r11.xyzx, r12.xyzx
    r3.xyz = ((r0.wwww)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 120: mad r3.xyz, r3.xyzx, r0.wwww, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r0.wwww)+(r10.xyzx)).xyz;
    // 121: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 122: max r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = (max(r0.wwww,r3.xyzx)).xyz;
    // 123: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 124: dp3 r9.x, r5.xyzx, r1.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 125: dp3 r9.y, r8.xyzx, r1.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 126: dp2 r10.z, r9.xyxx, cb0[18].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 127: dp3 r10.y, r7.xyzx, r1.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 128: mul r4.xy, cb0[18].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((source[18].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 129: dp2 r10.x, r9.xyxx, r4.xyxx
    r10.x = (dot((r9.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 130: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 131: dp4 r11.x, cb0[19].xyzw, r10.xyzw
    r11.x = (dot((source[19].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 132: dp4 r11.y, cb0[20].xyzw, r10.xyzw
    r11.y = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 133: dp4 r11.z, cb0[21].xyzw, r10.xyzw
    r11.z = (dot((source[21].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 134: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 135: dp4 r13.x, cb0[22].xyzw, r12.xyzw
    r13.x = (dot((source[22].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 136: dp4 r13.y, cb0[23].xyzw, r12.xyzw
    r13.y = (dot((source[23].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 137: dp4 r13.z, cb0[24].xyzw, r12.xyzw
    r13.z = (dot((source[24].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 138: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 139: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 140: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 141: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 142: mad r10.xyz, cb0[25].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[25].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 143: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 144: mul r10.xyz, r10.xyzx, cb0[17].xyzx
    r10.xyz = ((r10.xyzx)*(source[17].xyzx)).xyz;
    // 145: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[17].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[17].wwww)).xyz;
    // 146: mov_sat r2.w, cb0[15].z
    r2.w = (saturate(source[15].zzzz)).w;
    // 147: mad r11.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r11.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 148: mul r1.w, r2.w, l(0.080000)
    r1.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 149: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 150: mad r11.xyz, r4.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r4.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 151: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 152: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 153: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 154: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 155: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 156: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 157: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 158: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 159: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 160: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 162: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 163: mad_sat r13.y, r3.w, l(0.300000), r4.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 164: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 165: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 167: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 168: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 169: add r1.w, r1.z, l(1.000000)
    r1.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 172: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 173: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 174: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 175: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 176: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 177: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 178: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 179: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 180: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 184: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 185: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 186: mad r3.xyz, -r3.xyzx, r4.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r4.wwww)+(r3.xyzx)).xyz;
    // 187: dp3 r5.x, r5.xyzx, r1.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 188: dp3 r5.y, r8.xyzx, r1.xyzx
    r5.y = (dot((r8.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 189: dp2 r4.x, r5.xyxx, r4.xyxx
    r4.x = (dot((r5.xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 190: dp2 r4.z, r5.xyxx, cb0[18].xyxx
    r4.z = (dot((r5.xyxx).xy,(source[18].xyxx).xy).xxxx).z;
    // 191: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 192: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 193: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 194: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 195: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 196: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 197: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 198: dp3 r4.y, r7.xyzx, r1.xyzx
    r4.y = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 199: dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 200: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 201: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 202: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r4.xyzx, t7.xyzw, s6, r2.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r4.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 203: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 204: mul r4.xyz, r4.xyzx, cb0[17].xyzx
    r4.xyz = ((r4.xyzx)*(source[17].xyzx)).xyz;
    // 205: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[17].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[17].wwww)).xyz;
    // 206: mad r1.z, r0.w, r11.x, r11.y
    r1.z = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).z;
    // 207: mad r1.z, r1.z, r0.w, r11.z
    r1.z = ((r1.zzzz)*(r0.wwww)+(r11.zzzz)).z;
    // 208: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 209: max r0.w, r0.w, r1.z
    r0.w = (max(r0.wwww,r1.zzzz)).w;
    // 210: mul r1.yzw, r1.yyyy, cb0[28].xxyz
    r1.yzw = ((r1.yyyy)*(source[28].xxyz)).yzw;
    // 211: mad r1.xyz, cb0[27].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[27].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 212: mul r1.xyz, r1.xyzx, cb0[29].wwww
    r1.xyz = ((r1.xyzx)*(source[29].wwww)).xyz;
    // 213: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 214: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 215: mad r3.xyz, r1.xyzx, r13.xzwx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r13.xzwx)+(r3.xyzx)).xyz;
    // 216: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 217: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 218: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 219: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 220: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 221: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 222: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 223: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 224: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 225: mul r0.yzw, r0.yyyy, cb0[3].xxyz
    r0.yzw = ((r0.yyyy)*(source[3].xxyz)).yzw;
    // 226: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 227: movc r0.yzw, r1.xxxx, l(0,0,0,0), r0.yyzw
    r0.yzw = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzw)).yzw;
    // 228: mad_sat r1.x, r0.x, cb0[10].w, -cb0[11].x
    r1.x = (saturate((r0.xxxx)*(source[10].wwww)+(-(source[11].xxxx)))).x;
    // 229: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 230: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 231: mul r1.y, r1.y, cb0[11].y
    r1.y = ((r1.yyyy)*(source[11].yyyy)).y;
    // 232: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 233: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 234: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 235: add r1.xyz, r1.xyzx, -cb0[4].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[4].xyzx))).xyz;
    // 236: mad r1.xyz, cb0[4].wwww, r1.xyzx, cb0[4].xyzx
    r1.xyz = ((source[4].wwww)*(r1.xyzx)+(source[4].xyzx)).xyz;
    // 237: mul r4.xyz, cb0[5].xyzx, cb0[11].wwww
    r4.xyz = ((source[5].xyzx)*(source[11].wwww)).xyz;
    // 238: mul r4.xyz, r4.xyzx, cb0[12].zzzz
    r4.xyz = ((r4.xyzx)*(source[12].zzzz)).xyz;
    // 239: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 240: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 241: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 242: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 243: mul r4.xyz, r4.xyzx, cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(source[12].wwww)).xyz;
    // 244: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 245: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 246: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 247: mad r0.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r0.yzwy
    r0.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r0.yzwy)).xyz;
    // 248: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 249: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 250: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 251: mad o0.xyz, r2.xyzx, cb0[29].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[29].xyzx)+(r0.xyzx)).xyz;
    // 252: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 253: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 254: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 255: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 256: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 257: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 258: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 259: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 260: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 261: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 262: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 263: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 264: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 265: ftou r0.x, cb0[26].z
    r0.x = (asfloat((uint4)(source[26].zzzz))).x;
    // 266: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 267: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 268: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 269: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 270: ret
    return output;
}

// source.character.static-map-native-1124.v1 / source program a740582bac31b344ac78de5248145370
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1124(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[11]=g_SourceCharacterEnvironmentColor;source[12]=g_SourceCharacterEnvironmentRotation;}
    source[24]=1.f;
    source[25]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[5].yyww
    r2.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[5].zzzz
    r0.zw = ((r0.zzzw)*(source[5].zzzz)).zw;
    // 12: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r4.xyz, -r1.xyzx, r1.wwww
    r4.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 39: mad r1.xyz, cb0[6].wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((source[6].wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 40: mul r4.xyz, cb0[4].xyzx, cb0[7].yyyy
    r4.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 41: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 42: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 44: mad r3.xyz, cb0[7].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[7].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 45: mul r4.xyz, cb0[3].xyzx, cb0[7].xxxx
    r4.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // 46: mad r3.xyz, -r1.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 47: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 48: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 49: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 50: mul r3.xyz, r1.xyzx, cb0[8].xxxx
    r3.xyz = ((r1.xyzx)*(source[8].xxxx)).xyz;
    // 51: mad r1.xyz, cb0[8].yyyy, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[8].yyyy)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r1.w, r4.z, cb0[8].z
    r1.w = ((r4.zzzz)*(source[8].zzzz)).w;
    // 54: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 55: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: mul r2.z, r2.z, cb0[8].w
    r2.z = ((r2.zzzz)*(source[8].wwww)).z;
    // 57: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 58: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 59: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 61: mad r1.xyz, r2.zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 62: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 64: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 65: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 66: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 67: mad r6.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r6.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 68: mul r2.z, r4.x, cb0[10].x
    r2.z = ((r4.xxxx)*(source[10].xxxx)).z;
    // 69: mul r2.w, r4.y, cb0[9].z
    r2.w = ((r4.yyyy)*(source[9].zzzz)).w;
    // 70: log r3.w, |r2.z|
    r3.w = (log2(abs(r2.zzzz))).w;
    // 71: lt r2.z, |r2.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r3.w, r3.w, cb0[10].y
    r3.w = ((r3.wwww)*(source[10].yyyy)).w;
    // 73: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 74: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: movc r2.z, r2.z, l(0), r3.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).z;
    // 76: mad r5.xyz, r2.zzzz, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.zzzz)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 77: mad r3.xyz, r5.xyzx, r2.zzzz, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r2.zzzz)+(r3.xyzx)).xyz;
    // 78: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 79: max r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = (max(r2.zzzz,r3.xyzx)).xyz;
    // 80: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 81: mul r5.xy, r2.xyxx, cb0[6].xxxx
    r5.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 82: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 84: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 85: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 86: add r5.xyz, -r0.xyzx, r5.xyzx
    r5.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 87: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 88: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r5.xyz, r0.wwww, r0.xyzx
    r5.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 91: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 92: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 93: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 94: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 95: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 97: mul r6.xyz, r2.yyyy, cb0[22].xyzx
    r6.xyz = ((r2.yyyy)*(source[22].xyzx)).xyz;
    // 98: mad r6.xyz, r2.xxxx, cb0[21].xyzx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(source[21].xyzx)+(r6.xyzx)).xyz;
    // 99: mul r6.xyz, r6.xyzx, cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(source[23].wwww)).xyz;
    // 100: mul r7.xyz, r1.xyzx, r6.xyzx
    r7.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 101: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 102: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 103: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 104: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 105: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t9.xyzw, s6
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 106: mul r9.xyz, r9.xyzx, cb0[25].xyzx
    r9.xyz = ((r9.xyzx)*(source[25].xyzx)).xyz;
    // 107: dp3 r0.w, r9.xyzx, r8.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 108: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s6
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 109: mul r8.xyz, r8.xyzx, cb0[24].xyzx
    r8.xyz = ((r8.xyzx)*(source[24].xyzx)).xyz;
    // 110: mul r10.xyz, r0.wwww, r8.xyzx
    r10.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 111: mad r7.xyz, r1.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 112: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 113: dp3 r2.x, v1.xyzx, v1.xyzx
    r2.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 114: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 115: mul r7.xyz, r2.xxxx, v1.xyzx
    r7.xyz = ((r2.xxxx)*(v1.xyzx)).xyz;
    // 116: dp3 r10.y, r7.xyzx, r5.xyzx
    r10.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 117: dp3 r2.x, v0.xyzx, v0.xyzx
    r2.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 118: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 119: mul r11.xyz, r2.xxxx, v0.xyzx
    r11.xyz = ((r2.xxxx)*(v0.xyzx)).xyz;
    // 120: mul r12.xyz, r7.zxyz, r11.yzxy
    r12.xyz = ((r7.zxyz)*(r11.yzxy)).xyz;
    // 121: mad r12.xyz, r7.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r7.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 122: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 123: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 124: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 125: dp2 r10.z, r13.xyxx, cb0[12].xyxx
    r10.z = (dot((r13.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 126: mul r2.xy, cb0[12].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((source[12].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 127: dp2 r10.x, r13.xyxx, r2.xyxx
    r10.x = (dot((r13.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 128: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r14.x, cb0[13].xyzw, r10.xyzw
    r14.x = (dot((source[13].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 130: dp4 r14.y, cb0[14].xyzw, r10.xyzw
    r14.y = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 131: dp4 r14.z, cb0[15].xyzw, r10.xyzw
    r14.z = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 132: mul r15.xyzw, r10.yzzx, r10.xyzz
    r15.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 133: dp4 r16.x, cb0[16].xyzw, r15.xyzw
    r16.x = (dot((source[16].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 134: dp4 r16.y, cb0[17].xyzw, r15.xyzw
    r16.y = (dot((source[17].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 135: dp4 r16.z, cb0[18].xyzw, r15.xyzw
    r16.z = (dot((source[18].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 136: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 137: mul r3.w, r10.y, r10.y
    r3.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 138: mov r13.z, r10.y
    r13.z = (r10.yyyy).z;
    // 139: mad r3.w, r10.x, r10.x, -r3.w
    r3.w = ((r10.xxxx)*(r10.xxxx)+(-(r3.wwww))).w;
    // 140: mad r10.xyz, cb0[19].xyzx, r3.wwww, r14.xyzx
    r10.xyz = ((source[19].xyzx)*(r3.wwww)+(r14.xyzx)).xyz;
    // 141: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r10.xyz, r10.xyzx, cb0[11].xyzx
    r10.xyz = ((r10.xyzx)*(source[11].xyzx)).xyz;
    // 143: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[11].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[11].wwww)).xyz;
    // 144: mov_sat r1.w, cb0[9].x
    r1.w = (saturate(source[9].xxxx)).w;
    // 145: mad r14.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r14.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 146: mul r3.w, r1.w, l(0.080000)
    r3.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 148: mad r14.xyz, r4.wwww, r14.xyzx, r3.wwww
    r14.xyz = ((r4.wwww)*(r14.xyzx)+(r3.wwww)).xyz;
    // 149: mul_sat r1.w, r14.y, l(50.000000)
    r1.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: log r3.w, |r2.w|
    r3.w = (log2(abs(r2.wwww))).w;
    // 151: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 152: mul r3.w, r3.w, cb0[9].w
    r3.w = ((r3.wwww)*(source[9].wwww)).w;
    // 153: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 154: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 155: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 156: min r4.z, r2.w, l(1.000000)
    r4.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 158: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 159: mul r15.xyz, r2.wwww, v5.xyzx
    r15.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 160: dp3 r2.w, r5.xyzx, r15.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r15.xyzx).xyz).xxxx).w;
    // 161: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 162: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 163: deriv_rtx_coarse r4.x, r2.w
    r4.x = (ddx_coarse(r2.wwww)).x;
    // 164: deriv_rty_coarse r4.y, r2.w
    r4.y = (ddy_coarse(r2.wwww)).y;
    // 165: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: dp2 r3.w, r4.xyxx, r4.xyxx
    r3.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 167: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 168: mad_sat r4.y, r3.w, l(0.300000), r4.z
    r4.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 169: add r3.w, -r4.y, l(1.000000)
    r3.w = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: max r16.xyz, r14.xyzx, r3.wwww
    r16.xyz = (max(r14.xyzx,r3.wwww)).xyz;
    // 171: add r16.xyz, -r14.xyzx, r16.xyzx
    r16.xyz = ((-(r14.xyzx))+(r16.xyzx)).xyz;
    // 172: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 173: add r1.w, r5.z, l(1.000000)
    r1.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: add_sat r4.x, -r1.w, r2.w
    r4.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 176: sample_indexable(texture2d)(float,float,float,float) r17.xy, r4.xyxx, t6.xyzw, s8
    r17.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 177: add r1.w, r2.z, r4.x
    r1.w = ((r2.zzzz)+(r4.xxxx)).w;
    // 178: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 179: mul r18.xyz, r14.xyzx, r17.yyyy
    r18.xyz = ((r14.xyzx)*(r17.yyyy)).xyz;
    // 180: mad r16.xyz, r16.xyzx, r17.xxxx, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xxxx)+(r18.xyzx)).xyz;
    // 181: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r17.y
    r2.w = r17.y != 0.f ? 1.f / r17.y : 0.f;
    // 182: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 183: mad r17.xyz, r14.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r14.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 185: mad r14.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 186: mad r18.xyz, -r16.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 187: mul r16.xyz, r16.xyzx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xyzx)).xyz;
    // 188: mul r10.xyz, r10.xyzx, r18.xyzx
    r10.xyz = ((r10.xyzx)*(r18.xyzx)).xyz;
    // 189: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 190: mad r3.xyz, -r3.xyzx, r4.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r4.wwww)+(r3.xyzx)).xyz;
    // 191: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 192: dp2_sat r10.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 193: dp3_sat r10.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 194: dp3_sat r10.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 195: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 196: dp3 r2.w, r9.xyzx, r10.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 197: add r0.w, r0.w, -r2.w
    r0.w = ((r0.wwww)+(-(r2.wwww))).w;
    // 198: mad r0.w, r4.z, r0.w, r2.w
    r0.w = ((r4.zzzz)*(r0.wwww)+(r2.wwww)).w;
    // 199: mad r4.xzw, r8.xxyz, r0.wwww, r6.xxyz
    r4.xzw = ((r8.xxyz)*(r0.wwww)+(r6.xxyz)).xzw;
    // 200: mul r6.xyz, r0.wwww, r8.xyzx
    r6.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 201: mul r0.w, r4.y, r4.y
    r0.w = ((r4.yyyy)*(r4.yyyy)).w;
    // 202: mul r2.w, r4.y, l(5.000000)
    r2.w = ((r4.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 203: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 204: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 205: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 206: mov o5.y, r2.z
    output.targets[5].y = (r2.zzzz).y;
    // 207: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 208: mad r1.w, r0.w, r14.x, r14.y
    r1.w = ((r0.wwww)*(r14.xxxx)+(r14.yyyy)).w;
    // 209: mad r1.w, r1.w, r0.w, r14.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r14.zzzz)).w;
    // 210: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 211: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 212: mul r8.xyz, r0.wwww, r4.xzwx
    r8.xyz = ((r0.wwww)*(r4.xzwx)).xyz;
    // 213: add r4.xyz, r4.xzwx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xzwx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 214: div r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)/(r4.xyzx)).xyz;
    // 215: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 216: dp3 r4.x, r11.xyzx, r5.xyzx
    r4.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 217: dp3 r4.y, r12.xyzx, r5.xyzx
    r4.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 218: dp3 r5.y, r7.xyzx, r5.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 219: dp2 r5.x, r4.xyxx, r2.xyxx
    r5.x = (dot((r4.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 220: dp2 r5.z, r4.xyxx, cb0[12].xyxx
    r5.z = (dot((r4.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 221: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t7.xyzw, s7, r2.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 222: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 223: mul r2.xyz, r2.xyzx, cb0[11].xyzx
    r2.xyz = ((r2.xyzx)*(source[11].xyzx)).xyz;
    // 224: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[11].wwww)).xyz;
    // 225: mul r2.xyz, r8.xyzx, r2.xyzx
    r2.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 226: mad r3.xyz, r2.xyzx, r16.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r16.xyzx)+(r3.xyzx)).xyz;
    // 227: mul r2.xyz, r16.xyzx, r2.xyzx
    r2.xyz = ((r16.xyzx)*(r2.xyzx)).xyz;
    // 228: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 229: dp3 r0.x, r0.xyzx, r15.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 230: add r0.y, -|r15.z|, l(1.000000)
    r0.y = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 231: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 232: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 233: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 234: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 235: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 236: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 237: mul r2.xyz, r0.xxxx, cb0[2].xyzx
    r2.xyz = ((r0.xxxx)*(source[2].xyzx)).xyz;
    // 238: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 239: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 240: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 241: mad o0.xyz, r1.xyzx, cb0[23].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[23].xyzx)+(r0.xyzx)).xyz;
    // 242: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 243: dp3 r0.x, r13.xyzx, r13.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 244: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 245: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 246: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 247: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 248: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 249: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 250: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 251: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 252: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 253: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 254: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 255: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 256: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 257: ftou r0.x, cb0[20].z
    r0.x = (asfloat((uint4)(source[20].zzzz))).x;
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

// source.character.static-map-native-1124.v1 / source program 5ca19dda21ba5d49b71db7448c800cbf
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1124(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1124(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[11]=g_SourceCharacterEnvironmentColor;source[12]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[5].yyww
    r2.xyzw = ((v4.xyxy)*(source[5].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[5].zzzz
    r0.zw = ((r0.zzzw)*(source[5].zzzz)).zw;
    // 12: mad r0.xy, cb0[5].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[5].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r4.xyz, -r1.xyzx, r1.wwww
    r4.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 39: mad r1.xyz, cb0[6].wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((source[6].wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 40: mul r4.xyz, cb0[4].xyzx, cb0[7].yyyy
    r4.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 41: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 42: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 44: mad r3.xyz, cb0[7].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[7].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 45: mul r4.xyz, cb0[3].xyzx, cb0[7].xxxx
    r4.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // 46: mad r3.xyz, -r1.xyzx, r4.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r3.xyzx)).xyz;
    // 47: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 48: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 49: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 50: mul r3.xyz, r1.xyzx, cb0[8].xxxx
    r3.xyz = ((r1.xyzx)*(source[8].xxxx)).xyz;
    // 51: mad r1.xyz, cb0[8].yyyy, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[8].yyyy)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r1.w, r4.z, cb0[8].z
    r1.w = ((r4.zzzz)*(source[8].zzzz)).w;
    // 54: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 55: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 56: mul r2.z, r2.z, cb0[8].w
    r2.z = ((r2.zzzz)*(source[8].wwww)).z;
    // 57: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 58: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 59: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 60: mul_sat r4.w, r1.w, cb2[3].w
    r4.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 61: mad r1.xyz, r2.zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 62: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 64: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 65: mad r3.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 66: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 67: mad r6.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r6.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 68: mul r2.z, r4.x, cb0[10].x
    r2.z = ((r4.xxxx)*(source[10].xxxx)).z;
    // 69: mul r2.w, r4.y, cb0[9].z
    r2.w = ((r4.yyyy)*(source[9].zzzz)).w;
    // 70: log r3.w, |r2.z|
    r3.w = (log2(abs(r2.zzzz))).w;
    // 71: lt r2.z, |r2.z|, l(0.000001)
    r2.z = (asfloat((uint4)((abs(r2.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r3.w, r3.w, cb0[10].y
    r3.w = ((r3.wwww)*(source[10].yyyy)).w;
    // 73: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 74: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: movc r2.z, r2.z, l(0), r3.w
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).z;
    // 76: mad r5.xyz, r2.zzzz, r5.xyzx, r6.xyzx
    r5.xyz = ((r2.zzzz)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 77: mad r3.xyz, r5.xyzx, r2.zzzz, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r2.zzzz)+(r3.xyzx)).xyz;
    // 78: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 79: max r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = (max(r2.zzzz,r3.xyzx)).xyz;
    // 80: dp2 r3.w, r2.xyxx, r2.xyxx
    r3.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 81: mul r5.xy, r2.xyxx, cb0[6].xxxx
    r5.xy = ((r2.xyxx)*(source[6].xxxx)).xy;
    // 82: add r2.x, -r3.w, l(1.000000)
    r2.x = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 84: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 85: add r5.z, r2.x, l(0.000010)
    r5.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 86: add r5.xyz, -r0.xyzx, r5.xyzx
    r5.xyz = ((-(r0.xyzx))+(r5.xyzx)).xyz;
    // 87: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 88: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r5.xyz, r0.wwww, r0.xyzx
    r5.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 91: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 92: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 93: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 94: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 95: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 97: mul r7.xyz, r2.yyyy, cb0[22].xyzx
    r7.xyz = ((r2.yyyy)*(source[22].xyzx)).xyz;
    // 98: mad r7.xyz, r2.xxxx, cb0[21].xyzx, r7.xyzx
    r7.xyz = ((r2.xxxx)*(source[21].xyzx)+(r7.xyzx)).xyz;
    // 99: mul r7.xyz, r7.xyzx, cb0[23].wwww
    r7.xyz = ((r7.xyzx)*(source[23].wwww)).xyz;
    // 100: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 101: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 102: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 103: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 104: mul r7.xyz, r0.wwww, v1.xyzx
    r7.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 105: dp3 r8.y, r7.xyzx, r5.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 106: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 107: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 108: mul r9.xyz, r0.wwww, v0.xyzx
    r9.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 109: mul r10.xyz, r7.zxyz, r9.yzxy
    r10.xyz = ((r7.zxyz)*(r9.yzxy)).xyz;
    // 110: mad r10.xyz, r7.yzxy, r9.zxyz, -r10.xyzx
    r10.xyz = ((r7.yzxy)*(r9.zxyz)+(-(r10.xyzx))).xyz;
    // 111: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 112: dp3 r11.y, r10.xyzx, r5.xyzx
    r11.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 113: dp3 r11.x, r9.xyzx, r5.xyzx
    r11.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 114: dp2 r8.z, r11.xyxx, cb0[12].xyxx
    r8.z = (dot((r11.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 115: mul r2.xy, cb0[12].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((source[12].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 116: dp2 r8.x, r11.xyxx, r2.xyxx
    r8.x = (dot((r11.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 117: mov r8.w, l(1.000000)
    r8.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 118: dp4 r12.x, cb0[13].xyzw, r8.xyzw
    r12.x = (dot((source[13].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).x;
    // 119: dp4 r12.y, cb0[14].xyzw, r8.xyzw
    r12.y = (dot((source[14].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).y;
    // 120: dp4 r12.z, cb0[15].xyzw, r8.xyzw
    r12.z = (dot((source[15].xyzw).xyzw,(r8.xyzw).xyzw).xxxx).z;
    // 121: mul r13.xyzw, r8.yzzx, r8.xyzz
    r13.xyzw = ((r8.yzzx)*(r8.xyzz)).xyzw;
    // 122: dp4 r14.x, cb0[16].xyzw, r13.xyzw
    r14.x = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 123: dp4 r14.y, cb0[17].xyzw, r13.xyzw
    r14.y = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 124: dp4 r14.z, cb0[18].xyzw, r13.xyzw
    r14.z = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 125: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 126: mul r0.w, r8.y, r8.y
    r0.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 127: mov r11.z, r8.y
    r11.z = (r8.yyyy).z;
    // 128: mad r0.w, r8.x, r8.x, -r0.w
    r0.w = ((r8.xxxx)*(r8.xxxx)+(-(r0.wwww))).w;
    // 129: mad r8.xyz, cb0[19].xyzx, r0.wwww, r12.xyzx
    r8.xyz = ((source[19].xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 130: max r8.xyz, r8.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r8.xyz = (max(r8.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 131: mul r8.xyz, r8.xyzx, cb0[11].xyzx
    r8.xyz = ((r8.xyzx)*(source[11].xyzx)).xyz;
    // 132: mad r8.xyz, r8.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[11].wwww
    r8.xyz = ((r8.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[11].wwww)).xyz;
    // 133: mov_sat r1.w, cb0[9].x
    r1.w = (saturate(source[9].xxxx)).w;
    // 134: mad r12.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r12.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 135: mul r0.w, r1.w, l(0.080000)
    r0.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 136: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 137: mad r12.xyz, r4.wwww, r12.xyzx, r0.wwww
    r12.xyz = ((r4.wwww)*(r12.xyzx)+(r0.wwww)).xyz;
    // 138: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 139: log r1.w, |r2.w|
    r1.w = (log2(abs(r2.wwww))).w;
    // 140: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 141: mul r1.w, r1.w, cb0[9].w
    r1.w = ((r1.wwww)*(source[9].wwww)).w;
    // 142: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 143: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 144: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 145: min r4.z, r1.w, l(1.000000)
    r4.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 146: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 147: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 148: mul r13.xyz, r1.wwww, v5.xyzx
    r13.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 149: dp3 r1.w, r5.xyzx, r13.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 150: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 151: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 152: deriv_rtx_coarse r4.x, r1.w
    r4.x = (ddx_coarse(r1.wwww)).x;
    // 153: deriv_rty_coarse r4.y, r1.w
    r4.y = (ddy_coarse(r1.wwww)).y;
    // 154: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 155: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 156: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 157: mad_sat r4.y, r2.w, l(0.300000), r4.z
    r4.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r4.zzzz))).y;
    // 158: mov o2.zw, r4.zzzw
    output.targets[2].zw = (r4.zzzw).zw;
    // 159: add r2.w, -r4.y, l(1.000000)
    r2.w = ((-(r4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 160: max r14.xyz, r12.xyzx, r2.wwww
    r14.xyz = (max(r12.xyzx,r2.wwww)).xyz;
    // 161: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 162: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 163: add r0.w, r5.z, l(1.000000)
    r0.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: add_sat r4.x, -r0.w, r1.w
    r4.x = (saturate((-(r0.wwww))+(r1.wwww))).x;
    // 166: sample_indexable(texture2d)(float,float,float,float) r15.xy, r4.xyxx, t6.xyzw, s7
    r15.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 167: add r0.w, r2.z, r4.x
    r0.w = ((r2.zzzz)+(r4.xxxx)).w;
    // 168: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 169: mul r16.xyz, r12.xyzx, r15.yyyy
    r16.xyz = ((r12.xyzx)*(r15.yyyy)).xyz;
    // 170: mad r14.xyz, r14.xyzx, r15.xxxx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xxxx)+(r16.xyzx)).xyz;
    // 171: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.y
    r1.w = r15.y != 0.f ? 1.f / r15.y : 0.f;
    // 172: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 173: mad r15.xyz, r12.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r12.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 174: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: mad r12.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 176: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 178: mul r8.xyz, r8.xyzx, r16.xyzx
    r8.xyz = ((r8.xyzx)*(r16.xyzx)).xyz;
    // 179: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 180: mad r3.xyz, -r3.xyzx, r4.wwww, r3.xyzx
    r3.xyz = ((-(r3.xyzx))*(r4.wwww)+(r3.xyzx)).xyz;
    // 181: mul r1.w, r4.y, l(5.000000)
    r1.w = ((r4.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 182: mul r2.w, r4.y, r4.y
    r2.w = ((r4.yyyy)*(r4.yyyy)).w;
    // 183: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 184: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 185: add r0.w, r2.z, r0.w
    r0.w = ((r2.zzzz)+(r0.wwww)).w;
    // 186: mov o5.y, r2.z
    output.targets[5].y = (r2.zzzz).y;
    // 187: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 188: dp3 r4.x, r9.xyzx, r5.xyzx
    r4.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 189: dp3 r4.y, r10.xyzx, r5.xyzx
    r4.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 190: dp2 r2.x, r4.xyxx, r2.xyxx
    r2.x = (dot((r4.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 191: dp2 r2.z, r4.xyxx, cb0[12].xyxx
    r2.z = (dot((r4.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 192: dp3 r2.y, r7.xyzx, r5.xyzx
    r2.y = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 193: dp3 r2.w, r6.xyzx, r5.xyzx
    r2.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 194: mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 195: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 196: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t7.xyzw, s6, r1.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 197: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 198: mul r2.xyz, r2.xyzx, cb0[11].xyzx
    r2.xyz = ((r2.xyzx)*(source[11].xyzx)).xyz;
    // 199: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[11].wwww)).xyz;
    // 200: mad r1.w, r0.w, r12.x, r12.y
    r1.w = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).w;
    // 201: mad r1.w, r1.w, r0.w, r12.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r12.zzzz)).w;
    // 202: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 203: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 204: mul r4.yzw, r4.yyyy, cb0[22].xxyz
    r4.yzw = ((r4.yyyy)*(source[22].xxyz)).yzw;
    // 205: mad r4.xyz, cb0[21].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[21].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 206: mul r4.xyz, r4.xyzx, cb0[23].wwww
    r4.xyz = ((r4.xyzx)*(source[23].wwww)).xyz;
    // 207: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 208: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 209: mad r3.xyz, r2.xyzx, r14.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r14.xyzx)+(r3.xyzx)).xyz;
    // 210: mul r2.xyz, r14.xyzx, r2.xyzx
    r2.xyz = ((r14.xyzx)*(r2.xyzx)).xyz;
    // 211: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 212: dp3 r0.x, r0.xyzx, r13.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 213: add r0.y, -|r13.z|, l(1.000000)
    r0.y = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 214: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 215: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 216: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 217: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 218: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 219: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 220: mul r0.xzw, r0.xxxx, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[2].xxyz)).xzw;
    // 221: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 222: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 223: add r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)+(r0.xyzx)).xyz;
    // 224: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 225: mad o0.xyz, r1.xyzx, cb0[23].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[23].xyzx)+(r0.xyzx)).xyz;
    // 226: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 227: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 228: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 229: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 230: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 231: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 232: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 233: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 234: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 235: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 236: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 237: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 238: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 239: ftou r0.x, cb0[20].z
    r0.x = (asfloat((uint4)(source[20].zzzz))).x;
    // 240: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 241: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 242: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 243: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 244: ret
    return output;
}

// source.character.static-map-native-1125.v1 / source program e5e535f1fd4e27478aebb241bd0e43e8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1125(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
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
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 12: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: mul r1.zw, r1.zzzw, cb0[6].zzzz
    r1.zw = ((r1.zzzw)*(source[6].zzzz)).zw;
    // 14: mad r1.xy, cb0[6].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
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
    // 22: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 28: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 40: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 41: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 42: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 43: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 44: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 45: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 46: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 47: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 48: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 50: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 51: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 52: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 53: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 54: add r3.xyz, -r4.xyzx, r0.yyyy
    r3.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 55: mad r3.xyz, cb0[8].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[8].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[5].xyzx, cb0[9].xxxx
    r4.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 57: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 58: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mad r4.xyz, -r4.xyzx, r7.xyzx, r0.yyyy
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 60: mad r4.xyz, cb0[9].zzzz, r4.xyzx, r8.xyzx
    r4.xyz = ((source[9].zzzz)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 61: mul r7.xyz, cb0[4].xyzx, cb0[8].wwww
    r7.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 62: mad r4.xyz, -r3.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 64: mad r3.xyz, r0.xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r4.xyz, r3.xyzx, cb0[9].wwww
    r4.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // 67: mad r3.xyz, cb0[10].xxxx, r3.xyzx, -r4.xyzx
    r3.xyz = ((source[10].xxxx)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 69: mul r0.y, r7.z, cb0[10].y
    r0.y = ((r7.zzzz)*(source[10].yyyy)).y;
    // 70: mul r7.xy, r7.yxyy, cb0[11].ywyy
    r7.xy = ((r7.yxyy)*(source[11].ywyy)).xy;
    // 71: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 72: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: mul r1.w, r1.w, cb0[10].z
    r1.w = ((r1.wwww)*(source[10].zzzz)).w;
    // 74: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 75: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 76: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 78: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 79: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 81: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 83: mul r4.xy, r0.zwzz, cb0[7].xxxx
    r4.xy = ((r0.zwzz)*(source[7].xxxx)).xy;
    // 84: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 85: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 86: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 87: add r4.z, r0.y, l(0.000010)
    r4.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: add r0.yzw, -r1.xxyz, r4.xxyz
    r0.yzw = ((-(r1.xxyz))+(r4.xxyz)).yzw;
    // 89: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 90: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 93: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 97: mad r4.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 98: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 99: mul r4.yzw, r4.yyyy, cb0[24].xxyz
    r4.yzw = ((r4.yyyy)*(source[24].xxyz)).yzw;
    // 100: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r4.yzwy)).xyz;
    // 101: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 102: mul r8.xyz, r3.xyzx, r4.xyzx
    r8.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 103: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 104: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 105: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 106: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 107: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t9.xyzw, s6
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 108: mul r10.xyz, r10.xyzx, cb0[27].xyzx
    r10.xyz = ((r10.xyzx)*(source[27].xyzx)).xyz;
    // 109: dp3 r0.w, r10.xyzx, r9.xyzx
    r0.w = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 110: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s6
    r9.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 111: mul r9.xyz, r9.xyzx, cb0[26].xyzx
    r9.xyz = ((r9.xyzx)*(source[26].xyzx)).xyz;
    // 112: mul r11.xyz, r0.wwww, r9.xyzx
    r11.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 113: mad r8.xyz, r3.xyzx, r11.xyzx, r8.xyzx
    r8.xyz = ((r3.xyzx)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 114: mad r11.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 115: mad r12.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r12.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 116: mad r13.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r13.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 117: log r14.xy, |r7.xyxx|
    r14.xy = (log2(abs(r7.xyxx))).xy;
    // 118: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 119: mul r1.w, r14.y, cb0[12].x
    r1.w = ((r14.yyyy)*(source[12].xxxx)).w;
    // 120: mul r2.w, r14.x, cb0[11].z
    r2.w = ((r14.xxxx)*(source[11].zzzz)).w;
    // 121: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 122: movc r2.w, r7.x, l(0), r2.w
    r2.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 123: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 124: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 125: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 126: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: movc r1.w, r7.y, l(0), r1.w
    r1.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 128: mad r12.xyz, r1.wwww, r12.xyzx, r13.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)+(r13.xyzx)).xyz;
    // 129: mad r11.xyz, r12.xyzx, r1.wwww, r11.xyzx
    r11.xyz = ((r12.xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 130: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 131: max r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = (max(r1.wwww,r11.xyzx)).xyz;
    // 132: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 133: dp3 r11.x, r2.xyzx, r1.xyzx
    r11.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 134: dp3 r11.y, r6.xyzx, r1.xyzx
    r11.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 135: dp2 r12.z, r11.xyxx, cb0[14].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 136: dp3 r12.y, r5.xyzx, r1.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 137: mul r7.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 138: dp2 r12.x, r11.xyxx, r7.xyxx
    r12.x = (dot((r11.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 139: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 140: dp4 r13.x, cb0[15].xyzw, r12.xyzw
    r13.x = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 141: dp4 r13.y, cb0[16].xyzw, r12.xyzw
    r13.y = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 142: dp4 r13.z, cb0[17].xyzw, r12.xyzw
    r13.z = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 143: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 144: dp4 r15.x, cb0[18].xyzw, r14.xyzw
    r15.x = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 145: dp4 r15.y, cb0[19].xyzw, r14.xyzw
    r15.y = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 146: dp4 r15.z, cb0[20].xyzw, r14.xyzw
    r15.z = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 147: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 148: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 149: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 150: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 151: mad r12.xyz, cb0[21].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[21].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 152: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 153: mul r12.xyz, r12.xyzx, cb0[13].xyzx
    r12.xyz = ((r12.xyzx)*(source[13].xyzx)).xyz;
    // 154: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 155: mov_sat r3.w, cb0[10].w
    r3.w = (saturate(source[10].wwww)).w;
    // 156: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 157: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 158: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 159: mad r13.xyz, r7.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 160: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 161: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 162: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 163: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 164: dp3 r3.w, r1.xyzx, r14.xyzx
    r3.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 165: mul r1.xyz, r1.xyzx, r3.wwww
    r1.xyz = ((r1.xyzx)*(r3.wwww)).xyz;
    // 166: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 167: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 168: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 169: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 176: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 177: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 180: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 181: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 182: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
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
    // 193: mul r8.xyz, r8.xyzx, r12.xyzx
    r8.xyz = ((r8.xyzx)*(r12.xyzx)).xyz;
    // 194: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 195: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 196: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 197: dp3 r2.y, r6.xyzx, r1.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 198: dp2 r6.x, r2.xyxx, r7.xyxx
    r6.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 199: dp2 r6.z, r2.xyxx, cb0[14].xyxx
    r6.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 200: mul r2.x, r15.y, l(5.000000)
    r2.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 201: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 202: mul r2.y, r2.w, r2.y
    r2.y = ((r2.wwww)*(r2.yyyy)).y;
    // 203: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 204: add r2.y, r1.w, r2.y
    r2.y = ((r1.wwww)+(r2.yyyy)).y;
    // 205: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 206: add_sat r1.w, r2.y, l(-1.000000)
    r1.w = (saturate((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 207: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r6.xyzx, t7.xyzw, s7, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 211: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 212: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 213: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 214: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 215: mul r1.xyz, r5.xyzx, r5.xyzx
    r1.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 216: dp3 r1.x, r10.xyzx, r1.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 217: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 218: mad r0.w, r7.z, r0.w, r1.x
    r0.w = ((r7.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 219: mad r1.xyz, r9.xyzx, r0.wwww, r4.xyzx
    r1.xyz = ((r9.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 220: mul r4.xyz, r0.wwww, r9.xyzx
    r4.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 221: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 222: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 223: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 224: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 225: mul r5.xyz, r0.wwww, r1.xyzx
    r5.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 226: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 227: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 228: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mul r1.xyz, r2.xyzx, r5.xyzx
    r1.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 230: mad r2.xyz, r1.xyzx, r15.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r8.xyzx)).xyz;
    // 231: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 232: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 233: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 234: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 235: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 236: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 237: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 238: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 239: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 240: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 241: mul r1.xyz, r0.xxxx, cb0[3].xyzx
    r1.xyz = ((r0.xxxx)*(source[3].xyzx)).xyz;
    // 242: movc r0.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 243: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 244: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 245: mad o0.xyz, r3.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 246: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 247: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 248: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 249: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 250: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 251: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 252: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 253: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 254: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 255: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 256: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 257: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 258: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 259: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 260: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 261: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
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

// source.character.static-map-native-1125.v1 / source program 6d4545882af8144caccb2520c86415ee
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1125(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1125(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
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
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 12: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: mul r1.zw, r1.zzzw, cb0[6].zzzz
    r1.zw = ((r1.zzzw)*(source[6].zzzz)).zw;
    // 14: mad r1.xy, cb0[6].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
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
    // 22: dp3 r4.x, r2.xyzx, r1.xyzx
    r4.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 23: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 24: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 25: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 26: dp3 r4.z, r5.xyzx, r1.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 27: mul r6.xyz, r2.yzxy, r5.zxyz
    r6.xyz = ((r2.yzxy)*(r5.zxyz)).xyz;
    // 28: mad r6.xyz, r5.yzxy, r2.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r2.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 33: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 34: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 37: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r3.zwzz, t2.zwxy, s2, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 40: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 41: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 42: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 43: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 44: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 45: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 46: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 47: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 48: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 50: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 51: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 52: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 53: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 54: add r3.xyz, -r4.xyzx, r0.yyyy
    r3.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 55: mad r3.xyz, cb0[8].zzzz, r3.xyzx, r4.xyzx
    r3.xyz = ((source[8].zzzz)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 56: mul r4.xyz, cb0[5].xyzx, cb0[9].xxxx
    r4.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 57: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 58: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mad r4.xyz, -r4.xyzx, r7.xyzx, r0.yyyy
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 60: mad r4.xyz, cb0[9].zzzz, r4.xyzx, r8.xyzx
    r4.xyz = ((source[9].zzzz)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 61: mul r7.xyz, cb0[4].xyzx, cb0[8].wwww
    r7.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 62: mad r4.xyz, -r3.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 63: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 64: mad r3.xyz, r0.xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 65: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 66: mul r4.xyz, r3.xyzx, cb0[9].wwww
    r4.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // 67: mad r3.xyz, cb0[10].xxxx, r3.xyzx, -r4.xyzx
    r3.xyz = ((source[10].xxxx)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 69: mul r0.y, r7.z, cb0[10].y
    r0.y = ((r7.zzzz)*(source[10].yyyy)).y;
    // 70: mul r7.xy, r7.yxyy, cb0[11].ywyy
    r7.xy = ((r7.yxyy)*(source[11].ywyy)).xy;
    // 71: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 72: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 73: mul r1.w, r1.w, cb0[10].z
    r1.w = ((r1.wwww)*(source[10].zzzz)).w;
    // 74: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 75: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 76: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 78: mad r3.xyz, r1.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 79: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 81: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 83: mul r4.xy, r0.zwzz, cb0[7].xxxx
    r4.xy = ((r0.zwzz)*(source[7].xxxx)).xy;
    // 84: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 85: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 86: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 87: add r4.z, r0.y, l(0.000010)
    r4.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: add r0.yzw, -r1.xxyz, r4.xxyz
    r0.yzw = ((-(r1.xxyz))+(r4.xxyz)).yzw;
    // 89: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 90: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 93: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 97: mad r8.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 98: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 99: mul r8.yzw, r8.yyyy, cb0[24].xxyz
    r8.yzw = ((r8.yyyy)*(source[24].xxyz)).yzw;
    // 100: mad r8.xyz, r8.xxxx, cb0[23].xyzx, r8.yzwy
    r8.xyz = ((r8.xxxx)*(source[23].xyzx)+(r8.yzwy)).xyz;
    // 101: mul r8.xyz, r8.xyzx, cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(source[25].wwww)).xyz;
    // 102: mul r8.xyz, r3.xyzx, r8.xyzx
    r8.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 103: mad r9.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 104: mad r10.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 105: mad r11.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 106: log r12.xy, |r7.xyxx|
    r12.xy = (log2(abs(r7.xyxx))).xy;
    // 107: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 108: mul r0.w, r12.y, cb0[12].x
    r0.w = ((r12.yyyy)*(source[12].xxxx)).w;
    // 109: mul r1.w, r12.x, cb0[11].z
    r1.w = ((r12.xxxx)*(source[11].zzzz)).w;
    // 110: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 111: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 112: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 113: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 115: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: movc r0.w, r7.y, l(0), r0.w
    r0.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 117: mad r10.xyz, r0.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 118: mad r9.xyz, r10.xyzx, r0.wwww, r9.xyzx
    r9.xyz = ((r10.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 119: mul r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 120: max r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = (max(r0.wwww,r9.xyzx)).xyz;
    // 121: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 122: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 123: dp3 r9.y, r6.xyzx, r1.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 124: dp2 r10.z, r9.xyxx, cb0[14].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 125: dp3 r10.y, r5.xyzx, r1.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 126: mul r7.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 127: dp2 r10.x, r9.xyxx, r7.xyxx
    r10.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 128: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r11.x, cb0[15].xyzw, r10.xyzw
    r11.x = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 130: dp4 r11.y, cb0[16].xyzw, r10.xyzw
    r11.y = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 131: dp4 r11.z, cb0[17].xyzw, r10.xyzw
    r11.z = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 132: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 133: dp4 r13.x, cb0[18].xyzw, r12.xyzw
    r13.x = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[19].xyzw, r12.xyzw
    r13.y = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[20].xyzw, r12.xyzw
    r13.z = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 137: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 138: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 139: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 140: mad r10.xyz, cb0[21].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[21].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 141: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 143: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 144: mov_sat r3.w, cb0[10].w
    r3.w = (saturate(source[10].wwww)).w;
    // 145: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 146: mul r1.w, r3.w, l(0.080000)
    r1.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 148: mad r11.xyz, r7.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r7.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 149: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 151: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 152: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 153: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 154: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 155: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 156: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 157: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 158: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 166: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 167: add r1.w, r1.z, l(1.000000)
    r1.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 170: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 171: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
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
    // 183: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 184: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 185: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 186: dp3 r2.y, r6.xyzx, r1.xyzx
    r2.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 187: dp2 r6.x, r2.xyxx, r7.xyxx
    r6.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 188: dp2 r6.z, r2.xyxx, cb0[14].xyxx
    r6.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 189: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 190: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 191: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 192: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 193: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 194: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 195: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 196: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 197: dp3 r1.x, r4.xyzx, r1.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 198: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r6.xyzx, t7.xyzw, s6, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 202: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 203: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 204: mad r1.z, r0.w, r11.x, r11.y
    r1.z = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).z;
    // 205: mad r1.z, r1.z, r0.w, r11.z
    r1.z = ((r1.zzzz)*(r0.wwww)+(r11.zzzz)).z;
    // 206: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 207: max r0.w, r0.w, r1.z
    r0.w = (max(r0.wwww,r1.zzzz)).w;
    // 208: mul r1.yzw, r1.yyyy, cb0[24].xxyz
    r1.yzw = ((r1.yyyy)*(source[24].xxyz)).yzw;
    // 209: mad r1.xyz, cb0[23].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[23].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 210: mul r1.xyz, r1.xyzx, cb0[25].wwww
    r1.xyz = ((r1.xyzx)*(source[25].wwww)).xyz;
    // 211: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 212: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 213: mad r2.xyz, r1.xyzx, r13.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r13.xzwx)+(r8.xyzx)).xyz;
    // 214: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 215: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
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
    // 229: mad o0.xyz, r3.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
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
    // 243: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
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

// source.character.static-map-native-1126.v1 / source program 9576c0304ff4a94ab8ff34c250abee8a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1126(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[27]=1.f;
    source[28]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xyz, cb0[7].xyzx, cb0[10].zzzz
    r0.xyz = ((source[7].xyzx)*(source[10].zzzz)).xyz;
    // 2: mul r1.xyz, cb0[6].xyzx, cb0[10].yyyy
    r1.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 3: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 5: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 6: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 7: mad r4.xyz, cb0[10].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[10].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r3.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 12: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mul r0.w, r3.z, cb0[10].w
    r0.w = ((r3.zzzz)*(source[10].wwww)).w;
    // 14: mul r2.zw, r3.yyyx, cb0[12].yyyw
    r2.zw = ((r3.yyyx)*(source[12].yyyw)).zw;
    // 15: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 16: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 18: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 19: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 20: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 22: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xyzx, cb0[11].yyyy
    r1.xyz = ((r0.xyzx)*(source[11].yyyy)).xyz;
    // 24: mad r0.xyz, cb0[11].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[11].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 25: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 26: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 28: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 29: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 30: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 31: mul r1.xyz, r1.xxxx, v6.xyzx
    r1.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 32: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 33: mul r2.xy, r2.xyxx, cb0[8].wwww
    r2.xy = ((r2.xyxx)*(source[8].wwww)).xy;
    // 34: mul r4.xy, r2.xyxx, v2.wwww
    r4.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 42: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 43: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 44: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 45: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 46: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 47: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 48: mul r1.yzw, r1.yyyy, cb0[25].xxyz
    r1.yzw = ((r1.yyyy)*(source[25].xxyz)).yzw;
    // 49: mad r1.xyz, r1.xxxx, cb0[24].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[24].xyzx)+(r1.yzwy)).xyz;
    // 50: mul r1.xyz, r1.xyzx, cb0[26].wwww
    r1.xyz = ((r1.xyzx)*(source[26].wwww)).xyz;
    // 51: mul r6.xyz, r0.xyzx, r1.xyzx
    r6.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 52: dp2_sat r7.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 53: dp3_sat r7.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 54: dp3_sat r7.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 55: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 56: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s4
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 57: mul r8.xyz, r8.xyzx, cb0[28].xyzx
    r8.xyz = ((r8.xyzx)*(source[28].xyzx)).xyz;
    // 58: dp3 r1.w, r8.xyzx, r7.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 59: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t6.xyzw, s4
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 60: mul r7.xyz, r7.xyzx, cb0[27].xyzx
    r7.xyz = ((r7.xyzx)*(source[27].xyzx)).xyz;
    // 61: mul r9.xyz, r1.wwww, r7.xyzx
    r9.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 62: mad r6.xyz, r0.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 63: mad r9.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 64: mad r10.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 65: mad r11.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 66: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 67: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 68: mul r2.y, r2.y, cb0[13].x
    r2.y = ((r2.yyyy)*(source[13].xxxx)).y;
    // 69: mul r2.x, r2.x, cb0[12].z
    r2.x = ((r2.xxxx)*(source[12].zzzz)).x;
    // 70: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 71: movc r2.x, r2.z, l(0), r2.x
    r2.x = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 72: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 73: min r3.z, r2.x, l(1.000000)
    r3.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: exp r2.x, r2.y
    r2.x = (exp2(r2.yyyy)).x;
    // 75: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 76: movc r2.x, r2.w, l(0), r2.x
    r2.x = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 77: mad r2.yzw, r2.xxxx, r10.xxyz, r11.xxyz
    r2.yzw = ((r2.xxxx)*(r10.xxyz)+(r11.xxyz)).yzw;
    // 78: mad r2.yzw, r2.yyzw, r2.xxxx, r9.xxyz
    r2.yzw = ((r2.yyzw)*(r2.xxxx)+(r9.xxyz)).yzw;
    // 79: mul r2.yzw, r2.xxxx, r2.yyzw
    r2.yzw = ((r2.xxxx)*(r2.yyzw)).yzw;
    // 80: max r2.yzw, r2.yyzw, r2.xxxx
    r2.yzw = (max(r2.yyzw,r2.xxxx)).yzw;
    // 81: mul r2.yzw, r2.yyzw, r6.xxyz
    r2.yzw = ((r2.yyzw)*(r6.xxyz)).yzw;
    // 82: dp3 r3.x, v1.xyzx, v1.xyzx
    r3.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 83: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 84: mul r6.xyz, r3.xxxx, v1.xyzx
    r6.xyz = ((r3.xxxx)*(v1.xyzx)).xyz;
    // 85: dp3 r3.x, v0.xyzx, v0.xyzx
    r3.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 86: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 87: mul r9.xyz, r3.xxxx, v0.xyzx
    r9.xyz = ((r3.xxxx)*(v0.xyzx)).xyz;
    // 88: mul r10.xyz, r6.zxyz, r9.yzxy
    r10.xyz = ((r6.zxyz)*(r9.yzxy)).xyz;
    // 89: mad r10.xyz, r6.yzxy, r9.zxyz, -r10.xyzx
    r10.xyz = ((r6.yzxy)*(r9.zxyz)+(-(r10.xyzx))).xyz;
    // 90: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 91: dp3 r11.y, r10.xyzx, r5.xyzx
    r11.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 92: dp3 r11.x, r9.xyzx, r5.xyzx
    r11.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 93: dp2 r12.z, r11.xyxx, cb0[15].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 94: mul r3.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 95: dp2 r12.x, r11.xyxx, r3.xyxx
    r12.x = (dot((r11.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 96: dp3 r12.y, r6.xyzx, r5.xyzx
    r12.y = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 97: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 98: dp4 r13.x, cb0[16].xyzw, r12.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 99: dp4 r13.y, cb0[17].xyzw, r12.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 100: dp4 r13.z, cb0[18].xyzw, r12.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 101: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 102: dp4 r15.x, cb0[19].xyzw, r14.xyzw
    r15.x = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 103: dp4 r15.y, cb0[20].xyzw, r14.xyzw
    r15.y = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 104: dp4 r15.z, cb0[21].xyzw, r14.xyzw
    r15.z = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 105: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 106: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 107: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 108: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 109: mad r12.xyz, cb0[22].xyzx, r4.wwww, r13.xyzx
    r12.xyz = ((source[22].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 110: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 111: mul r12.xyz, r12.xyzx, cb0[14].xyzx
    r12.xyz = ((r12.xyzx)*(source[14].xyzx)).xyz;
    // 112: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 113: mov_sat r0.w, cb0[11].w
    r0.w = (saturate(source[11].wwww)).w;
    // 114: mad r13.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r13.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 115: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 116: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 117: mad r13.xyz, r3.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r3.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 118: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 119: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 120: mul r14.xyz, r0.wwww, v5.xyzx
    r14.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 121: dp3 r0.w, r5.xyzx, r14.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 122: mul r5.xyz, r0.wwww, r5.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 123: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 124: deriv_rtx_coarse r15.x, r0.w
    r15.x = (ddx_coarse(r0.wwww)).x;
    // 125: deriv_rty_coarse r15.y, r0.w
    r15.y = (ddy_coarse(r0.wwww)).y;
    // 126: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 128: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 129: mad_sat r15.y, r4.w, l(0.300000), r3.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 130: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 132: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 133: mul_sat r4.w, r13.y, l(50.000000)
    r4.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 134: mul r16.xyz, r4.wwww, r16.xyzx
    r16.xyz = ((r4.wwww)*(r16.xyzx)).xyz;
    // 135: add r4.w, r5.z, l(1.000000)
    r4.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 136: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: add_sat r15.x, r0.w, -r4.w
    r15.x = (saturate((r0.wwww)+(-(r4.wwww)))).x;
    // 138: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t4.zwxy, s6
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 139: add r0.w, r2.x, r15.x
    r0.w = ((r2.xxxx)+(r15.xxxx)).w;
    // 140: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 141: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 142: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 143: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r4.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 144: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 145: mad r15.xzw, r13.xxyz, r4.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r4.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 146: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 147: mad r13.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 148: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 149: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 150: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 151: mul r2.yzw, r2.yyzw, r12.xxyz
    r2.yzw = ((r2.yyzw)*(r12.xxyz)).yzw;
    // 152: mad r2.yzw, -r2.yyzw, r3.wwww, r2.yyzw
    r2.yzw = ((-(r2.yyzw))*(r3.wwww)+(r2.yyzw)).yzw;
    // 153: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 154: dp2_sat r12.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r12.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 155: dp3_sat r12.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r12.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 156: dp3_sat r12.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r12.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 157: mul r12.xyz, r12.xyzx, r12.xyzx
    r12.xyz = ((r12.xyzx)*(r12.xyzx)).xyz;
    // 158: dp3 r3.w, r8.xyzx, r12.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 159: add r1.w, r1.w, -r3.w
    r1.w = ((r1.wwww)+(-(r3.wwww))).w;
    // 160: mad r1.w, r3.z, r1.w, r3.w
    r1.w = ((r3.zzzz)*(r1.wwww)+(r3.wwww)).w;
    // 161: mad r1.xyz, r7.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r7.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 162: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 163: mul r1.w, r15.y, r15.y
    r1.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 164: mul r3.z, r15.y, l(5.000000)
    r3.z = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 165: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 166: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 167: add r0.w, r2.x, r0.w
    r0.w = ((r2.xxxx)+(r0.wwww)).w;
    // 168: mov o5.y, r2.x
    output.targets[5].y = (r2.xxxx).y;
    // 169: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 170: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 171: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 172: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 173: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 174: mul r8.xyz, r0.wwww, r1.xyzx
    r8.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 175: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 176: div r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)/(r1.xyzx)).xyz;
    // 177: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: dp3 r1.x, r9.xyzx, r5.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 179: dp3 r1.y, r10.xyzx, r5.xyzx
    r1.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 180: dp3 r5.y, r6.xyzx, r5.xyzx
    r5.y = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 181: dp2 r5.x, r1.xyxx, r3.xyxx
    r5.x = (dot((r1.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 182: dp2 r5.z, r1.xyxx, cb0[15].xyxx
    r5.z = (dot((r1.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 183: sample_l_indexable(texturecube)(float,float,float,float) r1.xyzw, r5.xyzx, t5.xyzw, s5, r3.z
    r1.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r3.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 184: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 185: mul r1.xyz, r1.xyzx, cb0[14].xyzx
    r1.xyz = ((r1.xyzx)*(source[14].xyzx)).xyz;
    // 186: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 187: mul r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)*(r1.xyzx)).xyz;
    // 188: mad r2.xyz, r1.xyzx, r15.xzwx, r2.yzwy
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r2.yzwy)).xyz;
    // 189: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 190: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 191: dp3 r1.x, r4.xyzx, r14.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 192: add r1.y, -|r14.z|, l(1.000000)
    r1.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 193: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 194: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 195: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 196: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 197: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 198: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 199: mul r1.xzw, r1.xxxx, cb0[3].xxyz
    r1.xzw = ((r1.xxxx)*(source[3].xxyz)).xzw;
    // 200: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 201: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 202: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 203: mul r4.xyz, cb0[5].xyzx, cb0[9].zzzz
    r4.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 204: mad r1.xyz, r3.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 205: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 206: add r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)+(r1.xyzx)).xyz;
    // 207: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 208: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 209: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 210: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 211: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 212: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 213: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 214: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 215: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 216: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 217: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 218: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 219: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 220: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 221: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 222: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 223: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 224: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 225: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 226: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 227: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 228: ret
    return output;
}

// source.character.static-map-native-1126.v1 / source program 32f29a300e95554c97d6ea6b3e1126a4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1126(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1126(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: mul r0.xyz, cb0[7].xyzx, cb0[10].zzzz
    r0.xyz = ((source[7].xyzx)*(source[10].zzzz)).xyz;
    // 2: mul r1.xyz, cb0[6].xyzx, cb0[10].yyyy
    r1.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 3: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 5: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 6: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 7: mad r4.xyz, cb0[10].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[10].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r3.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 12: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 13: mul r0.w, r3.z, cb0[10].w
    r0.w = ((r3.zzzz)*(source[10].wwww)).w;
    // 14: mul r2.zw, r3.yyyx, cb0[12].yyyw
    r2.zw = ((r3.yyyx)*(source[12].yyyw)).zw;
    // 15: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 16: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 18: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 19: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 20: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 22: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xyzx, cb0[11].yyyy
    r1.xyz = ((r0.xyzx)*(source[11].yyyy)).xyz;
    // 24: mad r0.xyz, cb0[11].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[11].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 25: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 26: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 28: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 29: mad r1.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 30: mad r4.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r4.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 31: mad r5.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r5.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 32: log r3.xy, |r2.zwzz|
    r3.xy = (log2(abs(r2.zwzz))).xy;
    // 33: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 34: mul r1.w, r3.y, cb0[13].x
    r1.w = ((r3.yyyy)*(source[13].xxxx)).w;
    // 35: mul r3.x, r3.x, cb0[12].z
    r3.x = ((r3.xxxx)*(source[12].zzzz)).x;
    // 36: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 37: movc r2.z, r2.z, l(0), r3.x
    r2.z = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).z;
    // 38: max r2.z, r2.z, cb0[0].x
    r2.z = (max(r2.zzzz,source[0].xxxx)).z;
    // 39: min r3.z, r2.z, l(1.000000)
    r3.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 40: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 41: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 43: mad r4.xyz, r1.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 44: mad r1.xyz, r4.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 45: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 46: max r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = (max(r1.xyzx,r1.wwww)).xyz;
    // 47: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 48: mul r2.xy, r2.xyxx, cb0[8].wwww
    r2.xy = ((r2.xyxx)*(source[8].wwww)).xy;
    // 49: mul r4.xy, r2.xyxx, v2.wwww
    r4.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 50: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 52: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 53: add r4.z, r2.x, l(0.000010)
    r4.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 54: dp3 r2.x, r4.xyzx, r4.xyzx
    r2.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 55: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 56: div r2.xyz, r4.xyzx, r2.xxxx
    r2.xyz = ((r4.xyzx)/(r2.xxxx)).xyz;
    // 57: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r4.xyz, r2.wwww, r2.xyzx
    r4.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 60: dp3 r2.w, v6.xyzx, v6.xyzx
    r2.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 61: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 62: mul r5.xyz, r2.wwww, v6.xyzx
    r5.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 63: dp3 r2.w, r5.xyzx, r4.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 64: mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 66: mul r6.xyz, r3.yyyy, cb0[25].xyzx
    r6.xyz = ((r3.yyyy)*(source[25].xyzx)).xyz;
    // 67: mad r6.xyz, r3.xxxx, cb0[24].xyzx, r6.xyzx
    r6.xyz = ((r3.xxxx)*(source[24].xyzx)+(r6.xyzx)).xyz;
    // 68: mul r6.xyz, r6.xyzx, cb0[26].wwww
    r6.xyz = ((r6.xyzx)*(source[26].wwww)).xyz;
    // 69: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 70: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 71: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 72: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 73: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 74: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 75: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 76: mul r7.xyz, r2.wwww, v0.xyzx
    r7.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 77: mul r8.xyz, r6.zxyz, r7.yzxy
    r8.xyz = ((r6.zxyz)*(r7.yzxy)).xyz;
    // 78: mad r8.xyz, r6.yzxy, r7.zxyz, -r8.xyzx
    r8.xyz = ((r6.yzxy)*(r7.zxyz)+(-(r8.xyzx))).xyz;
    // 79: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 81: dp3 r9.x, r7.xyzx, r4.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 82: dp2 r10.z, r9.xyxx, cb0[15].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 83: mul r3.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 84: dp2 r10.x, r9.xyxx, r3.xyxx
    r10.x = (dot((r9.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 85: dp3 r10.y, r6.xyzx, r4.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 86: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 87: dp4 r11.x, cb0[16].xyzw, r10.xyzw
    r11.x = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 88: dp4 r11.y, cb0[17].xyzw, r10.xyzw
    r11.y = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 89: dp4 r11.z, cb0[18].xyzw, r10.xyzw
    r11.z = (dot((source[18].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 90: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 91: dp4 r13.x, cb0[19].xyzw, r12.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 92: dp4 r13.y, cb0[20].xyzw, r12.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 93: dp4 r13.z, cb0[21].xyzw, r12.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 94: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 95: mul r2.w, r10.y, r10.y
    r2.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 96: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 97: mad r2.w, r10.x, r10.x, -r2.w
    r2.w = ((r10.xxxx)*(r10.xxxx)+(-(r2.wwww))).w;
    // 98: mad r10.xyz, cb0[22].xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((source[22].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 99: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 100: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 101: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 102: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 103: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 104: mul r11.xyz, r2.wwww, v5.xyzx
    r11.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 105: dp3 r2.w, r4.xyzx, r11.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 106: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 107: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 108: deriv_rtx_coarse r12.x, r2.w
    r12.x = (ddx_coarse(r2.wwww)).x;
    // 109: deriv_rty_coarse r12.y, r2.w
    r12.y = (ddy_coarse(r2.wwww)).y;
    // 110: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: dp2 r4.w, r12.xyxx, r12.xyxx
    r4.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 112: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 113: mad_sat r12.y, r4.w, l(0.300000), r3.z
    r12.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 114: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 115: add r3.z, -r12.y, l(1.000000)
    r3.z = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 116: mov_sat r0.w, cb0[11].w
    r0.w = (saturate(source[11].wwww)).w;
    // 117: mad r13.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r13.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 118: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 119: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 120: mad r13.xyz, r3.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r3.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 121: max r14.xyz, r3.zzzz, r13.xyzx
    r14.xyz = (max(r3.zzzz,r13.xyzx)).xyz;
    // 122: add r14.xyz, -r13.xyzx, r14.xyzx
    r14.xyz = ((-(r13.xyzx))+(r14.xyzx)).xyz;
    // 123: mul_sat r0.w, r13.y, l(50.000000)
    r0.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 124: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 125: add r0.w, r4.z, l(1.000000)
    r0.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 127: add_sat r12.x, -r0.w, r2.w
    r12.x = (saturate((-(r0.wwww))+(r2.wwww))).x;
    // 128: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t4.zwxy, s5
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 129: add r0.w, r1.w, r12.x
    r0.w = ((r1.wwww)+(r12.xxxx)).w;
    // 130: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 131: mul r15.xyz, r12.wwww, r13.xyzx
    r15.xyz = ((r12.wwww)*(r13.xyzx)).xyz;
    // 132: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 133: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 134: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 135: mad r12.xzw, r13.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r13.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 136: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: mad r13.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 138: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 139: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 140: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 141: mul r1.xyz, r1.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 142: mad r1.xyz, -r1.xyzx, r3.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r3.wwww)+(r1.xyzx)).xyz;
    // 143: mul r2.w, r12.y, l(5.000000)
    r2.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 144: mul r3.z, r12.y, r12.y
    r3.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 145: mul r0.w, r0.w, r3.z
    r0.w = ((r0.wwww)*(r3.zzzz)).w;
    // 146: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 147: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 148: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 149: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 150: dp3 r7.x, r7.xyzx, r4.xyzx
    r7.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 151: dp3 r7.y, r8.xyzx, r4.xyzx
    r7.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 152: dp2 r3.x, r7.xyxx, r3.xyxx
    r3.x = (dot((r7.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 153: dp2 r3.z, r7.xyxx, cb0[15].xyxx
    r3.z = (dot((r7.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 154: dp3 r3.y, r6.xyzx, r4.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 155: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 156: mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 157: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 158: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t5.xyzw, s4, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 159: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 160: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 161: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 162: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 163: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 164: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 165: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 166: mul r4.yzw, r4.yyyy, cb0[25].xxyz
    r4.yzw = ((r4.yyyy)*(source[25].xxyz)).yzw;
    // 167: mad r4.xyz, cb0[24].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[24].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 168: mul r4.xyz, r4.xyzx, cb0[26].wwww
    r4.xyz = ((r4.xyzx)*(source[26].wwww)).xyz;
    // 169: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 170: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 171: mad r1.xyz, r3.xyzx, r12.xzwx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r12.xzwx)+(r1.xyzx)).xyz;
    // 172: mul r3.xyz, r12.xzwx, r3.xyzx
    r3.xyz = ((r12.xzwx)*(r3.xyzx)).xyz;
    // 173: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 174: dp3 r0.w, r2.xyzx, r11.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 175: add r1.w, -|r11.z|, l(1.000000)
    r1.w = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 178: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 179: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 180: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 181: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 182: mul r2.xyz, r0.wwww, cb0[3].xyzx
    r2.xyz = ((r0.wwww)*(source[3].xyzx)).xyz;
    // 183: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 184: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 185: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 186: mul r4.xyz, cb0[5].xyzx, cb0[9].zzzz
    r4.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 187: mad r2.xyz, r3.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 188: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 189: add r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 190: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 191: mad o0.xyz, r0.xyzx, cb0[26].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[26].xyzx)+(r2.xyzx)).xyz;
    // 192: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 193: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 194: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 195: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 196: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 197: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 198: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 199: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 200: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 201: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 202: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 203: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 204: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 205: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 206: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 207: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 208: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 209: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 210: ret
    return output;
}

// source.character.static-map-native-1127.v1 / source program ac234f5b47ce6b4dad87a15622b97a95
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1127(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].wwww
    r0.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[9].zzzz
    r1.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 3: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 5: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 6: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 7: mad r4.xyz, cb0[9].yyyy, r4.xyzx, r3.xyzx
    r4.xyz = ((source[9].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r3.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 12: mul r0.w, r3.z, cb0[10].x
    r0.w = ((r3.zzzz)*(source[10].xxxx)).w;
    // 13: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 14: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 15: mul r1.w, r1.w, cb0[10].y
    r1.w = ((r1.wwww)*(source[10].yyyy)).w;
    // 16: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 17: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 18: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mul r0.w, r2.z, cb0[8].w
    r0.w = ((r2.zzzz)*(source[8].wwww)).w;
    // 23: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 24: mul r1.xy, r1.xyxx, cb0[7].wwww
    r1.xy = ((r1.xyxx)*(source[7].wwww)).xy;
    // 25: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 26: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 28: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 29: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 31: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 32: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 33: dp3 r2.x, r1.xyzx, r1.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 35: mul r2.xyz, r1.xyzx, r2.xxxx
    r2.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 36: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 37: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 38: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 39: dp3 r2.w, r2.xyzx, r4.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 40: mul r5.xyz, r2.wwww, r2.xyzx
    r5.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 41: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 42: add r6.xyz, r5.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r6.xyz = ((r5.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 43: mad r6.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 44: min r4.w, r6.z, l(1.000000)
    r4.w = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r6.xy, r6.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r6.xyz, r6.xyzx, cb0[4].xyzx
    r6.xyz = ((r6.xyzx)*(source[4].xyzx)).xyz;
    // 48: mad r6.xyz, cb0[8].zzzz, r6.xyzx, r6.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 49: add r6.xyz, r6.xyzx, -cb0[8].zzzz
    r6.xyz = ((r6.xyzx)+(-(source[8].zzzz))).xyz;
    // 50: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 51: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 52: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r0.xyz, r0.wwww, r7.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 54: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 55: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 56: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 57: mul r6.xyz, r0.xyzx, cb0[10].zzzz
    r6.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 58: mad r0.xyz, cb0[10].wwww, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[10].wwww)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 59: mad r0.xyz, r1.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 60: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 62: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 63: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 64: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 65: mul r6.xyz, r1.wwww, v6.xyzx
    r6.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 66: dp3 r1.w, r6.xyzx, r2.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 67: mad r6.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 68: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 69: mul r6.yzw, r6.yyyy, cb0[24].xxyz
    r6.yzw = ((r6.yyyy)*(source[24].xxyz)).yzw;
    // 70: mad r6.xyz, r6.xxxx, cb0[23].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[23].xyzx)+(r6.yzwy)).xyz;
    // 71: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 72: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 73: dp2_sat r8.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 74: dp3_sat r8.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 75: dp3_sat r8.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 76: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 77: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 78: mul r9.xyz, r9.xyzx, cb0[27].xyzx
    r9.xyz = ((r9.xyzx)*(source[27].xyzx)).xyz;
    // 79: dp3 r1.w, r9.xyzx, r8.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 80: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 81: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 82: mul r10.xyz, r1.wwww, r8.xyzx
    r10.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 83: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 84: mad r10.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 85: mad r11.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 86: mul r3.x, r3.x, cb0[12].x
    r3.x = ((r3.xxxx)*(source[12].xxxx)).x;
    // 87: mul r3.y, r3.y, cb0[11].z
    r3.y = ((r3.yyyy)*(source[11].zzzz)).y;
    // 88: log r5.w, |r3.x|
    r5.w = (log2(abs(r3.xxxx))).w;
    // 89: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 90: mul r5.w, r5.w, cb0[12].y
    r5.w = ((r5.wwww)*(source[12].yyyy)).w;
    // 91: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 92: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: movc r3.x, r3.x, l(0), r5.w
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).x;
    // 94: mad r10.xyz, r3.xxxx, r10.xyzx, r11.xyzx
    r10.xyz = ((r3.xxxx)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 95: mad r11.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 96: mad r10.xyz, r10.xyzx, r3.xxxx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r3.xxxx)+(r11.xyzx)).xyz;
    // 97: mul r10.xyz, r3.xxxx, r10.xyzx
    r10.xyz = ((r3.xxxx)*(r10.xyzx)).xyz;
    // 98: max r10.xyz, r3.xxxx, r10.xyzx
    r10.xyz = (max(r3.xxxx,r10.xyzx)).xyz;
    // 99: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 100: dp3 r5.w, v1.xyzx, v1.xyzx
    r5.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 101: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 102: mul r10.xyz, r5.wwww, v1.xyzx
    r10.xyz = ((r5.wwww)*(v1.xyzx)).xyz;
    // 103: dp3 r5.w, v0.xyzx, v0.xyzx
    r5.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 104: rsq r5.w, r5.w
    r5.w = (rsqrt(r5.wwww)).w;
    // 105: mul r11.xyz, r5.wwww, v0.xyzx
    r11.xyz = ((r5.wwww)*(v0.xyzx)).xyz;
    // 106: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 107: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 108: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 109: dp3 r13.y, r12.xyzx, r2.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 110: dp3 r12.y, r12.xyzx, r5.xyzx
    r12.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 111: dp3 r13.x, r11.xyzx, r2.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 112: dp3 r14.y, r10.xyzx, r2.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 113: dp3 r2.y, r10.xyzx, r5.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 114: dp3 r12.x, r11.xyzx, r5.xyzx
    r12.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 115: dp2 r14.z, r13.xyxx, cb0[14].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 116: mul r10.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 117: dp2 r14.x, r13.xyxx, r10.xyxx
    r14.x = (dot((r13.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 118: dp2 r2.x, r12.xyxx, r10.xyxx
    r2.x = (dot((r12.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 119: dp2 r2.z, r12.xyxx, cb0[14].xyxx
    r2.z = (dot((r12.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 120: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 121: dp4 r10.x, cb0[15].xyzw, r14.xyzw
    r10.x = (dot((source[15].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 122: dp4 r10.y, cb0[16].xyzw, r14.xyzw
    r10.y = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 123: dp4 r10.z, cb0[17].xyzw, r14.xyzw
    r10.z = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 124: mul r11.xyzw, r14.yzzx, r14.xyzz
    r11.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 125: dp4 r12.x, cb0[18].xyzw, r11.xyzw
    r12.x = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 126: dp4 r12.y, cb0[19].xyzw, r11.xyzw
    r12.y = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 127: dp4 r12.z, cb0[20].xyzw, r11.xyzw
    r12.z = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 128: add r10.xyz, r10.xyzx, r12.xyzx
    r10.xyz = ((r10.xyzx)+(r12.xyzx)).xyz;
    // 129: mul r5.w, r14.y, r14.y
    r5.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 130: mov r13.z, r14.y
    r13.z = (r14.yyyy).z;
    // 131: mad r5.w, r14.x, r14.x, -r5.w
    r5.w = ((r14.xxxx)*(r14.xxxx)+(-(r5.wwww))).w;
    // 132: mad r10.xyz, cb0[21].xyzx, r5.wwww, r10.xyzx
    r10.xyz = ((source[21].xyzx)*(r5.wwww)+(r10.xyzx)).xyz;
    // 133: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 134: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 135: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 136: log r5.w, |r3.y|
    r5.w = (log2(abs(r3.yyyy))).w;
    // 137: lt r3.y, |r3.y|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 138: mul r5.w, r5.w, cb0[11].w
    r5.w = ((r5.wwww)*(source[11].wwww)).w;
    // 139: exp r5.w, r5.w
    r5.w = (exp2(r5.wwww)).w;
    // 140: movc r3.y, r3.y, l(0), r5.w
    r3.y = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.wwww)).y;
    // 141: max r3.y, r3.y, cb0[0].x
    r3.y = (max(r3.yyyy,source[0].xxxx)).y;
    // 142: min r3.z, r3.y, l(1.000000)
    r3.z = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 143: deriv_rtx_coarse r11.x, r2.w
    r11.x = (ddx_coarse(r2.wwww)).x;
    // 144: deriv_rty_coarse r11.y, r2.w
    r11.y = (ddy_coarse(r2.wwww)).y;
    // 145: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: add_sat r12.x, -r4.w, r2.w
    r12.x = (saturate((-(r4.wwww))+(r2.wwww))).x;
    // 147: dp2 r2.w, r11.xyxx, r11.xyxx
    r2.w = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).w;
    // 148: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 149: mad_sat r12.y, r2.w, l(0.300000), r3.z
    r12.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 150: add r2.w, -r12.y, l(1.000000)
    r2.w = ((-(r12.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mov_sat r0.w, cb0[11].x
    r0.w = (saturate(source[11].xxxx)).w;
    // 152: mad r11.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r11.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 153: mul r3.y, r0.w, l(0.080000)
    r3.y = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).y;
    // 154: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 155: mad r11.xyz, r3.wwww, r11.xyzx, r3.yyyy
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r3.yyyy)).xyz;
    // 156: max r14.xyz, r2.wwww, r11.xyzx
    r14.xyz = (max(r2.wwww,r11.xyzx)).xyz;
    // 157: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 158: mul_sat r0.w, r11.y, l(50.000000)
    r0.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 159: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 160: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t4.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 161: add r0.w, r3.x, r12.x
    r0.w = ((r3.xxxx)+(r12.xxxx)).w;
    // 162: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 163: mul r15.xyz, r11.xyzx, r12.wwww
    r15.xyz = ((r11.xyzx)*(r12.wwww)).xyz;
    // 164: mad r14.xyz, r14.xyzx, r12.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r12.zzzz)+(r15.xyzx)).xyz;
    // 165: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r2.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 166: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 167: mad r12.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 168: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 169: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 170: mad r15.xyz, -r14.xyzx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 171: mul r12.xzw, r12.xxzw, r14.xxyz
    r12.xzw = ((r12.xxzw)*(r14.xxyz)).xzw;
    // 172: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 173: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 174: mad r7.xyz, -r7.xyzx, r3.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r3.wwww)+(r7.xyzx)).xyz;
    // 175: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 176: dp2_sat r10.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 177: dp3_sat r10.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 178: dp3_sat r10.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 179: mul r5.xyz, r10.xyzx, r10.xyzx
    r5.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 180: dp3 r2.w, r9.xyzx, r5.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 181: add r1.w, r1.w, -r2.w
    r1.w = ((r1.wwww)+(-(r2.wwww))).w;
    // 182: mad r1.w, r3.z, r1.w, r2.w
    r1.w = ((r3.zzzz)*(r1.wwww)+(r2.wwww)).w;
    // 183: mad r3.yzw, r8.xxyz, r1.wwww, r6.xxyz
    r3.yzw = ((r8.xxyz)*(r1.wwww)+(r6.xxyz)).yzw;
    // 184: mul r5.xyz, r1.wwww, r8.xyzx
    r5.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 185: mul r1.w, r12.y, r12.y
    r1.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 186: mul r2.w, r12.y, l(5.000000)
    r2.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 187: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r2.xyzx, t5.xyzw, s5, r2.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r2.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 188: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 189: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 190: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 191: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 192: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 193: add r0.w, r3.x, r0.w
    r0.w = ((r3.xxxx)+(r0.wwww)).w;
    // 194: mov o5.y, r3.x
    output.targets[5].y = (r3.xxxx).y;
    // 195: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 196: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 197: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 198: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 199: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 200: mul r6.xyz, r0.wwww, r3.yzwy
    r6.xyz = ((r0.wwww)*(r3.yzwy)).xyz;
    // 201: add r3.xyz, r3.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 202: div r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)/(r3.xyzx)).xyz;
    // 203: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 204: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 205: mad r3.xyz, r2.xyzx, r12.xzwx, r7.xyzx
    r3.xyz = ((r2.xyzx)*(r12.xzwx)+(r7.xyzx)).xyz;
    // 206: mul r2.xyz, r12.xzwx, r2.xyzx
    r2.xyz = ((r12.xzwx)*(r2.xyzx)).xyz;
    // 207: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 208: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 209: add r1.y, -|r4.z|, l(1.000000)
    r1.y = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 210: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 211: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 212: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 213: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 214: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 215: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 216: mul r1.xzw, r1.xxxx, cb0[3].xxyz
    r1.xzw = ((r1.xxxx)*(source[3].xxyz)).xzw;
    // 217: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 218: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 219: add r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)+(r1.xyzx)).xyz;
    // 220: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r1.xyzx)).xyz;
    // 221: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 222: dp3 r0.x, r13.xyzx, r13.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 223: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 224: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 225: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 226: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 227: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 228: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 229: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 230: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 231: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 232: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 233: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 234: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 235: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 236: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
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

// source.character.static-map-native-1127.v1 / source program 1de2e9d97210304b9f3af93f530e610e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1127(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1127(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f;
    // 1: mul r0.xyz, cb0[6].xyzx, cb0[9].wwww
    r0.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 2: mul r1.xyz, cb0[5].xyzx, cb0[9].zzzz
    r1.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 3: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 5: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 6: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 7: mad r4.xyz, cb0[9].yyyy, r4.xyzx, r3.xyzx
    r4.xyz = ((source[9].yyyy)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 8: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 9: mad r0.xyz, r0.xyzx, r3.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 12: mul r0.w, r3.z, cb0[10].x
    r0.w = ((r3.zzzz)*(source[10].xxxx)).w;
    // 13: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 14: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 15: mul r1.w, r1.w, cb0[10].y
    r1.w = ((r1.wwww)*(source[10].yyyy)).w;
    // 16: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 17: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 18: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 20: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 21: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mul r0.w, r2.z, cb0[8].w
    r0.w = ((r2.zzzz)*(source[8].wwww)).w;
    // 23: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 24: mul r1.xy, r1.xyxx, cb0[7].wwww
    r1.xy = ((r1.xyxx)*(source[7].wwww)).xy;
    // 25: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 26: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 28: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 29: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 31: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 32: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 33: dp3 r2.x, r1.xyzx, r1.xyzx
    r2.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: rsq r2.x, r2.x
    r2.x = (rsqrt(r2.xxxx)).x;
    // 35: mul r2.xyz, r1.xyzx, r2.xxxx
    r2.xyz = ((r1.xyzx)*(r2.xxxx)).xyz;
    // 36: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 37: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 38: mul r4.xyz, r2.wwww, v5.xyzx
    r4.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 39: dp3 r2.w, r2.xyzx, r4.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 40: mul r5.xyz, r2.wwww, r2.xyzx
    r5.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 41: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 42: add r6.xyz, r5.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r6.xyz = ((r5.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 43: mad r6.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 44: min r4.w, r6.z, l(1.000000)
    r4.w = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r6.xy, r6.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r6.xyz, r6.xyzx, cb0[4].xyzx
    r6.xyz = ((r6.xyzx)*(source[4].xyzx)).xyz;
    // 48: mad r6.xyz, cb0[8].zzzz, r6.xyzx, r6.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 49: add r6.xyz, r6.xyzx, -cb0[8].zzzz
    r6.xyz = ((r6.xyzx)+(-(source[8].zzzz))).xyz;
    // 50: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 51: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 52: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r0.xyz, r0.wwww, r7.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 54: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 55: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 56: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 57: mul r6.xyz, r0.xyzx, cb0[10].zzzz
    r6.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 58: mad r0.xyz, cb0[10].wwww, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[10].wwww)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 59: mad r0.xyz, r1.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 60: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 62: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 63: mad r6.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r6.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 64: mad r7.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 65: mad r8.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 66: mul r1.w, r3.x, cb0[12].x
    r1.w = ((r3.xxxx)*(source[12].xxxx)).w;
    // 67: mul r3.x, r3.y, cb0[11].z
    r3.x = ((r3.yyyy)*(source[11].zzzz)).x;
    // 68: log r3.y, |r1.w|
    r3.y = (log2(abs(r1.wwww))).y;
    // 69: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 70: mul r3.y, r3.y, cb0[12].y
    r3.y = ((r3.yyyy)*(source[12].yyyy)).y;
    // 71: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 72: min r3.y, r3.y, l(1.000000)
    r3.y = (min(r3.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 73: movc r1.w, r1.w, l(0), r3.y
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).w;
    // 74: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 75: mad r6.xyz, r7.xyzx, r1.wwww, r6.xyzx
    r6.xyz = ((r7.xyzx)*(r1.wwww)+(r6.xyzx)).xyz;
    // 76: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 77: max r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = (max(r1.wwww,r6.xyzx)).xyz;
    // 78: dp3 r3.y, v6.xyzx, v6.xyzx
    r3.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 79: rsq r3.y, r3.y
    r3.y = (rsqrt(r3.yyyy)).y;
    // 80: mul r7.xyz, r3.yyyy, v6.xyzx
    r7.xyz = ((r3.yyyy)*(v6.xyzx)).xyz;
    // 81: dp3 r3.y, r7.xyzx, r2.xyzx
    r3.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 82: dp3 r5.w, r7.xyzx, r5.xyzx
    r5.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 83: mad r7.xy, r5.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r5.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 84: mad r7.zw, r3.yyyy, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r7.zw = ((r3.yyyy)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 85: mul r7.xyzw, r7.xyzw, r7.xyzw
    r7.xyzw = ((r7.xyzw)*(r7.xyzw)).xyzw;
    // 86: mul r8.xyz, r7.wwww, cb0[24].xyzx
    r8.xyz = ((r7.wwww)*(source[24].xyzx)).xyz;
    // 87: mad r8.xyz, r7.zzzz, cb0[23].xyzx, r8.xyzx
    r8.xyz = ((r7.zzzz)*(source[23].xyzx)+(r8.xyzx)).xyz;
    // 88: mul r8.xyz, r8.xyzx, cb0[25].wwww
    r8.xyz = ((r8.xyzx)*(source[25].wwww)).xyz;
    // 89: mul r8.xyz, r0.xyzx, r8.xyzx
    r8.xyz = ((r0.xyzx)*(r8.xyzx)).xyz;
    // 90: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 91: log r3.y, |r3.x|
    r3.y = (log2(abs(r3.xxxx))).y;
    // 92: lt r3.x, |r3.x|, l(0.000001)
    r3.x = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 93: mul r3.y, r3.y, cb0[11].w
    r3.y = ((r3.yyyy)*(source[11].wwww)).y;
    // 94: exp r3.y, r3.y
    r3.y = (exp2(r3.yyyy)).y;
    // 95: movc r3.x, r3.x, l(0), r3.y
    r3.x = ((asuint(r3.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.yyyy)).x;
    // 96: max r3.x, r3.x, cb0[0].x
    r3.x = (max(r3.xxxx,source[0].xxxx)).x;
    // 97: min r3.z, r3.x, l(1.000000)
    r3.z = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: deriv_rtx_coarse r3.x, r2.w
    r3.x = (ddx_coarse(r2.wwww)).x;
    // 99: deriv_rty_coarse r3.y, r2.w
    r3.y = (ddy_coarse(r2.wwww)).y;
    // 100: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: add_sat r8.x, -r4.w, r2.w
    r8.x = (saturate((-(r4.wwww))+(r2.wwww))).x;
    // 102: dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 103: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 104: mad_sat r8.y, r2.w, l(0.300000), r3.z
    r8.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 105: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 106: add r2.w, -r8.y, l(1.000000)
    r2.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 107: mov_sat r0.w, cb0[11].x
    r0.w = (saturate(source[11].xxxx)).w;
    // 108: mad r3.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r3.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 109: mul r4.w, r0.w, l(0.080000)
    r4.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 110: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 111: mad r3.xyz, r3.wwww, r3.xyzx, r4.wwww
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r4.wwww)).xyz;
    // 112: max r9.xyz, r2.wwww, r3.xyzx
    r9.xyz = (max(r2.wwww,r3.xyzx)).xyz;
    // 113: add r9.xyz, -r3.xyzx, r9.xyzx
    r9.xyz = ((-(r3.xyzx))+(r9.xyzx)).xyz;
    // 114: mul_sat r0.w, r3.y, l(50.000000)
    r0.w = (saturate((r3.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 115: mul r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 116: sample_indexable(texture2d)(float,float,float,float) r7.zw, r8.xyxx, t4.zwxy, s5
    r7.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 117: add r0.w, r1.w, r8.x
    r0.w = ((r1.wwww)+(r8.xxxx)).w;
    // 118: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 119: mul r8.xzw, r3.xxyz, r7.wwww
    r8.xzw = ((r3.xxyz)*(r7.wwww)).xzw;
    // 120: mad r8.xzw, r9.xxyz, r7.zzzz, r8.xxzw
    r8.xzw = ((r9.xxyz)*(r7.zzzz)+(r8.xxzw)).xzw;
    // 121: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.w
    r2.w = r7.w != 0.f ? 1.f / r7.w : 0.f;
    // 122: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 123: mad r9.xyz, r3.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r3.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 124: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: mad r3.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r3.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 126: mad r10.xyz, -r8.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r8.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 127: mul r8.xzw, r8.xxzw, r9.xxyz
    r8.xzw = ((r8.xxzw)*(r9.xxyz)).xzw;
    // 128: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 129: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 130: mul r9.xyz, r2.wwww, v1.xyzx
    r9.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 131: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 132: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 133: mul r11.xyz, r2.wwww, v0.xyzx
    r11.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 134: mul r12.xyz, r9.zxyz, r11.yzxy
    r12.xyz = ((r9.zxyz)*(r11.yzxy)).xyz;
    // 135: mad r12.xyz, r9.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r9.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 136: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 137: dp3 r13.y, r12.xyzx, r2.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 138: dp3 r12.y, r12.xyzx, r5.xyzx
    r12.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 139: dp3 r13.x, r11.xyzx, r2.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 140: dp3 r2.y, r9.xyzx, r2.xyzx
    r2.y = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 141: dp3 r9.y, r9.xyzx, r5.xyzx
    r9.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 142: dp3 r12.x, r11.xyzx, r5.xyzx
    r12.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 143: dp2 r2.z, r13.xyxx, cb0[14].xyxx
    r2.z = (dot((r13.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 144: mul r5.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 145: dp2 r2.x, r13.xyxx, r5.xyxx
    r2.x = (dot((r13.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 146: dp2 r9.x, r12.xyxx, r5.xyxx
    r9.x = (dot((r12.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 147: dp2 r9.z, r12.xyxx, cb0[14].xyxx
    r9.z = (dot((r12.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 148: mov r2.w, l(1.000000)
    r2.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 149: dp4 r5.x, cb0[15].xyzw, r2.xyzw
    r5.x = (dot((source[15].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).x;
    // 150: dp4 r5.y, cb0[16].xyzw, r2.xyzw
    r5.y = (dot((source[16].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).y;
    // 151: dp4 r5.z, cb0[17].xyzw, r2.xyzw
    r5.z = (dot((source[17].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).z;
    // 152: mul r11.xyzw, r2.yzzx, r2.xyzz
    r11.xyzw = ((r2.yzzx)*(r2.xyzz)).xyzw;
    // 153: dp4 r12.x, cb0[18].xyzw, r11.xyzw
    r12.x = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 154: dp4 r12.y, cb0[19].xyzw, r11.xyzw
    r12.y = (dot((source[19].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 155: dp4 r12.z, cb0[20].xyzw, r11.xyzw
    r12.z = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 156: add r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)+(r12.xyzx)).xyz;
    // 157: mul r2.z, r2.y, r2.y
    r2.z = ((r2.yyyy)*(r2.yyyy)).z;
    // 158: mov r13.z, r2.y
    r13.z = (r2.yyyy).z;
    // 159: mad r2.x, r2.x, r2.x, -r2.z
    r2.x = ((r2.xxxx)*(r2.xxxx)+(-(r2.zzzz))).x;
    // 160: mad r2.xyz, cb0[21].xyzx, r2.xxxx, r5.xyzx
    r2.xyz = ((source[21].xyzx)*(r2.xxxx)+(r5.xyzx)).xyz;
    // 161: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 162: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 163: mad r2.xyz, r2.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 164: mul r2.xyz, r10.xyzx, r2.xyzx
    r2.xyz = ((r10.xyzx)*(r2.xyzx)).xyz;
    // 165: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 166: mad r2.xyz, -r2.xyzx, r3.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r3.wwww)+(r2.xyzx)).xyz;
    // 167: mul r2.w, r8.y, r8.y
    r2.w = ((r8.yyyy)*(r8.yyyy)).w;
    // 168: mul r3.w, r8.y, l(5.000000)
    r3.w = ((r8.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 169: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r9.xyzx, t5.xyzw, s4, r3.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 170: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 171: mul r5.xyz, r5.xyzx, cb0[13].xyzx
    r5.xyz = ((r5.xyzx)*(source[13].xyzx)).xyz;
    // 172: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 173: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 174: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 175: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 176: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 177: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 178: mad r1.w, r0.w, r3.x, r3.y
    r1.w = ((r0.wwww)*(r3.xxxx)+(r3.yyyy)).w;
    // 179: mad r1.w, r1.w, r0.w, r3.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r3.zzzz)).w;
    // 180: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 181: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 182: mul r3.xyz, r7.yyyy, cb0[24].xyzx
    r3.xyz = ((r7.yyyy)*(source[24].xyzx)).xyz;
    // 183: mad r3.xyz, cb0[23].xyzx, r7.xxxx, r3.xyzx
    r3.xyz = ((source[23].xyzx)*(r7.xxxx)+(r3.xyzx)).xyz;
    // 184: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 185: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 186: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 187: mad r2.xyz, r3.xyzx, r8.xzwx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r8.xzwx)+(r2.xyzx)).xyz;
    // 188: mul r3.xyz, r8.xzwx, r3.xyzx
    r3.xyz = ((r8.xzwx)*(r3.xyzx)).xyz;
    // 189: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: dp3 r0.w, r1.xyzx, r4.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 191: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 194: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 195: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 196: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 197: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 198: mul r1.yzw, r0.wwww, cb0[3].xxyz
    r1.yzw = ((r0.wwww)*(source[3].xxyz)).yzw;
    // 199: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 200: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 201: add r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)+(r1.xyzx)).xyz;
    // 202: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 203: mad o0.xyz, r0.xyzx, cb0[25].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[25].xyzx)+(r1.xyzx)).xyz;
    // 204: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 205: dp3 r0.x, r13.xyzx, r13.xyzx
    r0.x = (dot((r13.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 206: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 207: mul r0.xyz, r0.xxxx, r13.xyzx
    r0.xyz = ((r0.xxxx)*(r13.xyzx)).xyz;
    // 208: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 209: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 210: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 211: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 212: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 213: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 214: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 215: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 216: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 217: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 218: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 219: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 220: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 221: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 222: ret
    return output;
}

// source.character.static-map-native-1128.v1 / source program 06076616d501a7469e670ed2f52d0bb4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1128(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
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
    // 6: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
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
    // 29: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 30: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 33: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.zw, v4.xxxy, cb0[7].yyyy
    r0.zw = ((v4.xxxy)*(source[7].yyyy)).zw;
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
    // 40: max r1.w, cb0[7].w, l(0.000000)
    r1.w = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
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
    // 50: mul r7.xyz, cb0[6].xyzx, cb0[9].wwww
    r7.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 51: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 52: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 54: mad r6.xyz, cb0[10].yyyy, r6.xyzx, r8.xyzx
    r6.xyz = ((source[10].yyyy)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[4].xyzx, cb0[8].wwww
    r7.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 56: mul r7.xyz, r3.xyzx, r7.xyzx
    r7.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 57: mul r8.xyz, cb0[5].xyzx, cb0[9].xxxx
    r8.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 58: mad r3.xyz, r8.xyzx, r3.xyzx, -r7.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)+(-(r7.xyzx))).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 60: mul r0.y, r8.z, cb0[9].y
    r0.y = ((r8.zzzz)*(source[9].yyyy)).y;
    // 61: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 62: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 64: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 65: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 66: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 68: mad r3.xyz, r1.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 69: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 70: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 71: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 72: mul r6.xyz, r3.xyzx, cb0[10].zzzz
    r6.xyz = ((r3.xyzx)*(source[10].zzzz)).xyz;
    // 73: mad r3.xyz, cb0[10].wwww, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(-(r6.xyzx))).xyz;
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
    // 79: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 80: mad r9.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 81: mul r0.y, r8.x, cb0[12].x
    r0.y = ((r8.xxxx)*(source[12].xxxx)).y;
    // 82: mul r1.w, r8.y, cb0[11].z
    r1.w = ((r8.yyyy)*(source[11].zzzz)).w;
    // 83: log r2.w, |r0.y|
    r2.w = (log2(abs(r0.yyyy))).w;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r2.w, r2.w, cb0[12].y
    r2.w = ((r2.wwww)*(source[12].yyyy)).w;
    // 86: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 87: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: movc r0.y, r0.y, l(0), r2.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 89: mad r7.xyz, r0.yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((r0.yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 90: mad r6.xyz, r7.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r7.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 91: mul r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = ((r0.yyyy)*(r6.xyzx)).xyz;
    // 92: max r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = (max(r0.yyyy,r6.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r7.xy, r0.zwzz, cb0[7].zzzz
    r7.xy = ((r0.zwzz)*(source[7].zzzz)).xy;
    // 95: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 97: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 98: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 99: add r7.xyz, -r1.xyzx, r7.xyzx
    r7.xyz = ((-(r1.xyzx))+(r7.xyzx)).xyz;
    // 100: mad r0.xzw, r0.xxxx, r7.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r7.xxyz)+(r1.xxyz)).xzw;
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
    // 106: mul r7.xyz, r2.wwww, v6.xyzx
    r7.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 107: dp3 r2.w, r7.xyzx, r1.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 108: mad r7.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 110: mul r7.yzw, r7.yyyy, cb0[24].xxyz
    r7.yzw = ((r7.yyyy)*(source[24].xxyz)).yzw;
    // 111: mad r7.xyz, r7.xxxx, cb0[23].xyzx, r7.yzwy
    r7.xyz = ((r7.xxxx)*(source[23].xyzx)+(r7.yzwy)).xyz;
    // 112: mul r7.xyz, r7.xyzx, cb0[25].wwww
    r7.xyz = ((r7.xyzx)*(source[25].wwww)).xyz;
    // 113: mul r9.xyz, r3.xyzx, r7.xyzx
    r9.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
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
    // 119: mul r11.xyz, r11.xyzx, cb0[27].xyzx
    r11.xyz = ((r11.xyzx)*(source[27].xyzx)).xyz;
    // 120: dp3 r2.w, r11.xyzx, r10.xyzx
    r2.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 121: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 122: mul r10.xyz, r10.xyzx, cb0[26].xyzx
    r10.xyz = ((r10.xyzx)*(source[26].xyzx)).xyz;
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
    // 128: dp2 r12.z, r9.xyxx, cb0[14].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 129: dp3 r12.y, r4.xyzx, r1.xyzx
    r12.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 130: mul r8.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r12.x, r9.xyxx, r8.xyxx
    r12.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 132: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 133: dp4 r13.x, cb0[15].xyzw, r12.xyzw
    r13.x = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[16].xyzw, r12.xyzw
    r13.y = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[17].xyzw, r12.xyzw
    r13.z = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 137: dp4 r15.x, cb0[18].xyzw, r14.xyzw
    r15.x = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 138: dp4 r15.y, cb0[19].xyzw, r14.xyzw
    r15.y = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 139: dp4 r15.z, cb0[20].xyzw, r14.xyzw
    r15.z = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 140: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 141: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 142: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 143: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 144: mad r12.xyz, cb0[21].xyzx, r4.wwww, r13.xyzx
    r12.xyz = ((source[21].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 145: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 146: mul r12.xyz, r12.xyzx, cb0[13].xyzx
    r12.xyz = ((r12.xyzx)*(source[13].xyzx)).xyz;
    // 147: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 148: mov_sat r3.w, cb0[11].x
    r3.w = (saturate(source[11].xxxx)).w;
    // 149: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 150: mul r4.w, r3.w, l(0.080000)
    r4.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 151: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 152: mad r13.xyz, r8.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r8.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 153: mul_sat r3.w, r13.y, l(50.000000)
    r3.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: log r4.w, |r1.w|
    r4.w = (log2(abs(r1.wwww))).w;
    // 155: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 156: mul r4.w, r4.w, cb0[11].w
    r4.w = ((r4.wwww)*(source[11].wwww)).w;
    // 157: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 158: movc r1.w, r1.w, l(0), r4.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 159: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 160: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
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
    // 172: mad_sat r15.y, r4.w, l(0.300000), r8.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
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
    // 194: mad r6.xyz, -r6.xyzx, r8.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.wwww)+(r6.xyzx)).xyz;
    // 195: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 196: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 197: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 198: dp2 r5.x, r2.xyxx, r8.xyxx
    r5.x = (dot((r2.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 199: dp2 r5.z, r2.xyxx, cb0[14].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
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
    // 210: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 211: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
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
    // 218: mad r1.x, r8.z, r1.y, r1.x
    r1.x = ((r8.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 219: mad r1.yzw, r10.xxyz, r1.xxxx, r7.xxyz
    r1.yzw = ((r10.xxyz)*(r1.xxxx)+(r7.xxyz)).yzw;
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
    // 245: mad o0.xyz, r3.xyzx, cb0[25].xyzx, r0.xzwx
    output.targets[0].xyz = ((r3.xyzx)*(source[25].xyzx)+(r0.xzwx)).xyz;
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
    // 261: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
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

// source.character.static-map-native-1128.v1 / source program 9a841df587f62a40b583958a26d20ae3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1128(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1128(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[13]=g_SourceCharacterEnvironmentColor;source[14]=g_SourceCharacterEnvironmentRotation;}
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
    // 6: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
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
    // 29: mad r0.x, r0.x, l(0.500000), cb0[8].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].zzzz)).x;
    // 30: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 32: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 33: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 34: mul r0.zw, v4.xxxy, cb0[7].yyyy
    r0.zw = ((v4.xxxy)*(source[7].yyyy)).zw;
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
    // 40: max r1.w, cb0[7].w, l(0.000000)
    r1.w = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
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
    // 50: mul r7.xyz, cb0[6].xyzx, cb0[9].wwww
    r7.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 51: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 52: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 54: mad r6.xyz, cb0[10].yyyy, r6.xyzx, r8.xyzx
    r6.xyz = ((source[10].yyyy)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[4].xyzx, cb0[8].wwww
    r7.xyz = ((source[4].xyzx)*(source[8].wwww)).xyz;
    // 56: mul r7.xyz, r3.xyzx, r7.xyzx
    r7.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 57: mul r8.xyz, cb0[5].xyzx, cb0[9].xxxx
    r8.xyz = ((source[5].xyzx)*(source[9].xxxx)).xyz;
    // 58: mad r3.xyz, r8.xyzx, r3.xyzx, -r7.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)+(-(r7.xyzx))).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 60: mul r0.y, r8.z, cb0[9].y
    r0.y = ((r8.zzzz)*(source[9].yyyy)).y;
    // 61: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 62: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 63: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 64: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 65: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 66: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 67: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 68: mad r3.xyz, r1.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 69: add r6.xyz, -r3.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))+(r6.xyzx)).xyz;
    // 70: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 71: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 72: mul r6.xyz, r3.xyzx, cb0[10].zzzz
    r6.xyz = ((r3.xyzx)*(source[10].zzzz)).xyz;
    // 73: mad r3.xyz, cb0[10].wwww, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(-(r6.xyzx))).xyz;
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
    // 79: mad r7.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 80: mad r9.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 81: mul r0.y, r8.x, cb0[12].x
    r0.y = ((r8.xxxx)*(source[12].xxxx)).y;
    // 82: mul r1.w, r8.y, cb0[11].z
    r1.w = ((r8.yyyy)*(source[11].zzzz)).w;
    // 83: log r2.w, |r0.y|
    r2.w = (log2(abs(r0.yyyy))).w;
    // 84: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 85: mul r2.w, r2.w, cb0[12].y
    r2.w = ((r2.wwww)*(source[12].yyyy)).w;
    // 86: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 87: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: movc r0.y, r0.y, l(0), r2.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 89: mad r7.xyz, r0.yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((r0.yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 90: mad r6.xyz, r7.xyzx, r0.yyyy, r6.xyzx
    r6.xyz = ((r7.xyzx)*(r0.yyyy)+(r6.xyzx)).xyz;
    // 91: mul r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = ((r0.yyyy)*(r6.xyzx)).xyz;
    // 92: max r6.xyz, r0.yyyy, r6.xyzx
    r6.xyz = (max(r0.yyyy,r6.xyzx)).xyz;
    // 93: dp2 r2.w, r0.zwzz, r0.zwzz
    r2.w = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).w;
    // 94: mul r7.xy, r0.zwzz, cb0[7].zzzz
    r7.xy = ((r0.zwzz)*(source[7].zzzz)).xy;
    // 95: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 96: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 97: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 98: add r7.z, r0.z, l(0.000010)
    r7.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 99: add r7.xyz, -r1.xyzx, r7.xyzx
    r7.xyz = ((-(r1.xyzx))+(r7.xyzx)).xyz;
    // 100: mad r0.xzw, r0.xxxx, r7.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r7.xxyz)+(r1.xxyz)).xzw;
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
    // 106: mul r7.xyz, r2.wwww, v6.xyzx
    r7.xyz = ((r2.wwww)*(v6.xyzx)).xyz;
    // 107: dp3 r2.w, r7.xyzx, r1.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 108: mad r8.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 109: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 110: mul r9.xyz, r8.yyyy, cb0[24].xyzx
    r9.xyz = ((r8.yyyy)*(source[24].xyzx)).xyz;
    // 111: mad r9.xyz, r8.xxxx, cb0[23].xyzx, r9.xyzx
    r9.xyz = ((r8.xxxx)*(source[23].xyzx)+(r9.xyzx)).xyz;
    // 112: mul r9.xyz, r9.xyzx, cb0[25].wwww
    r9.xyz = ((r9.xyzx)*(source[25].wwww)).xyz;
    // 113: mul r9.xyz, r3.xyzx, r9.xyzx
    r9.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 114: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 115: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 116: dp3 r9.y, r5.xyzx, r1.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 117: dp2 r10.z, r9.xyxx, cb0[14].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 118: dp3 r10.y, r4.xyzx, r1.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 119: mul r8.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 120: dp2 r10.x, r9.xyxx, r8.xyxx
    r10.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 121: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 122: dp4 r11.x, cb0[15].xyzw, r10.xyzw
    r11.x = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 123: dp4 r11.y, cb0[16].xyzw, r10.xyzw
    r11.y = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 124: dp4 r11.z, cb0[17].xyzw, r10.xyzw
    r11.z = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 125: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 126: dp4 r13.x, cb0[18].xyzw, r12.xyzw
    r13.x = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 127: dp4 r13.y, cb0[19].xyzw, r12.xyzw
    r13.y = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 128: dp4 r13.z, cb0[20].xyzw, r12.xyzw
    r13.z = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 129: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 130: mul r2.w, r10.y, r10.y
    r2.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 131: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 132: mad r2.w, r10.x, r10.x, -r2.w
    r2.w = ((r10.xxxx)*(r10.xxxx)+(-(r2.wwww))).w;
    // 133: mad r10.xyz, cb0[21].xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((source[21].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 134: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 136: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 137: mov_sat r3.w, cb0[11].x
    r3.w = (saturate(source[11].xxxx)).w;
    // 138: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 139: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 140: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 141: mad r11.xyz, r8.wwww, r11.xyzx, r2.wwww
    r11.xyz = ((r8.wwww)*(r11.xyzx)+(r2.wwww)).xyz;
    // 142: mul_sat r2.w, r11.y, l(50.000000)
    r2.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 143: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 144: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 145: mul r3.w, r3.w, cb0[11].w
    r3.w = ((r3.wwww)*(source[11].wwww)).w;
    // 146: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 147: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 148: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 149: min r8.z, r1.w, l(1.000000)
    r8.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
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
    // 161: mad_sat r13.y, r3.w, l(0.300000), r8.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 162: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
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
    // 184: mad r6.xyz, -r6.xyzx, r8.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r8.wwww)+(r6.xyzx)).xyz;
    // 185: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 186: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 187: dp2 r5.x, r2.xyxx, r8.xyxx
    r5.x = (dot((r2.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 188: dp2 r5.z, r2.xyxx, cb0[14].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
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
    // 197: dp3 r1.x, r7.xyzx, r1.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 198: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s5, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 202: mul r2.xyz, r2.xyzx, cb0[13].xyzx
    r2.xyz = ((r2.xyzx)*(source[13].xyzx)).xyz;
    // 203: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 204: mad r1.z, r0.y, r11.x, r11.y
    r1.z = ((r0.yyyy)*(r11.xxxx)+(r11.yyyy)).z;
    // 205: mad r1.z, r1.z, r0.y, r11.z
    r1.z = ((r1.zzzz)*(r0.yyyy)+(r11.zzzz)).z;
    // 206: mul r1.z, r0.y, r1.z
    r1.z = ((r0.yyyy)*(r1.zzzz)).z;
    // 207: max r0.y, r0.y, r1.z
    r0.y = (max(r0.yyyy,r1.zzzz)).y;
    // 208: mul r1.yzw, r1.yyyy, cb0[24].xxyz
    r1.yzw = ((r1.yyyy)*(source[24].xxyz)).yzw;
    // 209: mad r1.xyz, cb0[23].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[23].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 210: mul r1.xyz, r1.xyzx, cb0[25].wwww
    r1.xyz = ((r1.xyzx)*(source[25].wwww)).xyz;
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
    // 229: mad o0.xyz, r3.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
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
    // 243: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
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

// source.character.static-map-native-1129.v1 / source program 2fa8923f6b44cf43b7358f69aeb679da
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1129(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[10]=1.f;
    source[11]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[5].xxxx
    r2.xy = ((r2.xyxx)*(source[5].xxxx)).xy;
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
    // 26: mul r3.xyz, cb0[4].xyzx, cb0[6].wwww
    r3.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xzwy, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 28: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.yyyy
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r6.xyz, r3.xyzx, r4.yyyy
    r6.xyz = ((r3.xyzx)*(r4.yyyy)).xyz;
    // 30: mad r1.yzw, r6.xxyz, r5.xxyz, r1.yyyy
    r1.yzw = ((r6.xxyz)*(r5.xxyz)+(r1.yyyy)).yzw;
    // 31: mul r1.yzw, r1.yyzw, cb0[8].xxyz
    r1.yzw = ((r1.yyzw)*(source[8].xxyz)).yzw;
    // 32: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.xxxx
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 33: mad r5.xyz, r6.xyzx, r5.xyzx, r1.xxxx
    r5.xyz = ((r6.xyzx)*(r5.xyzx)+(r1.xxxx)).xyz;
    // 34: mad r1.xyz, r5.xyzx, cb0[7].xyzx, r1.yzwy
    r1.xyz = ((r5.xyzx)*(source[7].xyzx)+(r1.yzwy)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[9].wwww
    r1.xyz = ((r1.xyzx)*(source[9].wwww)).xyz;
    // 36: mad r3.xyz, -r4.yyyy, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r4.yyyy))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 39: mul r0.w, r4.z, cb0[5].w
    r0.w = ((r4.zzzz)*(source[5].wwww)).w;
    // 40: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 41: mad r5.xyz, cb0[6].xxxx, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[6].xxxx)*(source[2].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: mad r4.yzw, r4.zzzz, r5.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r4.yzw = ((r4.zzzz)*(r5.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 43: mul r5.xyz, r4.xxxx, cb0[3].xyzx
    r5.xyz = ((r4.xxxx)*(source[3].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[6].yyyy
    r5.xyz = ((r5.xyzx)*(source[6].yyyy)).xyz;
    // 45: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 46: mul r0.xyz, r0.xyzx, r4.yzwy
    r0.xyz = ((r0.xyzx)*(r4.yzwy)).xyz;
    // 47: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 48: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 49: dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 50: dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 51: dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 52: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 53: mad r3.xyz, r7.xyzx, r3.xyzx, r6.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 54: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t5.xyzw, s4
    r6.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 55: mul r6.xyz, r6.xyzx, cb0[11].xyzx
    r6.xyz = ((r6.xyzx)*(source[11].xyzx)).xyz;
    // 56: dp3 r0.w, r6.xyzx, r3.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 57: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s4
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((r3.xyzx)*(source[10].xyzx)).xyz;
    // 59: mul r7.xyz, r0.wwww, r3.xyzx
    r7.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 60: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 61: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 62: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 63: div r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)/(r1.xyzx)).xyz;
    // 64: mad r4.xyz, r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 65: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 66: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 67: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 68: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 69: dp3 r1.w, r2.xyzx, r1.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 70: mul r5.xyz, r1.wwww, r2.xyzx
    r5.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 71: mad r1.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 72: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 73: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 74: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 75: log r1.xyz, r5.xyzx
    r1.xyz = (log2(r5.xyzx)).xyz;
    // 76: add r1.w, cb0[6].z, l(1.000000)
    r1.w = ((source[6].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 77: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 78: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 79: dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 80: mad r1.yzw, r3.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r3.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 81: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 82: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 83: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 84: mul r4.xyz, cb0[1].xyzx, cb0[5].yyyy
    r4.xyz = ((source[1].xyzx)*(source[5].yyyy)).xyz;
    // 85: mad r3.xyz, r3.xyzx, r4.xyzx, cb0[0].xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)+(source[0].xyzx)).xyz;
    // 86: add r3.xyz, r1.yzwy, r3.xyzx
    r3.xyz = ((r1.yzwy)+(r3.xyzx)).xyz;
    // 87: mad o0.xyz, r0.xyzx, cb0[9].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 88: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 89: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 90: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 91: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 92: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 93: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 94: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 95: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 96: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 97: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 98: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 99: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 100: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 101: dp3 r0.y, r3.xyzx, r2.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 102: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 103: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 104: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 105: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 106: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 107: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 108: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 109: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 110: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 111: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 112: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 113: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 114: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 115: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 116: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 117: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 118: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 119: ret
    return output;
}

// source.character.static-map-native-1129.v1 / source program 4bb4bcfd7f160743a328e033c50d129b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1129(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1129(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[6];
    source[5]=g_SourceCharacterBaseConstants[7];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
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
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 10: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 12: mul r2.xy, r2.xyxx, cb0[4].xxxx
    r2.xy = ((r2.xyxx)*(source[4].xxxx)).xy;
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
    // 26: mul r3.xyz, cb0[3].xyzx, cb0[5].wwww
    r3.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
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
    // 33: mul r1.xyz, r1.xyzx, cb0[7].xyzx
    r1.xyz = ((r1.xyzx)*(source[7].xyzx)).xyz;
    // 34: mad r1.xyz, r3.xyzx, cb0[6].xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(source[6].xyzx)+(r1.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t3.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: mul r4.xyz, cb0[1].xyzx, cb0[4].yyyy
    r4.xyz = ((source[1].xyzx)*(source[4].yyyy)).xyz;
    // 38: mad r3.xyz, r3.xyzx, r4.xyzx, cb0[0].xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)+(source[0].xyzx)).xyz;
    // 39: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 41: mul r0.w, r1.w, cb0[4].w
    r0.w = ((r1.wwww)*(source[4].wwww)).w;
    // 42: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 43: mad r4.xyz, cb0[5].xxxx, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[5].xxxx)*(source[2].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 44: mad r4.xyz, r1.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 45: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 46: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 47: mad r3.xyz, r1.xyzx, r0.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 48: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 49: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 50: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 51: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 52: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 53: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 54: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 55: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 56: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 57: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 58: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 59: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 60: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 61: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 62: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 63: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 64: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 65: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 68: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 69: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 70: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 71: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 72: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 73: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 74: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 75: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 76: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 77: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 78: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 79: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 80: ret
    return output;
}

// source.character.static-map-native-1130.v1 / source program d040a72c3642a04d877f3cbd60ce11b3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1130(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[6];
    source[6]=g_SourceCharacterBaseConstants[7];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[11]=1.f;
    source[12]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 12: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 13: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: add r3.z, r0.w, l(0.000010)
    r3.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 16: mul r2.zw, v4.xxxy, cb0[5].yyyy
    r2.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.zwzz, t1.zwxy, s1, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 18: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: mul r2.zw, r2.zzzw, cb0[5].zzzz
    r2.zw = ((r2.zzzw)*(source[5].zzzz)).zw;
    // 20: mad r3.xy, cb0[5].xxxx, r2.xyxx, r2.zwzz
    r3.xy = ((source[5].xxxx)*(r2.xyxx)+(r2.zwzz)).xy;
    // 21: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 22: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 23: div r2.xyz, r3.xyzx, r0.wwww
    r2.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 24: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 27: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 28: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 29: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 30: mul r3.xyz, cb0[4].xyzx, cb0[7].yyyy
    r3.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t3.xzwy, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 32: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.yyyy
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 33: mul r6.xyz, r3.xyzx, r4.yyyy
    r6.xyz = ((r3.xyzx)*(r4.yyyy)).xyz;
    // 34: mad r1.yzw, r6.xxyz, r5.xxyz, r1.yyyy
    r1.yzw = ((r6.xxyz)*(r5.xxyz)+(r1.yyyy)).yzw;
    // 35: mul r1.yzw, r1.yyzw, cb0[9].xxyz
    r1.yzw = ((r1.yyzw)*(source[9].xxyz)).yzw;
    // 36: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.xxxx
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 37: mad r5.xyz, r6.xyzx, r5.xyzx, r1.xxxx
    r5.xyz = ((r6.xyzx)*(r5.xyzx)+(r1.xxxx)).xyz;
    // 38: mad r1.xyz, r5.xyzx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r5.xyzx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 39: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 40: mad r3.xyz, -r4.yyyy, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r4.yyyy))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 41: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 42: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 43: mul r0.w, r4.z, cb0[6].y
    r0.w = ((r4.zzzz)*(source[6].yyyy)).w;
    // 44: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 45: mad r5.xyz, cb0[6].zzzz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[6].zzzz)*(source[2].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 46: mad r4.yzw, r4.zzzz, r5.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r4.yzw = ((r4.zzzz)*(r5.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 47: mul r5.xyz, r4.xxxx, cb0[3].xyzx
    r5.xyz = ((r4.xxxx)*(source[3].xyzx)).xyz;
    // 48: mul r5.xyz, r5.xyzx, cb0[6].wwww
    r5.xyz = ((r5.xyzx)*(source[6].wwww)).xyz;
    // 49: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 50: mul r0.xyz, r0.xyzx, r4.yzwy
    r0.xyz = ((r0.xyzx)*(r4.yzwy)).xyz;
    // 51: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 52: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 53: dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 54: dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 55: dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 56: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 57: mad r3.xyz, r7.xyzx, r3.xyzx, r6.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 58: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t6.xyzw, s5
    r6.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 59: mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // 60: dp3 r0.w, r6.xyzx, r3.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 61: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t5.xyzw, s5
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 62: mul r3.xyz, r3.xyzx, cb0[11].xyzx
    r3.xyz = ((r3.xyzx)*(source[11].xyzx)).xyz;
    // 63: mul r7.xyz, r0.wwww, r3.xyzx
    r7.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 64: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 65: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 66: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 67: div r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)/(r1.xyzx)).xyz;
    // 68: mad r4.xyz, r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 69: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 70: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 71: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 72: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 73: dp3 r1.w, r2.xyzx, r1.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 74: mul r5.xyz, r1.wwww, r2.xyzx
    r5.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 75: mad r1.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 76: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 77: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 78: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 79: log r1.xyz, r5.xyzx
    r1.xyz = (log2(r5.xyzx)).xyz;
    // 80: add r1.w, cb0[7].x, l(1.000000)
    r1.w = ((source[7].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 81: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 82: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 83: dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 84: mad r1.yzw, r3.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r3.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 85: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 86: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 88: mul r4.xyz, cb0[1].xyzx, cb0[5].wwww
    r4.xyz = ((source[1].xyzx)*(source[5].wwww)).xyz;
    // 89: mad r3.xyz, r3.xyzx, r4.xyzx, cb0[0].xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)+(source[0].xyzx)).xyz;
    // 90: add r3.xyz, r1.yzwy, r3.xyzx
    r3.xyz = ((r1.yzwy)+(r3.xyzx)).xyz;
    // 91: mad o0.xyz, r0.xyzx, cb0[10].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)+(r3.xyzx)).xyz;
    // 92: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 93: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 94: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 95: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 96: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 97: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 98: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 99: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 100: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 101: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 102: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 103: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 104: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 105: dp3 r0.y, r3.xyzx, r2.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 106: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 107: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 108: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 109: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 110: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 111: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 112: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 113: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 114: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 115: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 116: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 117: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 118: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 119: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 120: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 121: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 122: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 123: ret
    return output;
}

// source.character.static-map-native-1130.v1 / source program aee74ecf2bd4f54ebcebb58ec271b514
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1130(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1130(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[6];
    source[5]=g_SourceCharacterBaseConstants[7];
    source[6]=g_SourceCharacterBaseConstants[8];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 7: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: mul r1.zw, v4.xxxy, cb0[4].yyyy
    r1.zw = ((v4.xxxy)*(source[4].yyyy)).zw;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 15: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 16: mul r1.zw, r1.zzzw, cb0[4].zzzz
    r1.zw = ((r1.zzzw)*(source[4].zzzz)).zw;
    // 17: mad r2.xy, cb0[4].xxxx, r1.xyxx, r1.zwzz
    r2.xy = ((source[4].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
    // 18: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 19: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 20: div r1.xyz, r2.xyzx, r0.wwww
    r1.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 21: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 22: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 23: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 24: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 27: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 28: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 29: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 30: mul r3.xyz, cb0[3].xyzx, cb0[6].yyyy
    r3.xyz = ((source[3].xyzx)*(source[6].yyyy)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).zw;
    // 32: mad r4.xyz, r2.zzzz, r3.xyzx, -r2.yyyy
    r4.xyz = ((r2.zzzz)*(r3.xyzx)+(-(r2.yyyy))).xyz;
    // 33: mul r5.xyz, r3.xyzx, r2.zzzz
    r5.xyz = ((r3.xyzx)*(r2.zzzz)).xyz;
    // 34: mad r3.xyz, r2.zzzz, r3.xyzx, -r2.xxxx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)+(-(r2.xxxx))).xyz;
    // 35: mad r3.xyz, r5.xyzx, r3.xyzx, r2.xxxx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r2.xxxx)).xyz;
    // 36: mad r2.xyz, r5.xyzx, r4.xyzx, r2.yyyy
    r2.xyz = ((r5.xyzx)*(r4.xyzx)+(r2.yyyy)).xyz;
    // 37: mul r2.xyz, r2.xyzx, cb0[8].xyzx
    r2.xyz = ((r2.xyzx)*(source[8].xyzx)).xyz;
    // 38: mad r2.xyz, r3.xyzx, cb0[7].xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(source[7].xyzx)+(r2.xyzx)).xyz;
    // 39: mul r2.xyz, r2.xyzx, cb0[9].wwww
    r2.xyz = ((r2.xyzx)*(source[9].wwww)).xyz;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t4.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: mul r4.xyz, cb0[1].xyzx, cb0[4].wwww
    r4.xyz = ((source[1].xyzx)*(source[4].wwww)).xyz;
    // 42: mad r3.xyz, r3.xyzx, r4.xyzx, cb0[0].xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)+(source[0].xyzx)).xyz;
    // 43: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 44: add r4.xyz, -r0.xyzx, r0.wwww
    r4.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 45: mul r0.w, r2.w, cb0[5].y
    r0.w = ((r2.wwww)*(source[5].yyyy)).w;
    // 46: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 47: mad r4.xyz, cb0[5].zzzz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[5].zzzz)*(source[2].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 48: mad r4.xyz, r2.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((r2.wwww)*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 50: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 51: mad r3.xyz, r2.xyzx, r0.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xyzx)+(r3.xyzx)).xyz;
    // 52: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 53: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 54: mad o0.xyz, r0.xyzx, cb0[9].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)+(r3.xyzx)).xyz;
    // 55: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 56: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 57: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 58: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 59: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 60: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 61: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 62: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 63: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 64: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 65: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 66: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 68: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 70: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 71: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 72: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 73: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 74: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 75: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 76: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 77: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 78: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 79: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 80: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 81: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 82: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 83: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 84: ret
    return output;
}

// source.character.static-map-native-1131.v1 / source program 5e4579283550344d8f20685235275386
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1131(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    source[29]=1.f;
    source[30]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
    // 1: mul r0.xyz, cb0[7].xyzx, cb0[11].wwww
    r0.xyz = ((source[7].xyzx)*(source[11].wwww)).xyz;
    // 2: mul r1.xyz, cb0[6].xyzx, cb0[11].zzzz
    r1.xyz = ((source[6].xyzx)*(source[11].zzzz)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[11].yyyy, r3.xyzx, r2.xyzx
    r3.xyz = ((source[11].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[12].x
    r0.w = ((r2.zzzz)*(source[12].xxxx)).w;
    // 11: mul r2.xy, r2.yxyy, cb0[14].ywyy
    r2.xy = ((r2.yxyy)*(source[14].ywyy)).xy;
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
    // 18: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 19: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: mul r1.xyz, cb0[8].xyzx, cb0[12].zzzz
    r1.xyz = ((source[8].xyzx)*(source[12].zzzz)).xyz;
    // 21: mul r3.xy, v4.xyxx, cb0[9].yyyy
    r3.xy = ((v4.xyxx)*(source[9].yyyy)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: mul r5.xyz, r1.xyzx, r4.xyzx
    r5.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 26: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: mad r1.xyz, -r1.xyzx, r4.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r0.wwww)).xyz;
    // 28: mul r0.w, r4.w, r4.w
    r0.w = ((r4.wwww)*(r4.wwww)).w;
    // 29: mad r1.xyz, cb0[13].xxxx, r1.xyzx, r5.xyzx
    r1.xyz = ((source[13].xxxx)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 30: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 32: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 33: dp2 r2.z, r4.xyxx, r4.xyxx
    r2.z = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).z;
    // 34: mul r4.xy, r4.xyxx, cb0[9].xxxx
    r4.xy = ((r4.xyxx)*(source[9].xxxx)).xy;
    // 35: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 36: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 38: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 39: add r4.z, r2.z, l(0.000010)
    r4.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 40: dp3 r2.z, r4.xyzx, r4.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 41: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 42: div r4.xyz, r4.xyzx, r2.zzzz
    r4.xyz = ((r4.xyzx)/(r2.zzzz)).xyz;
    // 43: mul r2.z, r4.z, r4.z
    r2.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 44: mul_sat r2.z, r2.z, r2.w
    r2.z = (saturate((r2.zzzz)*(r2.wwww))).z;
    // 45: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 47: max r2.z, cb0[9].w, l(0.000000)
    r2.z = (max(source[9].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 48: min r2.z, r2.z, l(0.990000)
    r2.z = (min(r2.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 49: mul r2.w, r0.w, r2.z
    r2.w = ((r0.wwww)*(r2.zzzz)).w;
    // 50: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 51: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 52: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 53: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 54: mul r6.xyz, r4.wwww, v0.xyzx
    r6.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 55: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 56: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 57: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 58: mul r8.xyz, r4.wwww, v1.xyzx
    r8.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 59: dp3 r7.z, r8.xyzx, r4.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 60: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 61: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 62: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 64: dp3 r4.w, r7.xyzx, r5.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 65: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 67: mad r2.w, r4.w, r2.w, r4.w
    r2.w = ((r4.wwww)*(r2.wwww)+(r4.wwww)).w;
    // 68: add r4.w, -r2.z, r2.w
    r4.w = ((-(r2.zzzz))+(r2.wwww)).w;
    // 69: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: div r2.z, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.zzzz)).z;
    // 71: mad r2.w, -r2.z, r4.w, r2.w
    r2.w = ((-(r2.zzzz))*(r4.wwww)+(r2.wwww)).w;
    // 72: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 73: mad_sat r0.w, r0.w, r2.w, r2.z
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r2.zzzz))).w;
    // 74: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 75: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 76: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 77: mad r0.xyz, cb0[13].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[13].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 78: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 79: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 81: mad_sat r1.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: dp2 r0.x, r3.xyxx, r3.xyxx
    r0.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 83: mul r5.xy, r3.xyxx, cb0[9].zzzz
    r5.xy = ((r3.xyxx)*(source[9].zzzz)).xy;
    // 84: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 86: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 87: add r5.z, r0.x, l(0.000010)
    r5.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: add r0.xyz, -r4.xyzx, r5.xyzx
    r0.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 89: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 90: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r4.xyz, r0.wwww, r0.xyzx
    r4.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 93: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r0.w, r5.xyzx, r4.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 97: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 98: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 99: mul r5.xyz, r2.wwww, cb0[27].xyzx
    r5.xyz = ((r2.wwww)*(source[27].xyzx)).xyz;
    // 100: mad r5.xyz, r2.zzzz, cb0[26].xyzx, r5.xyzx
    r5.xyz = ((r2.zzzz)*(source[26].xyzx)+(r5.xyzx)).xyz;
    // 101: mul r5.xyz, r5.xyzx, cb0[28].wwww
    r5.xyz = ((r5.xyzx)*(source[28].wwww)).xyz;
    // 102: mul r7.xyz, r1.xyzx, r5.xyzx
    r7.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 103: dp2_sat r10.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 104: dp3_sat r10.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 105: dp3_sat r10.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 106: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 107: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t9.xyzw, s6
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 108: mul r11.xyz, r11.xyzx, cb0[30].xyzx
    r11.xyz = ((r11.xyzx)*(source[30].xyzx)).xyz;
    // 109: dp3 r0.w, r11.xyzx, r10.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 110: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s6
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 111: mul r10.xyz, r10.xyzx, cb0[29].xyzx
    r10.xyz = ((r10.xyzx)*(source[29].xyzx)).xyz;
    // 112: mul r12.xyz, r0.wwww, r10.xyzx
    r12.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 113: mad r7.xyz, r1.xyzx, r12.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r12.xyzx)+(r7.xyzx)).xyz;
    // 114: mad r12.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r12.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 115: mad r13.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 116: mad r14.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 117: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 118: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 119: mul r2.w, r2.w, cb0[15].x
    r2.w = ((r2.wwww)*(source[15].xxxx)).w;
    // 120: mul r2.z, r2.z, cb0[14].z
    r2.z = ((r2.zzzz)*(source[14].zzzz)).z;
    // 121: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 122: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 123: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 124: min r3.z, r2.x, l(1.000000)
    r3.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 125: exp r2.x, r2.w
    r2.x = (exp2(r2.wwww)).x;
    // 126: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: movc r2.x, r2.y, l(0), r2.x
    r2.x = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 128: mad r2.yzw, r2.xxxx, r13.xxyz, r14.xxyz
    r2.yzw = ((r2.xxxx)*(r13.xxyz)+(r14.xxyz)).yzw;
    // 129: mad r2.yzw, r2.yyzw, r2.xxxx, r12.xxyz
    r2.yzw = ((r2.yyzw)*(r2.xxxx)+(r12.xxyz)).yzw;
    // 130: mul r2.yzw, r2.xxxx, r2.yyzw
    r2.yzw = ((r2.xxxx)*(r2.yyzw)).yzw;
    // 131: max r2.yzw, r2.yyzw, r2.xxxx
    r2.yzw = (max(r2.yyzw,r2.xxxx)).yzw;
    // 132: mul r2.yzw, r2.yyzw, r7.xxyz
    r2.yzw = ((r2.yyzw)*(r7.xxyz)).yzw;
    // 133: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 134: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 135: dp2 r12.z, r7.xyxx, cb0[17].xyxx
    r12.z = (dot((r7.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 136: dp3 r12.y, r8.xyzx, r4.xyzx
    r12.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 137: mul r3.xy, cb0[17].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((source[17].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 138: dp2 r12.x, r7.xyxx, r3.xyxx
    r12.x = (dot((r7.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 139: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 140: dp4 r13.x, cb0[18].xyzw, r12.xyzw
    r13.x = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 141: dp4 r13.y, cb0[19].xyzw, r12.xyzw
    r13.y = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 142: dp4 r13.z, cb0[20].xyzw, r12.xyzw
    r13.z = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 143: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 144: dp4 r15.x, cb0[21].xyzw, r14.xyzw
    r15.x = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 145: dp4 r15.y, cb0[22].xyzw, r14.xyzw
    r15.y = (dot((source[22].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 146: dp4 r15.z, cb0[23].xyzw, r14.xyzw
    r15.z = (dot((source[23].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 147: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 148: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 149: mov r7.z, r12.y
    r7.z = (r12.yyyy).z;
    // 150: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 151: mad r12.xyz, cb0[24].xyzx, r4.wwww, r13.xyzx
    r12.xyz = ((source[24].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 152: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 153: mul r12.xyz, r12.xyzx, cb0[16].xyzx
    r12.xyz = ((r12.xyzx)*(source[16].xyzx)).xyz;
    // 154: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 155: mov_sat r1.w, cb0[13].w
    r1.w = (saturate(source[13].wwww)).w;
    // 156: mad r13.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r13.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 157: mul r4.w, r1.w, l(0.080000)
    r4.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 158: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 159: mad r13.xyz, r3.wwww, r13.xyzx, r4.wwww
    r13.xyz = ((r3.wwww)*(r13.xyzx)+(r4.wwww)).xyz;
    // 160: mul_sat r1.w, r13.y, l(50.000000)
    r1.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 161: dp3 r4.w, v5.xyzx, v5.xyzx
    r4.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 162: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 163: mul r14.xyz, r4.wwww, v5.xyzx
    r14.xyz = ((r4.wwww)*(v5.xyzx)).xyz;
    // 164: dp3 r4.w, r4.xyzx, r14.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 165: mul r4.xyz, r4.wwww, r4.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 166: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 167: deriv_rtx_coarse r15.x, r4.w
    r15.x = (ddx_coarse(r4.wwww)).x;
    // 168: deriv_rty_coarse r15.y, r4.w
    r15.y = (ddy_coarse(r4.wwww)).y;
    // 169: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 171: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 172: mad_sat r15.y, r5.w, l(0.300000), r3.z
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 173: add r5.w, -r15.y, l(1.000000)
    r5.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 174: max r16.xyz, r13.xyzx, r5.wwww
    r16.xyz = (max(r13.xyzx,r5.wwww)).xyz;
    // 175: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 176: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 177: add r1.w, r4.z, l(1.000000)
    r1.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 179: add_sat r15.x, -r1.w, r4.w
    r15.x = (saturate((-(r1.wwww))+(r4.wwww))).x;
    // 180: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 181: add r1.w, r2.x, r15.x
    r1.w = ((r2.xxxx)+(r15.xxxx)).w;
    // 182: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 183: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 184: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 185: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r4.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 186: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 187: mad r15.xzw, r13.xxyz, r4.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r4.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 188: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 189: mad r13.xyz, r4.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r4.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 190: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 191: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 192: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 193: mul r2.yzw, r2.yyzw, r12.xxyz
    r2.yzw = ((r2.yyzw)*(r12.xxyz)).yzw;
    // 194: mad r2.yzw, -r2.yyzw, r3.wwww, r2.yyzw
    r2.yzw = ((-(r2.yyzw))*(r3.wwww)+(r2.yyzw)).yzw;
    // 195: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 196: dp3 r6.x, r6.xyzx, r4.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 197: dp3 r6.y, r9.xyzx, r4.xyzx
    r6.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 198: dp2 r9.x, r6.xyxx, r3.xyxx
    r9.x = (dot((r6.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 199: dp2 r9.z, r6.xyxx, cb0[17].xyxx
    r9.z = (dot((r6.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 200: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 201: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 202: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 203: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 204: add r1.w, r2.x, r1.w
    r1.w = ((r2.xxxx)+(r1.wwww)).w;
    // 205: mov o5.y, r2.x
    output.targets[5].y = (r2.xxxx).y;
    // 206: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 207: dp3 r9.y, r8.xyzx, r4.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r9.xyzx, t7.xyzw, s7, r3.x
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r3.xyw, r6.xyxz, r6.wwww
    r3.xyw = ((r6.xyxz)*(r6.wwww)).xyw;
    // 210: mul r3.xyw, r3.xyxw, cb0[16].xyxz
    r3.xyw = ((r3.xyxw)*(source[16].xyxz)).xyw;
    // 211: mad r3.xyw, r3.xyxw, l(6.000000, 6.000000, 0.000000, 6.000000), cb0[16].wwww
    r3.xyw = ((r3.xyxw)*(float4(6.000000,6.000000,0.000000,6.000000))+(source[16].wwww)).xyw;
    // 212: dp2_sat r6.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 213: dp3_sat r6.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 214: dp3_sat r6.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 215: mul r4.xyz, r6.xyzx, r6.xyzx
    r4.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 216: dp3 r2.x, r11.xyzx, r4.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 217: add r0.w, r0.w, -r2.x
    r0.w = ((r0.wwww)+(-(r2.xxxx))).w;
    // 218: mad r0.w, r3.z, r0.w, r2.x
    r0.w = ((r3.zzzz)*(r0.wwww)+(r2.xxxx)).w;
    // 219: mad r4.xyz, r10.xyzx, r0.wwww, r5.xyzx
    r4.xyz = ((r10.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 220: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 221: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 222: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 223: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 224: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 225: mul r6.xyz, r0.wwww, r4.xyzx
    r6.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 226: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 227: div r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)/(r4.xyzx)).xyz;
    // 228: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mul r3.xyz, r3.xywx, r6.xyzx
    r3.xyz = ((r3.xywx)*(r6.xyzx)).xyz;
    // 230: mad r2.xyz, r3.xyzx, r15.xzwx, r2.yzwy
    r2.xyz = ((r3.xyzx)*(r15.xzwx)+(r2.yzwy)).xyz;
    // 231: mul r3.xyz, r15.xzwx, r3.xyzx
    r3.xyz = ((r15.xzwx)*(r3.xyzx)).xyz;
    // 232: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 233: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 234: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 235: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 236: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 237: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 238: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 239: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 240: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 241: mul r3.xyz, r0.xxxx, cb0[3].xyzx
    r3.xyz = ((r0.xxxx)*(source[3].xyzx)).xyz;
    // 242: movc r0.xyz, r0.yyyy, l(0,0,0,0), r3.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 243: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 244: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 245: mul r4.xyz, cb0[5].xyzx, cb0[10].wwww
    r4.xyz = ((source[5].xyzx)*(source[10].wwww)).xyz;
    // 246: mad r0.xyz, r3.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 247: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 248: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 249: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 250: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 251: dp3 r0.x, r7.xyzx, r7.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 252: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 253: mul r0.xyz, r0.xxxx, r7.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)).xyz;
    // 254: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 255: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 256: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 257: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 258: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 259: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 260: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 261: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 262: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 263: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 264: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 265: ftou r0.x, cb0[25].z
    r0.x = (asfloat((uint4)(source[25].zzzz))).x;
    // 266: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 267: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 268: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 269: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 270: ret
    return output;
}

// source.character.static-map-native-1131.v1 / source program 5017e99ee0fcaf4bb09eb11ea8ee21f7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1131(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1131(input);
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
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[16]=g_SourceCharacterEnvironmentColor;source[17]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: mul r0.xyz, cb0[7].xyzx, cb0[11].wwww
    r0.xyz = ((source[7].xyzx)*(source[11].wwww)).xyz;
    // 2: mul r1.xyz, cb0[6].xyzx, cb0[11].zzzz
    r1.xyz = ((source[6].xyzx)*(source[11].zzzz)).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 4: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 5: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 6: mad r3.xyz, cb0[11].yyyy, r3.xyzx, r2.xyzx
    r3.xyz = ((source[11].yyyy)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 7: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 8: mad r0.xyz, r0.xyzx, r2.xyzx, -r1.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[12].x
    r0.w = ((r2.zzzz)*(source[12].xxxx)).w;
    // 11: mul r2.xy, r2.yxyy, cb0[14].ywyy
    r2.xy = ((r2.yxyy)*(source[14].ywyy)).xy;
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
    // 18: mul_sat r3.w, r0.w, cb2[3].w
    r3.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 19: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: mul r1.xyz, cb0[8].xyzx, cb0[12].zzzz
    r1.xyz = ((source[8].xyzx)*(source[12].zzzz)).xyz;
    // 21: mul r3.xy, v4.xyxx, cb0[9].yyyy
    r3.xy = ((v4.xyxx)*(source[9].yyyy)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 24: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 25: mul r5.xyz, r1.xyzx, r4.xyzx
    r5.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 26: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: mad r1.xyz, -r1.xyzx, r4.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r0.wwww)).xyz;
    // 28: mul r0.w, r4.w, r4.w
    r0.w = ((r4.wwww)*(r4.wwww)).w;
    // 29: mad r1.xyz, cb0[13].xxxx, r1.xyzx, r5.xyzx
    r1.xyz = ((source[13].xxxx)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 30: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 32: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 33: dp2 r2.z, r4.xyxx, r4.xyxx
    r2.z = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).z;
    // 34: mul r4.xy, r4.xyxx, cb0[9].xxxx
    r4.xy = ((r4.xyxx)*(source[9].xxxx)).xy;
    // 35: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 36: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 38: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 39: add r4.z, r2.z, l(0.000010)
    r4.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 40: dp3 r2.z, r4.xyzx, r4.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 41: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 42: div r4.xyz, r4.xyzx, r2.zzzz
    r4.xyz = ((r4.xyzx)/(r2.zzzz)).xyz;
    // 43: mul r2.z, r4.z, r4.z
    r2.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 44: mul_sat r2.z, r2.z, r2.w
    r2.z = (saturate((r2.zzzz)*(r2.wwww))).z;
    // 45: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 47: max r2.z, cb0[9].w, l(0.000000)
    r2.z = (max(source[9].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 48: min r2.z, r2.z, l(0.990000)
    r2.z = (min(r2.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 49: mul r2.w, r0.w, r2.z
    r2.w = ((r0.wwww)*(r2.zzzz)).w;
    // 50: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 51: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 52: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 53: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 54: mul r6.xyz, r4.wwww, v0.xyzx
    r6.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 55: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 56: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 57: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 58: mul r8.xyz, r4.wwww, v1.xyzx
    r8.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 59: dp3 r7.z, r8.xyzx, r4.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 60: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 61: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 62: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 64: dp3 r4.w, r7.xyzx, r5.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 65: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 67: mad r2.w, r4.w, r2.w, r4.w
    r2.w = ((r4.wwww)*(r2.wwww)+(r4.wwww)).w;
    // 68: add r4.w, -r2.z, r2.w
    r4.w = ((-(r2.zzzz))+(r2.wwww)).w;
    // 69: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 70: div r2.z, l(1.000000, 1.000000, 1.000000, 1.000000), r2.z
    r2.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.zzzz)).z;
    // 71: mad r2.w, -r2.z, r4.w, r2.w
    r2.w = ((-(r2.zzzz))*(r4.wwww)+(r2.wwww)).w;
    // 72: mul r2.z, r4.w, r2.z
    r2.z = ((r4.wwww)*(r2.zzzz)).z;
    // 73: mad_sat r0.w, r0.w, r2.w, r2.z
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r2.zzzz))).w;
    // 74: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 75: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 76: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 77: mad r0.xyz, cb0[13].zzzz, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[13].zzzz)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 78: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 79: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 81: mad_sat r1.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: dp2 r0.x, r3.xyxx, r3.xyxx
    r0.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 83: mul r5.xy, r3.xyxx, cb0[9].zzzz
    r5.xy = ((r3.xyxx)*(source[9].zzzz)).xy;
    // 84: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 85: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 86: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 87: add r5.z, r0.x, l(0.000010)
    r5.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 88: add r0.xyz, -r4.xyzx, r5.xyzx
    r0.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 89: mad r0.xyz, r0.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 90: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r4.xyz, r0.wwww, r0.xyzx
    r4.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 93: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 95: mul r5.xyz, r0.wwww, v6.xyzx
    r5.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r0.w, r5.xyzx, r4.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 97: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 98: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 99: mul r7.xyz, r2.wwww, cb0[27].xyzx
    r7.xyz = ((r2.wwww)*(source[27].xyzx)).xyz;
    // 100: mad r7.xyz, r2.zzzz, cb0[26].xyzx, r7.xyzx
    r7.xyz = ((r2.zzzz)*(source[26].xyzx)+(r7.xyzx)).xyz;
    // 101: mul r7.xyz, r7.xyzx, cb0[28].wwww
    r7.xyz = ((r7.xyzx)*(source[28].wwww)).xyz;
    // 102: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 103: mad r10.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 104: mad r11.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 105: mad r12.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 106: log r2.zw, |r2.xxxy|
    r2.zw = (log2(abs(r2.xxxy))).zw;
    // 107: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 108: mul r0.w, r2.w, cb0[15].x
    r0.w = ((r2.wwww)*(source[15].xxxx)).w;
    // 109: mul r2.z, r2.z, cb0[14].z
    r2.z = ((r2.zzzz)*(source[14].zzzz)).z;
    // 110: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 111: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 112: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 113: min r3.z, r2.x, l(1.000000)
    r3.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 115: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: movc r0.w, r2.y, l(0), r0.w
    r0.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 117: mad r2.xyz, r0.wwww, r11.xyzx, r12.xyzx
    r2.xyz = ((r0.wwww)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 118: mad r2.xyz, r2.xyzx, r0.wwww, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r10.xyzx)).xyz;
    // 119: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 120: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 121: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 122: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 123: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 124: dp2 r10.z, r7.xyxx, cb0[17].xyxx
    r10.z = (dot((r7.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 125: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 126: mul r3.xy, cb0[17].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((source[17].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 127: dp2 r10.x, r7.xyxx, r3.xyxx
    r10.x = (dot((r7.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 128: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r11.x, cb0[18].xyzw, r10.xyzw
    r11.x = (dot((source[18].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 130: dp4 r11.y, cb0[19].xyzw, r10.xyzw
    r11.y = (dot((source[19].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 131: dp4 r11.z, cb0[20].xyzw, r10.xyzw
    r11.z = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 132: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 133: dp4 r13.x, cb0[21].xyzw, r12.xyzw
    r13.x = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[22].xyzw, r12.xyzw
    r13.y = (dot((source[22].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[23].xyzw, r12.xyzw
    r13.z = (dot((source[23].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 137: mul r2.w, r10.y, r10.y
    r2.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 138: mov r7.z, r10.y
    r7.z = (r10.yyyy).z;
    // 139: mad r2.w, r10.x, r10.x, -r2.w
    r2.w = ((r10.xxxx)*(r10.xxxx)+(-(r2.wwww))).w;
    // 140: mad r10.xyz, cb0[24].xyzx, r2.wwww, r11.xyzx
    r10.xyz = ((source[24].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 141: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r10.xyz, r10.xyzx, cb0[16].xyzx
    r10.xyz = ((r10.xyzx)*(source[16].xyzx)).xyz;
    // 143: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[16].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[16].wwww)).xyz;
    // 144: mov_sat r1.w, cb0[13].w
    r1.w = (saturate(source[13].wwww)).w;
    // 145: mad r11.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r11.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 146: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 148: mad r11.xyz, r3.wwww, r11.xyzx, r2.wwww
    r11.xyz = ((r3.wwww)*(r11.xyzx)+(r2.wwww)).xyz;
    // 149: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 151: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 152: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 153: dp3 r2.w, r4.xyzx, r12.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 154: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 155: mad r4.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r4.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 156: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 157: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 158: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: dp2 r4.w, r13.xyxx, r13.xyxx
    r4.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 160: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 161: mad_sat r13.y, r4.w, l(0.300000), r3.z
    r13.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 162: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 163: add r3.z, -r13.y, l(1.000000)
    r3.z = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 164: max r14.xyz, r11.xyzx, r3.zzzz
    r14.xyz = (max(r11.xyzx,r3.zzzz)).xyz;
    // 165: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 166: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 167: add r1.w, r4.z, l(1.000000)
    r1.w = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 170: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 171: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
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
    // 183: mul r2.xyz, r2.xyzx, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)).xyz;
    // 184: mad r2.xyz, -r2.xyzx, r3.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r3.wwww)+(r2.xyzx)).xyz;
    // 185: dp3 r6.x, r6.xyzx, r4.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 186: dp3 r6.y, r9.xyzx, r4.xyzx
    r6.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 187: dp2 r3.x, r6.xyxx, r3.xyxx
    r3.x = (dot((r6.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 188: dp2 r3.z, r6.xyxx, cb0[17].xyxx
    r3.z = (dot((r6.xyxx).xy,(source[17].xyxx).xy).xxxx).z;
    // 189: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 190: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 191: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 192: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 193: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 194: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 195: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 196: dp3 r3.y, r8.xyzx, r4.xyzx
    r3.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 197: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 198: mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t7.xyzw, s6, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 202: mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // 203: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[16].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[16].wwww)).xyz;
    // 204: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 205: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 206: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 207: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 208: mul r4.yzw, r4.yyyy, cb0[27].xxyz
    r4.yzw = ((r4.yyyy)*(source[27].xxyz)).yzw;
    // 209: mad r4.xyz, cb0[26].xyzx, r4.xxxx, r4.yzwy
    r4.xyz = ((source[26].xyzx)*(r4.xxxx)+(r4.yzwy)).xyz;
    // 210: mul r4.xyz, r4.xyzx, cb0[28].wwww
    r4.xyz = ((r4.xyzx)*(source[28].wwww)).xyz;
    // 211: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 212: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 213: mad r2.xyz, r3.xyzx, r13.xzwx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r13.xzwx)+(r2.xyzx)).xyz;
    // 214: mul r3.xyz, r13.xzwx, r3.xyzx
    r3.xyz = ((r13.xzwx)*(r3.xyzx)).xyz;
    // 215: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 216: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
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
    // 226: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 227: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 228: mul r4.xyz, cb0[5].xyzx, cb0[10].wwww
    r4.xyz = ((source[5].xyzx)*(source[10].wwww)).xyz;
    // 229: mad r0.xyz, r3.xyzx, r4.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 230: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 231: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 232: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 233: mad o0.xyz, r1.xyzx, cb0[28].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[28].xyzx)+(r0.xyzx)).xyz;
    // 234: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 235: dp3 r0.x, r7.xyzx, r7.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 236: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 237: mul r0.xyz, r0.xxxx, r7.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)).xyz;
    // 238: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 239: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 240: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 241: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 242: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 243: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 244: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 245: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 246: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 247: ftou r0.x, cb0[25].z
    r0.x = (asfloat((uint4)(source[25].zzzz))).x;
    // 248: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 249: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 250: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 251: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 252: ret
    return output;
}

// source.character.static-map-native-1132.v1 / source program 4c6149abed547443a21eec547bba331d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1132(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
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
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    source[15].w=(g_SourceCharacterTime.xxxx).x;
    source[16]=g_SourceCharacterBaseConstants[16];
    source[16].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[16].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[17]=g_SourceCharacterBaseConstants[17];
    source[18]=g_SourceCharacterBaseConstants[18];
    source[19]=g_SourceCharacterBaseConstants[19];
    source[20]=g_SourceCharacterBaseConstants[20];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[21]=g_SourceCharacterEnvironmentColor;source[22]=g_SourceCharacterEnvironmentRotation;}
    source[34]=1.f;
    source[35]=1.f;
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
    // 6: max r1.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 7: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: mul r2.xy, r2.xyxx, cb0[12].xxxx
    r2.xy = ((r2.xyxx)*(source[12].xxxx)).xy;
    // 12: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 13: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 16: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 20: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 24: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r5.xyz, r1.wwww, v1.xyzx
    r5.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
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
    // 32: dp3 r1.x, r4.xyzx, r1.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 33: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r1.x, r1.x, l(0.500000), cb0[13].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[13].zzzz)).x;
    // 35: mul r1.y, r2.z, r2.z
    r1.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 37: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: mul r1.yz, v4.xxyx, cb0[12].yyyy
    r1.yz = ((v4.xxyx)*(source[12].yyyy)).yz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r1.yzyy, t1.zxyw, s1, l(0.000000)
    r1.yz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 41: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 42: mul r1.w, r4.w, r4.w
    r1.w = ((r4.wwww)*(r4.wwww)).w;
    // 43: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 44: max r1.w, cb0[12].w, l(0.000000)
    r1.w = (max(source[12].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 46: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 47: mad r1.x, r1.x, r2.w, r1.x
    r1.x = ((r1.xxxx)*(r2.wwww)+(r1.xxxx)).x;
    // 48: add r2.w, -r1.w, r1.x
    r2.w = ((-(r1.wwww))+(r1.xxxx)).w;
    // 49: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 51: mad r1.x, -r1.w, r2.w, r1.x
    r1.x = ((-(r1.wwww))*(r2.wwww)+(r1.xxxx)).x;
    // 52: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 53: mad_sat r0.w, r0.w, r1.x, r1.w
    r0.w = (saturate((r0.wwww)*(r1.xxxx)+(r1.wwww))).w;
    // 54: mul r7.xyz, cb0[11].xyzx, cb0[17].xxxx
    r7.xyz = ((source[11].xyzx)*(source[17].xxxx)).xyz;
    // 55: mul r8.xyz, r4.xyzx, r7.xyzx
    r8.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 56: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 57: mad r4.xyz, -r7.xyzx, r4.xyzx, r1.xxxx
    r4.xyz = ((-(r7.xyzx))*(r4.xyzx)+(r1.xxxx)).xyz;
    // 58: mad r4.xyz, cb0[17].zzzz, r4.xyzx, r8.xyzx
    r4.xyz = ((source[17].zzzz)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 59: mul r7.xyz, cb0[10].xyzx, cb0[16].wwww
    r7.xyz = ((source[10].xyzx)*(source[16].wwww)).xyz;
    // 60: mad r4.xyz, -r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 61: mul r0.xyz, r0.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // 62: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 63: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 64: mul r4.xyz, r0.xyzx, cb0[17].wwww
    r4.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 65: mad r0.xyz, cb0[18].xxxx, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[18].xxxx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r1.x, r7.z, cb0[18].y
    r1.x = ((r7.zzzz)*(source[18].yyyy)).x;
    // 68: mul r7.xy, r7.yxyy, cb0[19].ywyy
    r7.xy = ((r7.yxyy)*(source[19].ywyy)).xy;
    // 69: log r1.w, |r1.x|
    r1.w = (log2(abs(r1.xxxx))).w;
    // 70: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 72: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 73: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 74: min r1.w, r1.x, l(1.000000)
    r1.w = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul_sat r7.w, r1.x, cb2[3].w
    r7.w = (saturate((r1.xxxx)*(passValues[3].wwww))).w;
    // 76: mad r0.xyz, r1.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 77: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 79: mad_sat r4.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: dp2 r0.x, r1.yzyy, r1.yzyy
    r0.x = (dot((r1.yzyy).xy,(r1.yzyy).xy).xxxx).x;
    // 81: mul r1.xy, r1.yzyy, cb0[12].zzzz
    r1.xy = ((r1.yzyy)*(source[12].zzzz)).xy;
    // 82: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 84: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 85: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 86: add r0.xyz, -r2.xyzx, r1.xyzx
    r0.xyz = ((-(r2.xyzx))+(r1.xyzx)).xyz;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 88: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 91: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 92: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 93: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 94: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 95: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 97: mul r2.yzw, r2.yyyy, cb0[32].xxyz
    r2.yzw = ((r2.yyyy)*(source[32].xxyz)).yzw;
    // 98: mad r2.xyz, r2.xxxx, cb0[31].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[31].xyzx)+(r2.yzwy)).xyz;
    // 99: mul r2.xyz, r2.xyzx, cb0[33].wwww
    r2.xyz = ((r2.xyzx)*(source[33].wwww)).xyz;
    // 100: mul r8.xyz, r4.xyzx, r2.xyzx
    r8.xyz = ((r4.xyzx)*(r2.xyzx)).xyz;
    // 101: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 102: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 103: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 104: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 105: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t9.xyzw, s6
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 106: mul r10.xyz, r10.xyzx, cb0[35].xyzx
    r10.xyz = ((r10.xyzx)*(source[35].xyzx)).xyz;
    // 107: dp3 r0.w, r10.xyzx, r9.xyzx
    r0.w = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 108: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s6
    r9.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 109: mul r9.xyz, r9.xyzx, cb0[34].xyzx
    r9.xyz = ((r9.xyzx)*(source[34].xyzx)).xyz;
    // 110: mul r11.xyz, r0.wwww, r9.xyzx
    r11.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 111: mad r8.xyz, r4.xyzx, r11.xyzx, r8.xyzx
    r8.xyz = ((r4.xyzx)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 112: mad r11.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 113: mad r12.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r12.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 114: mad r13.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r13.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 115: log r14.xy, |r7.xyxx|
    r14.xy = (log2(abs(r7.xyxx))).xy;
    // 116: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 117: mul r1.w, r14.y, cb0[20].x
    r1.w = ((r14.yyyy)*(source[20].xxxx)).w;
    // 118: mul r2.w, r14.x, cb0[19].z
    r2.w = ((r14.xxxx)*(source[19].zzzz)).w;
    // 119: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 120: movc r2.w, r7.x, l(0), r2.w
    r2.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 121: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 122: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 123: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 124: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: movc r1.w, r7.y, l(0), r1.w
    r1.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 126: mad r12.xyz, r1.wwww, r12.xyzx, r13.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)+(r13.xyzx)).xyz;
    // 127: mad r11.xyz, r12.xyzx, r1.wwww, r11.xyzx
    r11.xyz = ((r12.xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 128: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 129: max r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = (max(r1.wwww,r11.xyzx)).xyz;
    // 130: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 131: dp3 r11.x, r3.xyzx, r1.xyzx
    r11.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 132: dp3 r11.y, r6.xyzx, r1.xyzx
    r11.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 133: dp2 r12.z, r11.xyxx, cb0[22].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[22].xyxx).xy).xxxx).z;
    // 134: dp3 r12.y, r5.xyzx, r1.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 135: mul r7.xy, cb0[22].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[22].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 136: dp2 r12.x, r11.xyxx, r7.xyxx
    r12.x = (dot((r11.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 137: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 138: dp4 r13.x, cb0[23].xyzw, r12.xyzw
    r13.x = (dot((source[23].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 139: dp4 r13.y, cb0[24].xyzw, r12.xyzw
    r13.y = (dot((source[24].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 140: dp4 r13.z, cb0[25].xyzw, r12.xyzw
    r13.z = (dot((source[25].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 141: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 142: dp4 r15.x, cb0[26].xyzw, r14.xyzw
    r15.x = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 143: dp4 r15.y, cb0[27].xyzw, r14.xyzw
    r15.y = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 144: dp4 r15.z, cb0[28].xyzw, r14.xyzw
    r15.z = (dot((source[28].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 145: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 146: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 147: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 148: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 149: mad r12.xyz, cb0[29].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[29].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 150: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 151: mul r12.xyz, r12.xyzx, cb0[21].xyzx
    r12.xyz = ((r12.xyzx)*(source[21].xyzx)).xyz;
    // 152: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[21].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[21].wwww)).xyz;
    // 153: mov_sat r4.w, cb0[18].w
    r4.w = (saturate(source[18].wwww)).w;
    // 154: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 155: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 156: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 157: mad r13.xyz, r7.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 158: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 159: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 160: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 161: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 162: dp3 r3.w, r1.xyzx, r14.xyzx
    r3.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 163: mul r1.xyz, r1.xyzx, r3.wwww
    r1.xyz = ((r1.xyzx)*(r3.wwww)).xyz;
    // 164: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 165: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 166: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 167: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 169: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 170: mad_sat r15.y, r4.w, l(0.300000), r7.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 171: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 173: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 174: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 175: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 178: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 179: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 180: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 181: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 182: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 183: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 184: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 185: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 186: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 187: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 188: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 189: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 190: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 191: mul r8.xyz, r8.xyzx, r12.xyzx
    r8.xyz = ((r8.xyzx)*(r12.xyzx)).xyz;
    // 192: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 193: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 194: dp3 r3.x, r3.xyzx, r1.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 195: dp3 r3.y, r6.xyzx, r1.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 196: dp2 r6.x, r3.xyxx, r7.xyxx
    r6.x = (dot((r3.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 197: dp2 r6.z, r3.xyxx, cb0[22].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[22].xyxx).xy).xxxx).z;
    // 198: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 199: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 200: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 201: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 202: add r2.w, r1.w, r2.w
    r2.w = ((r1.wwww)+(r2.wwww)).w;
    // 203: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 204: add_sat r1.w, r2.w, l(-1.000000)
    r1.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 205: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 206: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t7.xyzw, s7, r3.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 207: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 208: mul r3.xyz, r3.xyzx, cb0[21].xyzx
    r3.xyz = ((r3.xyzx)*(source[21].xyzx)).xyz;
    // 209: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[21].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[21].wwww)).xyz;
    // 210: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 211: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 212: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 213: mul r1.xyz, r5.xyzx, r5.xyzx
    r1.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 214: dp3 r1.x, r10.xyzx, r1.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 215: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 216: mad r0.w, r7.z, r0.w, r1.x
    r0.w = ((r7.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 217: mad r1.xyz, r9.xyzx, r0.wwww, r2.xyzx
    r1.xyz = ((r9.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 218: mul r2.xyz, r0.wwww, r9.xyzx
    r2.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 219: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 220: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 221: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 222: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 223: mul r5.xyz, r0.wwww, r1.xyzx
    r5.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 224: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 225: div r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)/(r1.xyzx)).xyz;
    // 226: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 227: mul r1.xyz, r3.xyzx, r5.xyzx
    r1.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 228: mad r2.xyz, r1.xyzx, r15.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r8.xyzx)).xyz;
    // 229: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 230: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 231: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 232: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 233: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 234: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 235: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 236: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 237: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 238: mul r1.xyz, r0.yyyy, cb0[3].xyzx
    r1.xyz = ((r0.yyyy)*(source[3].xyzx)).xyz;
    // 239: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 240: movc r1.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 241: mad_sat r0.y, r0.x, cb0[14].z, -cb0[14].w
    r0.y = (saturate((r0.xxxx)*(source[14].zzzz)+(-(source[14].wwww)))).y;
    // 242: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 243: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 244: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 245: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 246: mul r3.xyz, r0.zzzz, cb0[8].xyzx
    r3.xyz = ((r0.zzzz)*(source[8].xyzx)).xyz;
    // 247: movc r3.xyz, r0.yyyy, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 248: add r3.xyz, r3.xyzx, -cb0[8].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[8].xyzx))).xyz;
    // 249: mad r3.xyz, cb0[8].wwww, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((source[8].wwww)*(r3.xyzx)+(source[8].xyzx)).xyz;
    // 250: mul r5.xyz, cb0[9].xyzx, cb0[15].zzzz
    r5.xyz = ((source[9].xyzx)*(source[15].zzzz)).xyz;
    // 251: mul r5.xyz, r5.xyzx, cb0[16].yyyy
    r5.xyz = ((r5.xyzx)*(source[16].yyyy)).xyz;
    // 252: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 253: mul r0.xyz, r0.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 254: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 255: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 256: mul r0.xyz, r0.xyzx, cb0[16].zzzz
    r0.xyz = ((r0.xyzx)*(source[16].zzzz)).xyz;
    // 257: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 258: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 259: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 260: mul r0.xyz, r0.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 261: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 262: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 263: mul r5.xyz, cb0[5].xyzx, cb0[13].wwww
    r5.xyz = ((source[5].xyzx)*(source[13].wwww)).xyz;
    // 264: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 265: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 266: mad r3.xyz, -r3.xyzx, r5.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r1.wwww)).xyz;
    // 267: mad r3.xyz, cb0[14].xxxx, r3.xyzx, r6.xyzx
    r3.xyz = ((source[14].xxxx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 268: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 269: add r5.xyz, -r3.xyzx, r1.wwww
    r5.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 270: mad r3.xyz, cb0[14].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[14].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 271: mad r5.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 272: mad r6.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 273: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 274: mad r0.xyz, r3.xyzx, r5.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 275: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 276: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 277: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 278: mad o0.xyz, r4.xyzx, cb0[33].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[33].xyzx)+(r0.xyzx)).xyz;
    // 279: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 280: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 281: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 282: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 283: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 284: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 285: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 286: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 287: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 288: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 289: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 290: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 291: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 292: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 293: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 294: ftou r0.x, cb0[30].z
    r0.x = (asfloat((uint4)(source[30].zzzz))).x;
    // 295: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 296: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 297: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 298: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 299: ret
    return output;
}

// source.character.static-map-native-1132.v1 / source program 5b751f3a9789474b92d0ac130caae5a8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1132(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1132(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
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
    source[14]=g_SourceCharacterBaseConstants[13];
    source[15]=g_SourceCharacterBaseConstants[14];
    source[15].w=(g_SourceCharacterTime.xxxx).x;
    source[16]=g_SourceCharacterBaseConstants[16];
    source[16].x=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[16].y=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[17]=g_SourceCharacterBaseConstants[17];
    source[18]=g_SourceCharacterBaseConstants[18];
    source[19]=g_SourceCharacterBaseConstants[19];
    source[20]=g_SourceCharacterBaseConstants[20];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[21]=g_SourceCharacterEnvironmentColor;source[22]=g_SourceCharacterEnvironmentRotation;}
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
    // 6: max r1.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 7: min r1.xyz, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: mul r2.xy, r2.xyxx, cb0[12].xxxx
    r2.xy = ((r2.xyxx)*(source[12].xxxx)).xy;
    // 12: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 13: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 14: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 15: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 16: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 20: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r3.xyz, r1.wwww, v0.xyzx
    r3.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 23: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 24: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r5.xyz, r1.wwww, v1.xyzx
    r5.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
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
    // 32: dp3 r1.x, r4.xyzx, r1.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 33: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r1.x, r1.x, l(0.500000), cb0[13].z
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[13].zzzz)).x;
    // 35: mul r1.y, r2.z, r2.z
    r1.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: mul_sat r0.w, r0.w, r1.y
    r0.w = (saturate((r0.wwww)*(r1.yyyy))).w;
    // 37: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: mul r1.yz, v4.xxyx, cb0[12].yyyy
    r1.yz = ((v4.xxyx)*(source[12].yyyy)).yz;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.yz, r1.yzyy, t1.zxyw, s1, l(0.000000)
    r1.yz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 41: mad r1.yz, r1.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r1.yz = ((r1.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 42: mul r1.w, r4.w, r4.w
    r1.w = ((r4.wwww)*(r4.wwww)).w;
    // 43: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 44: max r1.w, cb0[12].w, l(0.000000)
    r1.w = (max(source[12].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 46: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 47: mad r1.x, r1.x, r2.w, r1.x
    r1.x = ((r1.xxxx)*(r2.wwww)+(r1.xxxx)).x;
    // 48: add r2.w, -r1.w, r1.x
    r2.w = ((-(r1.wwww))+(r1.xxxx)).w;
    // 49: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 51: mad r1.x, -r1.w, r2.w, r1.x
    r1.x = ((-(r1.wwww))*(r2.wwww)+(r1.xxxx)).x;
    // 52: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 53: mad_sat r0.w, r0.w, r1.x, r1.w
    r0.w = (saturate((r0.wwww)*(r1.xxxx)+(r1.wwww))).w;
    // 54: mul r7.xyz, cb0[11].xyzx, cb0[17].xxxx
    r7.xyz = ((source[11].xyzx)*(source[17].xxxx)).xyz;
    // 55: mul r8.xyz, r4.xyzx, r7.xyzx
    r8.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 56: dp3 r1.x, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 57: mad r4.xyz, -r7.xyzx, r4.xyzx, r1.xxxx
    r4.xyz = ((-(r7.xyzx))*(r4.xyzx)+(r1.xxxx)).xyz;
    // 58: mad r4.xyz, cb0[17].zzzz, r4.xyzx, r8.xyzx
    r4.xyz = ((source[17].zzzz)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 59: mul r7.xyz, cb0[10].xyzx, cb0[16].wwww
    r7.xyz = ((source[10].xyzx)*(source[16].wwww)).xyz;
    // 60: mad r4.xyz, -r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 61: mul r0.xyz, r0.xyzx, r7.xyzx
    r0.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // 62: mad r0.xyz, r0.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 63: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 64: mul r4.xyz, r0.xyzx, cb0[17].wwww
    r4.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 65: mad r0.xyz, cb0[18].xxxx, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[18].xxxx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 67: mul r1.x, r7.z, cb0[18].y
    r1.x = ((r7.zzzz)*(source[18].yyyy)).x;
    // 68: mul r7.xy, r7.yxyy, cb0[19].ywyy
    r7.xy = ((r7.yxyy)*(source[19].ywyy)).xy;
    // 69: log r1.w, |r1.x|
    r1.w = (log2(abs(r1.xxxx))).w;
    // 70: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 71: mul r1.w, r1.w, cb0[18].z
    r1.w = ((r1.wwww)*(source[18].zzzz)).w;
    // 72: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 73: movc r1.x, r1.x, l(0), r1.w
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).x;
    // 74: min r1.w, r1.x, l(1.000000)
    r1.w = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mul_sat r7.w, r1.x, cb2[3].w
    r7.w = (saturate((r1.xxxx)*(passValues[3].wwww))).w;
    // 76: mad r0.xyz, r1.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 77: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 79: mad_sat r4.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 80: dp2 r0.x, r1.yzyy, r1.yzyy
    r0.x = (dot((r1.yzyy).xy,(r1.yzyy).xy).xxxx).x;
    // 81: mul r1.xy, r1.yzyy, cb0[12].zzzz
    r1.xy = ((r1.yzyy)*(source[12].zzzz)).xy;
    // 82: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 84: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 85: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 86: add r0.xyz, -r2.xyzx, r1.xyzx
    r0.xyz = ((-(r2.xyzx))+(r1.xyzx)).xyz;
    // 87: mad r0.xyz, r0.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 88: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 91: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 92: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 93: mul r2.xyz, r0.wwww, v6.xyzx
    r2.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 94: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 95: mad r8.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 97: mul r8.yzw, r8.yyyy, cb0[32].xxyz
    r8.yzw = ((r8.yyyy)*(source[32].xxyz)).yzw;
    // 98: mad r8.xyz, r8.xxxx, cb0[31].xyzx, r8.yzwy
    r8.xyz = ((r8.xxxx)*(source[31].xyzx)+(r8.yzwy)).xyz;
    // 99: mul r8.xyz, r8.xyzx, cb0[33].wwww
    r8.xyz = ((r8.xyzx)*(source[33].wwww)).xyz;
    // 100: mul r8.xyz, r4.xyzx, r8.xyzx
    r8.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 101: mad r9.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 102: mad r10.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 103: mad r11.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 104: log r12.xy, |r7.xyxx|
    r12.xy = (log2(abs(r7.xyxx))).xy;
    // 105: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 106: mul r0.w, r12.y, cb0[20].x
    r0.w = ((r12.yyyy)*(source[20].xxxx)).w;
    // 107: mul r1.w, r12.x, cb0[19].z
    r1.w = ((r12.xxxx)*(source[19].zzzz)).w;
    // 108: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 109: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 110: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 111: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 112: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 113: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 114: movc r0.w, r7.y, l(0), r0.w
    r0.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 115: mad r10.xyz, r0.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 116: mad r9.xyz, r10.xyzx, r0.wwww, r9.xyzx
    r9.xyz = ((r10.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 117: mul r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 118: max r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = (max(r0.wwww,r9.xyzx)).xyz;
    // 119: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 120: dp3 r9.x, r3.xyzx, r1.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 121: dp3 r9.y, r6.xyzx, r1.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 122: dp2 r10.z, r9.xyxx, cb0[22].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[22].xyxx).xy).xxxx).z;
    // 123: dp3 r10.y, r5.xyzx, r1.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 124: mul r7.xy, cb0[22].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[22].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 125: dp2 r10.x, r9.xyxx, r7.xyxx
    r10.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 126: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 127: dp4 r11.x, cb0[23].xyzw, r10.xyzw
    r11.x = (dot((source[23].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 128: dp4 r11.y, cb0[24].xyzw, r10.xyzw
    r11.y = (dot((source[24].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 129: dp4 r11.z, cb0[25].xyzw, r10.xyzw
    r11.z = (dot((source[25].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 130: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 131: dp4 r13.x, cb0[26].xyzw, r12.xyzw
    r13.x = (dot((source[26].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 132: dp4 r13.y, cb0[27].xyzw, r12.xyzw
    r13.y = (dot((source[27].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 133: dp4 r13.z, cb0[28].xyzw, r12.xyzw
    r13.z = (dot((source[28].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 134: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 135: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 136: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 137: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 138: mad r10.xyz, cb0[29].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[29].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 139: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 140: mul r10.xyz, r10.xyzx, cb0[21].xyzx
    r10.xyz = ((r10.xyzx)*(source[21].xyzx)).xyz;
    // 141: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[21].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[21].wwww)).xyz;
    // 142: mov_sat r4.w, cb0[18].w
    r4.w = (saturate(source[18].wwww)).w;
    // 143: mad r11.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r11.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 144: mul r1.w, r4.w, l(0.080000)
    r1.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 145: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 146: mad r11.xyz, r7.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r7.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 147: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 148: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 149: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 150: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 151: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 152: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 153: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 154: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 155: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 156: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 158: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 159: mad_sat r13.y, r3.w, l(0.300000), r7.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 160: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 161: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 163: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 164: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 165: add r1.w, r1.z, l(1.000000)
    r1.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 168: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 169: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 170: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 171: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 172: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 173: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 174: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 175: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 176: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 177: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 178: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 179: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 180: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 181: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 182: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 183: dp3 r3.x, r3.xyzx, r1.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 184: dp3 r3.y, r6.xyzx, r1.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 185: dp2 r6.x, r3.xyxx, r7.xyxx
    r6.x = (dot((r3.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 186: dp2 r6.z, r3.xyxx, cb0[22].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[22].xyxx).xy).xxxx).z;
    // 187: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 188: mul r3.x, r13.y, r13.y
    r3.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 189: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 190: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 191: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 192: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 193: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 194: dp3 r6.y, r5.xyzx, r1.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 195: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 196: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 197: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 198: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r6.xyzx, t7.xyzw, s6, r2.w
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 199: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 200: mul r2.xyz, r2.xyzx, cb0[21].xyzx
    r2.xyz = ((r2.xyzx)*(source[21].xyzx)).xyz;
    // 201: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[21].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[21].wwww)).xyz;
    // 202: mad r1.z, r0.w, r11.x, r11.y
    r1.z = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).z;
    // 203: mad r1.z, r1.z, r0.w, r11.z
    r1.z = ((r1.zzzz)*(r0.wwww)+(r11.zzzz)).z;
    // 204: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 205: max r0.w, r0.w, r1.z
    r0.w = (max(r0.wwww,r1.zzzz)).w;
    // 206: mul r1.yzw, r1.yyyy, cb0[32].xxyz
    r1.yzw = ((r1.yyyy)*(source[32].xxyz)).yzw;
    // 207: mad r1.xyz, cb0[31].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[31].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 208: mul r1.xyz, r1.xyzx, cb0[33].wwww
    r1.xyz = ((r1.xyzx)*(source[33].wwww)).xyz;
    // 209: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 210: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 211: mad r2.xyz, r1.xyzx, r13.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r13.xzwx)+(r8.xyzx)).xyz;
    // 212: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 213: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 214: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 215: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 216: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 217: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 218: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 219: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 220: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 221: mul r0.yzw, r0.yyyy, cb0[3].xxyz
    r0.yzw = ((r0.yyyy)*(source[3].xxyz)).yzw;
    // 222: lt r1.x, |r0.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 223: movc r0.yzw, r1.xxxx, l(0,0,0,0), r0.yyzw
    r0.yzw = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyzw)).yzw;
    // 224: mad_sat r1.x, r0.x, cb0[14].z, -cb0[14].w
    r1.x = (saturate((r0.xxxx)*(source[14].zzzz)+(-(source[14].wwww)))).x;
    // 225: log r1.y, r1.x
    r1.y = (log2(r1.xxxx)).y;
    // 226: lt r1.x, r1.x, l(0.000001)
    r1.x = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 227: mul r1.y, r1.y, cb0[15].x
    r1.y = ((r1.yyyy)*(source[15].xxxx)).y;
    // 228: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 229: mul r1.yzw, r1.yyyy, cb0[8].xxyz
    r1.yzw = ((r1.yyyy)*(source[8].xxyz)).yzw;
    // 230: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 231: add r1.xyz, r1.xyzx, -cb0[8].xyzx
    r1.xyz = ((r1.xyzx)+(-(source[8].xyzx))).xyz;
    // 232: mad r1.xyz, cb0[8].wwww, r1.xyzx, cb0[8].xyzx
    r1.xyz = ((source[8].wwww)*(r1.xyzx)+(source[8].xyzx)).xyz;
    // 233: mul r3.xyz, cb0[9].xyzx, cb0[15].zzzz
    r3.xyz = ((source[9].xyzx)*(source[15].zzzz)).xyz;
    // 234: mul r3.xyz, r3.xyzx, cb0[16].yyyy
    r3.xyz = ((r3.xyzx)*(source[16].yyyy)).xyz;
    // 235: mul r3.xyz, r0.xxxx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 236: mul r3.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 237: max r3.xyz, |r3.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r3.xyz = (max(abs(r3.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 238: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 239: mul r3.xyz, r3.xyzx, cb0[16].zzzz
    r3.xyz = ((r3.xyzx)*(source[16].zzzz)).xyz;
    // 240: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 241: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 242: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 243: mul r1.xyz, r1.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 244: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t5.xyzw, s4, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 246: mul r5.xyz, cb0[5].xyzx, cb0[13].wwww
    r5.xyz = ((source[5].xyzx)*(source[13].wwww)).xyz;
    // 247: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 248: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 249: mad r3.xyz, -r3.xyzx, r5.xyzx, r0.xxxx
    r3.xyz = ((-(r3.xyzx))*(r5.xyzx)+(r0.xxxx)).xyz;
    // 250: mad r3.xyz, cb0[14].xxxx, r3.xyzx, r6.xyzx
    r3.xyz = ((source[14].xxxx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 251: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 252: add r5.xyz, -r3.xyzx, r0.xxxx
    r5.xyz = ((-(r3.xyzx))+(r0.xxxx)).xyz;
    // 253: mad r3.xyz, cb0[14].yyyy, r5.xyzx, r3.xyzx
    r3.xyz = ((source[14].yyyy)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 254: mad r5.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 255: mad r6.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 256: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 257: mad r1.xyz, r3.xyzx, r5.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 258: add r0.xyz, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 259: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 260: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 261: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 262: mad o0.xyz, r4.xyzx, cb0[33].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[33].xyzx)+(r0.xyzx)).xyz;
    // 263: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 264: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 265: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 266: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 267: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 268: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 269: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 270: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 271: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 272: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 273: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 274: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 275: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 276: ftou r0.x, cb0[30].z
    r0.x = (asfloat((uint4)(source[30].zzzz))).x;
    // 277: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 278: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 279: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 280: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 281: ret
    return output;
}

// source.character.static-map-native-1133.v1 / source program e4dab1e47b8fbb458abeb06bfc26df5d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1133(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[6].yyww
    r2.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[6].zzzz
    r0.zw = ((r0.zzzw)*(source[6].zzzz)).zw;
    // 12: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: mul r4.xyz, cb0[5].xyzx, cb0[8].zzzz
    r4.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 38: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 39: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 41: mad r3.xyz, cb0[9].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 42: mul r4.xyz, cb0[3].xyzx, cb0[7].zzzz
    r4.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 43: mul r4.xyz, r1.xyzx, r4.xyzx
    r4.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 44: mul r5.xyz, cb0[4].xyzx, cb0[7].wwww
    r5.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 45: mad r1.xyz, r5.xyzx, r1.xyzx, -r4.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)+(-(r4.xyzx))).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r1.w, r5.z, cb0[8].x
    r1.w = ((r5.zzzz)*(source[8].xxxx)).w;
    // 48: mul r2.zw, r5.yyyx, cb0[10].yyyw
    r2.zw = ((r5.yyyx)*(source[10].yyyw)).zw;
    // 49: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 50: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: mul r3.w, r3.w, cb0[8].y
    r3.w = ((r3.wwww)*(source[8].yyyy)).w;
    // 52: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 53: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 54: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mul_sat r5.w, r1.w, cb2[3].w
    r5.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 56: mad r1.xyz, r3.wwww, r1.xyzx, r4.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 57: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 58: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 59: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 60: mul r3.xyz, r1.xyzx, cb0[9].yyyy
    r3.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 61: mad r1.xyz, cb0[9].zzzz, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[9].zzzz)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 62: mad r1.xyz, r3.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 63: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 64: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 65: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 66: dp2 r3.x, r2.xyxx, r2.xyxx
    r3.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 67: mul r4.xy, r2.xyxx, cb0[7].xxxx
    r4.xy = ((r2.xyxx)*(source[7].xxxx)).xy;
    // 68: add r2.x, -r3.x, l(1.000000)
    r2.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 69: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 70: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 71: add r4.z, r2.x, l(0.000010)
    r4.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 72: add r3.xyz, -r0.xyzx, r4.xyzx
    r3.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 73: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 74: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 75: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 76: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 77: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 80: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 81: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 83: mul r4.xyz, r2.yyyy, cb0[23].xyzx
    r4.xyz = ((r2.yyyy)*(source[23].xyzx)).xyz;
    // 84: mad r4.xyz, r2.xxxx, cb0[22].xyzx, r4.xyzx
    r4.xyz = ((r2.xxxx)*(source[22].xyzx)+(r4.xyzx)).xyz;
    // 85: mul r4.xyz, r4.xyzx, cb0[24].wwww
    r4.xyz = ((r4.xyzx)*(source[24].wwww)).xyz;
    // 86: mul r6.xyz, r1.xyzx, r4.xyzx
    r6.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 87: dp2_sat r7.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 88: dp3_sat r7.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 89: dp3_sat r7.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 90: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 91: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t9.xyzw, s6
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 92: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 93: dp3 r0.w, r8.xyzx, r7.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 94: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s6
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 95: mul r7.xyz, r7.xyzx, cb0[25].xyzx
    r7.xyz = ((r7.xyzx)*(source[25].xyzx)).xyz;
    // 96: mul r9.xyz, r0.wwww, r7.xyzx
    r9.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 97: mad r6.xyz, r1.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 98: mad r9.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 99: mad r10.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 100: mad r11.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 101: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 102: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 103: mul r2.y, r2.y, cb0[11].x
    r2.y = ((r2.yyyy)*(source[11].xxxx)).y;
    // 104: mul r2.x, r2.x, cb0[10].z
    r2.x = ((r2.xxxx)*(source[10].zzzz)).x;
    // 105: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 106: movc r2.x, r2.z, l(0), r2.x
    r2.x = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 107: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 108: min r5.z, r2.x, l(1.000000)
    r5.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 109: exp r2.x, r2.y
    r2.x = (exp2(r2.yyyy)).x;
    // 110: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: movc r2.x, r2.w, l(0), r2.x
    r2.x = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 112: mad r2.yzw, r2.xxxx, r10.xxyz, r11.xxyz
    r2.yzw = ((r2.xxxx)*(r10.xxyz)+(r11.xxyz)).yzw;
    // 113: mad r2.yzw, r2.yyzw, r2.xxxx, r9.xxyz
    r2.yzw = ((r2.yyzw)*(r2.xxxx)+(r9.xxyz)).yzw;
    // 114: mul r2.yzw, r2.xxxx, r2.yyzw
    r2.yzw = ((r2.xxxx)*(r2.yyzw)).yzw;
    // 115: max r2.yzw, r2.yyzw, r2.xxxx
    r2.yzw = (max(r2.yyzw,r2.xxxx)).yzw;
    // 116: mul r2.yzw, r2.yyzw, r6.xxyz
    r2.yzw = ((r2.yyzw)*(r6.xxyz)).yzw;
    // 117: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 118: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 119: mul r6.xyz, r3.wwww, v1.xyzx
    r6.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 120: dp3 r9.y, r6.xyzx, r3.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 121: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 122: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 123: mul r10.xyz, r3.wwww, v0.xyzx
    r10.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 124: mul r11.xyz, r6.zxyz, r10.yzxy
    r11.xyz = ((r6.zxyz)*(r10.yzxy)).xyz;
    // 125: mad r11.xyz, r6.yzxy, r10.zxyz, -r11.xyzx
    r11.xyz = ((r6.yzxy)*(r10.zxyz)+(-(r11.xyzx))).xyz;
    // 126: mul r11.xyz, r11.xyzx, v1.wwww
    r11.xyz = ((r11.xyzx)*(v1.wwww)).xyz;
    // 127: dp3 r12.y, r11.xyzx, r3.xyzx
    r12.y = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 128: dp3 r12.x, r10.xyzx, r3.xyzx
    r12.x = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 129: dp2 r9.z, r12.xyxx, cb0[13].xyxx
    r9.z = (dot((r12.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 130: mul r5.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 131: dp2 r9.x, r12.xyxx, r5.xyxx
    r9.x = (dot((r12.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 132: mov r9.w, l(1.000000)
    r9.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 133: dp4 r13.x, cb0[14].xyzw, r9.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[15].xyzw, r9.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[16].xyzw, r9.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 136: mul r14.xyzw, r9.yzzx, r9.xyzz
    r14.xyzw = ((r9.yzzx)*(r9.xyzz)).xyzw;
    // 137: dp4 r15.x, cb0[17].xyzw, r14.xyzw
    r15.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 138: dp4 r15.y, cb0[18].xyzw, r14.xyzw
    r15.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 139: dp4 r15.z, cb0[19].xyzw, r14.xyzw
    r15.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 140: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 141: mul r3.w, r9.y, r9.y
    r3.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 142: mov r12.z, r9.y
    r12.z = (r9.yyyy).z;
    // 143: mad r3.w, r9.x, r9.x, -r3.w
    r3.w = ((r9.xxxx)*(r9.xxxx)+(-(r3.wwww))).w;
    // 144: mad r9.xyz, cb0[20].xyzx, r3.wwww, r13.xyzx
    r9.xyz = ((source[20].xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 145: max r9.xyz, r9.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r9.xyz = (max(r9.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 146: mul r9.xyz, r9.xyzx, cb0[12].xyzx
    r9.xyz = ((r9.xyzx)*(source[12].xyzx)).xyz;
    // 147: mad r9.xyz, r9.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r9.xyz = ((r9.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 148: mov_sat r1.w, cb0[9].w
    r1.w = (saturate(source[9].wwww)).w;
    // 149: mad r13.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r13.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 150: mul r3.w, r1.w, l(0.080000)
    r3.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 151: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 152: mad r13.xyz, r5.wwww, r13.xyzx, r3.wwww
    r13.xyz = ((r5.wwww)*(r13.xyzx)+(r3.wwww)).xyz;
    // 153: mul_sat r1.w, r13.y, l(50.000000)
    r1.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 154: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 155: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 156: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 157: dp3 r3.w, r3.xyzx, r14.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 158: mul r3.xyz, r3.wwww, r3.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)).xyz;
    // 159: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
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
    // 165: mad_sat r15.y, r4.w, l(0.300000), r5.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r5.zzzz))).y;
    // 166: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 168: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 169: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 170: add r1.w, r3.z, l(1.000000)
    r1.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: add_sat r15.x, -r1.w, r3.w
    r15.x = (saturate((-(r1.wwww))+(r3.wwww))).x;
    // 173: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 174: add r1.w, r2.x, r15.x
    r1.w = ((r2.xxxx)+(r15.xxxx)).w;
    // 175: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 176: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 177: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 178: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
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
    // 185: mul r9.xyz, r9.xyzx, r17.xyzx
    r9.xyz = ((r9.xyzx)*(r17.xyzx)).xyz;
    // 186: mul r2.yzw, r2.yyzw, r9.xxyz
    r2.yzw = ((r2.yyzw)*(r9.xxyz)).yzw;
    // 187: mad r2.yzw, -r2.yyzw, r5.wwww, r2.yyzw
    r2.yzw = ((-(r2.yyzw))*(r5.wwww)+(r2.yyzw)).yzw;
    // 188: mov o2.zw, r5.zzzw
    output.targets[2].zw = (r5.zzzw).zw;
    // 189: dp2_sat r9.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 190: dp3_sat r9.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 191: dp3_sat r9.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 192: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 193: dp3 r3.w, r8.xyzx, r9.xyzx
    r3.w = (dot((r8.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 194: add r0.w, r0.w, -r3.w
    r0.w = ((r0.wwww)+(-(r3.wwww))).w;
    // 195: mad r0.w, r5.z, r0.w, r3.w
    r0.w = ((r5.zzzz)*(r0.wwww)+(r3.wwww)).w;
    // 196: mad r4.xyz, r7.xyzx, r0.wwww, r4.xyzx
    r4.xyz = ((r7.xyzx)*(r0.wwww)+(r4.xyzx)).xyz;
    // 197: mul r7.xyz, r0.wwww, r7.xyzx
    r7.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 198: mul r0.w, r15.y, r15.y
    r0.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 199: mul r3.w, r15.y, l(5.000000)
    r3.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 200: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 201: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 202: add r0.w, r2.x, r0.w
    r0.w = ((r2.xxxx)+(r0.wwww)).w;
    // 203: mov o5.y, r2.x
    output.targets[5].y = (r2.xxxx).y;
    // 204: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 205: mad r1.w, r0.w, r13.x, r13.y
    r1.w = ((r0.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 206: mad r1.w, r1.w, r0.w, r13.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r13.zzzz)).w;
    // 207: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 208: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 209: mul r8.xyz, r0.wwww, r4.xyzx
    r8.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 210: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 211: div r4.xyz, r7.xyzx, r4.xyzx
    r4.xyz = ((r7.xyzx)/(r4.xyzx)).xyz;
    // 212: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: dp3 r4.x, r10.xyzx, r3.xyzx
    r4.x = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 214: dp3 r4.y, r11.xyzx, r3.xyzx
    r4.y = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 215: dp3 r3.y, r6.xyzx, r3.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 216: dp2 r3.x, r4.xyxx, r5.xyxx
    r3.x = (dot((r4.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 217: dp2 r3.z, r4.xyxx, cb0[13].xyxx
    r3.z = (dot((r4.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 218: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t7.xyzw, s7, r3.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 219: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 220: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 221: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 222: mul r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)*(r3.xyzx)).xyz;
    // 223: mad r2.xyz, r3.xyzx, r15.xzwx, r2.yzwy
    r2.xyz = ((r3.xyzx)*(r15.xzwx)+(r2.yzwy)).xyz;
    // 224: mul r3.xyz, r15.xzwx, r3.xyzx
    r3.xyz = ((r15.xzwx)*(r3.xyzx)).xyz;
    // 225: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 227: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 228: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 229: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 230: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 231: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 232: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 233: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 234: mul r3.xyz, r0.xxxx, cb0[2].xyzx
    r3.xyz = ((r0.xxxx)*(source[2].xyzx)).xyz;
    // 235: movc r0.xyz, r0.yyyy, l(0,0,0,0), r3.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 236: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 237: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 238: mad o0.xyz, r1.xyzx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 239: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 240: dp3 r0.x, r12.xyzx, r12.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 241: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 242: mul r0.xyz, r0.xxxx, r12.xyzx
    r0.xyz = ((r0.xxxx)*(r12.xyzx)).xyz;
    // 243: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 244: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 245: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 246: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 247: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 248: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 249: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 250: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 251: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 252: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 253: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 254: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
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

// source.character.static-map-native-1133.v1 / source program de5e9198a11d2f469aee998a6986094b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1133(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1133(input);
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
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 5: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 6: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 7: add r1.z, r0.z, l(0.000010)
    r1.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 8: mul r2.xyzw, v4.xyxy, cb0[6].yyww
    r2.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r2.xyxx, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 11: mul r0.zw, r0.zzzw, cb0[6].zzzz
    r0.zw = ((r0.zzzw)*(source[6].zzzz)).zw;
    // 12: mad r0.xy, cb0[6].xxxx, r0.xyxx, r0.zwzz
    r0.xy = ((source[6].xxxx)*(r0.xyxx)+(r0.zwzz)).xy;
    // 13: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 14: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 15: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 16: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 17: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 19: mul_sat r0.w, r0.w, r1.w
    r0.w = (saturate((r0.wwww)*(r1.wwww))).w;
    // 20: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.zwzz, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 23: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 24: mul r1.w, r3.w, r3.w
    r1.w = ((r3.wwww)*(r3.wwww)).w;
    // 25: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 26: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 28: mul r2.z, r0.w, r1.w
    r2.z = ((r0.wwww)*(r1.wwww)).z;
    // 29: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r2.z, r2.w, r2.z, r2.w
    r2.z = ((r2.wwww)*(r2.zzzz)+(r2.wwww)).z;
    // 31: add r2.w, -r1.w, r2.z
    r2.w = ((-(r1.wwww))+(r2.zzzz)).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 34: mad r2.z, -r1.w, r2.w, r2.z
    r2.z = ((-(r1.wwww))*(r2.wwww)+(r2.zzzz)).z;
    // 35: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 36: mad_sat r0.w, r0.w, r2.z, r1.w
    r0.w = (saturate((r0.wwww)*(r2.zzzz)+(r1.wwww))).w;
    // 37: mul r4.xyz, cb0[5].xyzx, cb0[8].zzzz
    r4.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 38: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 39: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 40: mad r3.xyz, -r4.xyzx, r3.xyzx, r1.wwww
    r3.xyz = ((-(r4.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 41: mad r3.xyz, cb0[9].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 42: mul r4.xyz, cb0[3].xyzx, cb0[7].zzzz
    r4.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 43: mul r4.xyz, r1.xyzx, r4.xyzx
    r4.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 44: mul r5.xyz, cb0[4].xyzx, cb0[7].wwww
    r5.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 45: mad r1.xyz, r5.xyzx, r1.xyzx, -r4.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)+(-(r4.xyzx))).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r1.w, r5.z, cb0[8].x
    r1.w = ((r5.zzzz)*(source[8].xxxx)).w;
    // 48: mul r2.zw, r5.yyyx, cb0[10].yyyw
    r2.zw = ((r5.yyyx)*(source[10].yyyw)).zw;
    // 49: log r3.w, |r1.w|
    r3.w = (log2(abs(r1.wwww))).w;
    // 50: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 51: mul r3.w, r3.w, cb0[8].y
    r3.w = ((r3.wwww)*(source[8].yyyy)).w;
    // 52: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 53: movc r1.w, r1.w, l(0), r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 54: min r3.w, r1.w, l(1.000000)
    r3.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: mul_sat r5.w, r1.w, cb2[3].w
    r5.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 56: mad r1.xyz, r3.wwww, r1.xyzx, r4.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r4.xyzx)).xyz;
    // 57: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 58: mad r1.xyz, r0.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 59: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 60: mul r3.xyz, r1.xyzx, cb0[9].yyyy
    r3.xyz = ((r1.xyzx)*(source[9].yyyy)).xyz;
    // 61: mad r1.xyz, cb0[9].zzzz, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[9].zzzz)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 62: mad r1.xyz, r3.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r3.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 63: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 64: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 65: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 66: dp2 r3.x, r2.xyxx, r2.xyxx
    r3.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 67: mul r4.xy, r2.xyxx, cb0[7].xxxx
    r4.xy = ((r2.xyxx)*(source[7].xxxx)).xy;
    // 68: add r2.x, -r3.x, l(1.000000)
    r2.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 69: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 70: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 71: add r4.z, r2.x, l(0.000010)
    r4.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 72: add r3.xyz, -r0.xyzx, r4.xyzx
    r3.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 73: mad r0.xyz, r0.wwww, r3.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 74: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 75: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 76: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 77: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 80: dp3 r0.w, r4.xyzx, r3.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 81: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 83: mul r6.xyz, r2.yyyy, cb0[23].xyzx
    r6.xyz = ((r2.yyyy)*(source[23].xyzx)).xyz;
    // 84: mad r6.xyz, r2.xxxx, cb0[22].xyzx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(source[22].xyzx)+(r6.xyzx)).xyz;
    // 85: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 86: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 87: mad r7.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r7.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 88: mad r8.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 89: mad r9.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 90: log r2.xy, |r2.zwzz|
    r2.xy = (log2(abs(r2.zwzz))).xy;
    // 91: lt r2.zw, |r2.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r2.zw = (asfloat((uint4)((abs(r2.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 92: mul r0.w, r2.y, cb0[11].x
    r0.w = ((r2.yyyy)*(source[11].xxxx)).w;
    // 93: mul r2.x, r2.x, cb0[10].z
    r2.x = ((r2.xxxx)*(source[10].zzzz)).x;
    // 94: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 95: movc r2.x, r2.z, l(0), r2.x
    r2.x = ((asuint(r2.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 96: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 97: min r5.z, r2.x, l(1.000000)
    r5.z = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 98: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 99: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 100: movc r0.w, r2.w, l(0), r0.w
    r0.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 101: mad r2.xyz, r0.wwww, r8.xyzx, r9.xyzx
    r2.xyz = ((r0.wwww)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 102: mad r2.xyz, r2.xyzx, r0.wwww, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 103: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 104: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 105: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 106: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 107: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 108: mul r6.xyz, r2.wwww, v1.xyzx
    r6.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 109: dp3 r7.y, r6.xyzx, r3.xyzx
    r7.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 110: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 111: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 112: mul r8.xyz, r2.wwww, v0.xyzx
    r8.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 113: mul r9.xyz, r6.zxyz, r8.yzxy
    r9.xyz = ((r6.zxyz)*(r8.yzxy)).xyz;
    // 114: mad r9.xyz, r6.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r6.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 115: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 116: dp3 r10.y, r9.xyzx, r3.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 117: dp3 r10.x, r8.xyzx, r3.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 118: dp2 r7.z, r10.xyxx, cb0[13].xyxx
    r7.z = (dot((r10.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 119: mul r5.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 120: dp2 r7.x, r10.xyxx, r5.xyxx
    r7.x = (dot((r10.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 121: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 122: dp4 r11.x, cb0[14].xyzw, r7.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 123: dp4 r11.y, cb0[15].xyzw, r7.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 124: dp4 r11.z, cb0[16].xyzw, r7.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 125: mul r12.xyzw, r7.yzzx, r7.xyzz
    r12.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 126: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 127: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 128: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 129: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 130: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 131: mov r10.z, r7.y
    r10.z = (r7.yyyy).z;
    // 132: mad r2.w, r7.x, r7.x, -r2.w
    r2.w = ((r7.xxxx)*(r7.xxxx)+(-(r2.wwww))).w;
    // 133: mad r7.xyz, cb0[20].xyzx, r2.wwww, r11.xyzx
    r7.xyz = ((source[20].xyzx)*(r2.wwww)+(r11.xyzx)).xyz;
    // 134: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r7.xyz, r7.xyzx, cb0[12].xyzx
    r7.xyz = ((r7.xyzx)*(source[12].xyzx)).xyz;
    // 136: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 137: mov_sat r1.w, cb0[9].w
    r1.w = (saturate(source[9].wwww)).w;
    // 138: mad r11.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r11.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 139: mul r2.w, r1.w, l(0.080000)
    r2.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 140: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 141: mad r11.xyz, r5.wwww, r11.xyzx, r2.wwww
    r11.xyz = ((r5.wwww)*(r11.xyzx)+(r2.wwww)).xyz;
    // 142: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 143: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 144: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 145: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 146: dp3 r2.w, r3.xyzx, r12.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 147: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 148: mad r3.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r3.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
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
    // 154: mad_sat r13.y, r3.w, l(0.300000), r5.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r5.zzzz))).y;
    // 155: mov o2.zw, r5.zzzw
    output.targets[2].zw = (r5.zzzw).zw;
    // 156: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 158: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 159: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 160: add r1.w, r3.z, l(1.000000)
    r1.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 163: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t6.zwxy, s7
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 164: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 165: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 166: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 167: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 168: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
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
    // 175: mul r7.xyz, r7.xyzx, r15.xyzx
    r7.xyz = ((r7.xyzx)*(r15.xyzx)).xyz;
    // 176: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 177: mad r2.xyz, -r2.xyzx, r5.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r5.wwww)+(r2.xyzx)).xyz;
    // 178: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 179: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 180: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 181: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 182: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 183: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 184: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 185: dp3 r7.x, r8.xyzx, r3.xyzx
    r7.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 186: dp3 r7.y, r9.xyzx, r3.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 187: dp2 r5.x, r7.xyxx, r5.xyxx
    r5.x = (dot((r7.xyxx).xy,(r5.xyxx).xy).xxxx).x;
    // 188: dp2 r5.z, r7.xyxx, cb0[13].xyxx
    r5.z = (dot((r7.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 189: dp3 r5.y, r6.xyzx, r3.xyzx
    r5.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 190: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 191: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 192: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 193: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r5.xyzx, t7.xyzw, s6, r2.w
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 194: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 195: mul r4.xyz, r4.xyzx, cb0[12].xyzx
    r4.xyz = ((r4.xyzx)*(source[12].xyzx)).xyz;
    // 196: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 197: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 198: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 199: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 200: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 201: mul r3.yzw, r3.yyyy, cb0[23].xxyz
    r3.yzw = ((r3.yyyy)*(source[23].xxyz)).yzw;
    // 202: mad r3.xyz, cb0[22].xyzx, r3.xxxx, r3.yzwy
    r3.xyz = ((source[22].xyzx)*(r3.xxxx)+(r3.yzwy)).xyz;
    // 203: mul r3.xyz, r3.xyzx, cb0[24].wwww
    r3.xyz = ((r3.xyzx)*(source[24].wwww)).xyz;
    // 204: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 205: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 206: mad r2.xyz, r3.xyzx, r13.xzwx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r13.xzwx)+(r2.xyzx)).xyz;
    // 207: mul r3.xyz, r13.xzwx, r3.xyzx
    r3.xyz = ((r13.xzwx)*(r3.xyzx)).xyz;
    // 208: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 209: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 210: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 211: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 212: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 213: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 214: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 215: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 216: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 217: mul r0.xzw, r0.xxxx, cb0[2].xxyz
    r0.xzw = ((r0.xxxx)*(source[2].xxyz)).xzw;
    // 218: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 219: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 220: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 221: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 222: mad o0.xyz, r1.xyzx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 223: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 224: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 225: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 226: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
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
    // 236: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
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

// source.character.static-map-native-1134.v1 / source program 1f711e3d11837a409437fa95896740be
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1134(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[11]=g_SourceCharacterEnvironmentColor;source[12]=g_SourceCharacterEnvironmentRotation;}
    source[24]=1.f;
    source[25]=1.f;
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
    // 23: mul r3.xyz, r0.yyyy, cb0[22].xyzx
    r3.xyz = ((r0.yyyy)*(source[22].xyzx)).xyz;
    // 24: mad r0.xyz, r0.xxxx, cb0[21].xyzx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(source[21].xyzx)+(r3.xyzx)).xyz;
    // 25: mul r0.xyz, r0.xyzx, cb0[23].wwww
    r0.xyz = ((r0.xyzx)*(source[23].wwww)).xyz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 27: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 28: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 29: mad r4.xyz, cb0[7].xxxx, r4.xyzx, r3.xyzx
    r4.xyz = ((source[7].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 30: mul r5.xyz, cb0[4].xyzx, cb0[7].yyyy
    r5.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 31: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 32: mul r5.xyz, cb0[5].xyzx, cb0[7].zzzz
    r5.xyz = ((source[5].xyzx)*(source[7].zzzz)).xyz;
    // 33: mad r3.xyz, r5.xyzx, r3.xyzx, -r4.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(-(r4.xyzx))).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 35: mul r1.w, r5.z, cb0[7].w
    r1.w = ((r5.zzzz)*(source[7].wwww)).w;
    // 36: mul r5.xy, r5.yxyy, cb0[9].ywyy
    r5.xy = ((r5.yxyy)*(source[9].ywyy)).xy;
    // 37: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 38: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 39: mul r2.w, r2.w, cb0[8].x
    r2.w = ((r2.wwww)*(source[8].xxxx)).w;
    // 40: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 41: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 42: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: mul_sat r5.w, r1.w, cb2[3].w
    r5.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 44: mad r3.xyz, r2.wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 45: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 46: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 47: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 48: dp3 r1.w, r2.xyzx, r4.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 49: mul r6.xyz, r1.wwww, r2.xyzx
    r6.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 50: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 51: add r7.xyz, r6.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r7.xyz = ((r6.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 52: mad r7.xy, r7.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 53: min r3.w, r7.z, l(1.000000)
    r3.w = (min(r7.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 54: mad r7.xy, r7.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r7.xy = ((r7.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t1.xyzw, s1, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 56: mul r7.xyz, r7.xyzx, cb0[3].xyzx
    r7.xyz = ((r7.xyzx)*(source[3].xyzx)).xyz;
    // 57: mad r7.xyz, cb0[6].yyyy, r7.xyzx, r7.xyzx
    r7.xyz = ((source[6].yyyy)*(r7.xyzx)+(r7.xyzx)).xyz;
    // 58: add r7.xyz, r7.xyzx, -cb0[6].yyyy
    r7.xyz = ((r7.xyzx)+(-(source[6].yyyy))).xyz;
    // 59: mov_sat r8.xyz, r7.xyzx
    r8.xyz = (saturate(r7.xyzx)).xyz;
    // 60: mov_sat r7.xyz, -r7.xyzx
    r7.xyz = (saturate(-(r7.xyzx))).xyz;
    // 61: mad r7.xyz, -r0.wwww, r7.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r0.wwww))*(r7.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mad r3.xyz, r0.wwww, r8.xyzx, r3.xyzx
    r3.xyz = ((r0.wwww)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 63: mul r3.xyz, r7.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)).xyz;
    // 64: max r3.xyz, r3.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xyz = (max(r3.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 65: min r3.xyz, r3.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 66: mul r7.xyz, r3.xyzx, cb0[8].yyyy
    r7.xyz = ((r3.xyzx)*(source[8].yyyy)).xyz;
    // 67: mad r3.xyz, cb0[8].zzzz, r3.xyzx, -r7.xyzx
    r3.xyz = ((source[8].zzzz)*(r3.xyzx)+(-(r7.xyzx))).xyz;
    // 68: mad r3.xyz, r2.wwww, r3.xyzx, r7.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 69: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 70: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 71: mad_sat r7.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 72: mul r3.xyz, r0.xyzx, r7.xyzx
    r3.xyz = ((r0.xyzx)*(r7.xyzx)).xyz;
    // 73: dp2_sat r8.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 74: dp3_sat r8.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 75: dp3_sat r8.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 76: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 77: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 78: mul r9.xyz, r9.xyzx, cb0[25].xyzx
    r9.xyz = ((r9.xyzx)*(source[25].xyzx)).xyz;
    // 79: dp3 r0.w, r9.xyzx, r8.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 80: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 81: mul r8.xyz, r8.xyzx, cb0[24].xyzx
    r8.xyz = ((r8.xyzx)*(source[24].xyzx)).xyz;
    // 82: mul r10.xyz, r0.wwww, r8.xyzx
    r10.xyz = ((r0.wwww)*(r8.xyzx)).xyz;
    // 83: mad r3.xyz, r7.xyzx, r10.xyzx, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r10.xyzx)+(r3.xyzx)).xyz;
    // 84: mad r10.xyz, r7.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r7.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 85: mad r11.xyz, r7.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r7.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 86: mad r12.xyz, r7.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r7.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 87: log r13.xy, |r5.xyxx|
    r13.xy = (log2(abs(r5.xyxx))).xy;
    // 88: lt r5.xy, |r5.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r5.xy = (asfloat((uint4)((abs(r5.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 89: mul r2.w, r13.y, cb0[10].x
    r2.w = ((r13.yyyy)*(source[10].xxxx)).w;
    // 90: mul r4.w, r13.x, cb0[9].z
    r4.w = ((r13.xxxx)*(source[9].zzzz)).w;
    // 91: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 92: movc r4.w, r5.x, l(0), r4.w
    r4.w = ((asuint(r5.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 93: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 94: min r5.z, r4.w, l(1.000000)
    r5.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 96: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: movc r2.w, r5.y, l(0), r2.w
    r2.w = ((asuint(r5.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 98: mad r11.xyz, r2.wwww, r11.xyzx, r12.xyzx
    r11.xyz = ((r2.wwww)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 99: mad r10.xyz, r11.xyzx, r2.wwww, r10.xyzx
    r10.xyz = ((r11.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 100: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 101: max r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = (max(r2.wwww,r10.xyzx)).xyz;
    // 102: mul r3.xyz, r3.xyzx, r10.xyzx
    r3.xyz = ((r3.xyzx)*(r10.xyzx)).xyz;
    // 103: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 104: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 105: mul r10.xyz, r4.wwww, v1.xyzx
    r10.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 106: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 107: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 108: mul r11.xyz, r4.wwww, v0.xyzx
    r11.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 109: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 110: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 111: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 112: dp3 r13.y, r12.xyzx, r2.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 113: dp3 r5.y, r12.xyzx, r6.xyzx
    r5.y = (dot((r12.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 114: dp3 r13.x, r11.xyzx, r2.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 115: dp3 r12.y, r10.xyzx, r2.xyzx
    r12.y = (dot((r10.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 116: dp3 r2.y, r10.xyzx, r6.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 117: dp3 r5.x, r11.xyzx, r6.xyzx
    r5.x = (dot((r11.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 118: dp2 r12.z, r13.xyxx, cb0[12].xyxx
    r12.z = (dot((r13.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 119: mul r10.xy, cb0[12].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r10.xy = ((source[12].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 120: dp2 r12.x, r13.xyxx, r10.xyxx
    r12.x = (dot((r13.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 121: dp2 r2.x, r5.xyxx, r10.xyxx
    r2.x = (dot((r5.xyxx).xy,(r10.xyxx).xy).xxxx).x;
    // 122: dp2 r2.z, r5.xyxx, cb0[12].xyxx
    r2.z = (dot((r5.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 123: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 124: dp4 r10.x, cb0[13].xyzw, r12.xyzw
    r10.x = (dot((source[13].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 125: dp4 r10.y, cb0[14].xyzw, r12.xyzw
    r10.y = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 126: dp4 r10.z, cb0[15].xyzw, r12.xyzw
    r10.z = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 127: mul r11.xyzw, r12.yzzx, r12.xyzz
    r11.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 128: dp4 r14.x, cb0[16].xyzw, r11.xyzw
    r14.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 129: dp4 r14.y, cb0[17].xyzw, r11.xyzw
    r14.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 130: dp4 r14.z, cb0[18].xyzw, r11.xyzw
    r14.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 131: add r10.xyz, r10.xyzx, r14.xyzx
    r10.xyz = ((r10.xyzx)+(r14.xyzx)).xyz;
    // 132: mul r4.w, r12.y, r12.y
    r4.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 133: mov r13.z, r12.y
    r13.z = (r12.yyyy).z;
    // 134: mad r4.w, r12.x, r12.x, -r4.w
    r4.w = ((r12.xxxx)*(r12.xxxx)+(-(r4.wwww))).w;
    // 135: mad r10.xyz, cb0[19].xyzx, r4.wwww, r10.xyzx
    r10.xyz = ((source[19].xyzx)*(r4.wwww)+(r10.xyzx)).xyz;
    // 136: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 137: mul r10.xyz, r10.xyzx, cb0[11].xyzx
    r10.xyz = ((r10.xyzx)*(source[11].xyzx)).xyz;
    // 138: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[11].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[11].wwww)).xyz;
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
    // 147: mov_sat r7.w, cb0[8].w
    r7.w = (saturate(source[8].wwww)).w;
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
    // 185: mul r2.xyz, r2.xyzx, cb0[11].xyzx
    r2.xyz = ((r2.xyzx)*(source[11].xyzx)).xyz;
    // 186: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[11].wwww)).xyz;
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
    // 216: mad o0.xyz, r7.xyzx, cb0[23].xyzx, r0.yzwy
    output.targets[0].xyz = ((r7.xyzx)*(source[23].xyzx)+(r0.yzwy)).xyz;
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
    // 232: ftou r0.x, cb0[20].z
    r0.x = (asfloat((uint4)(source[20].zzzz))).x;
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

// source.character.static-map-native-1134.v1 / source program 53abcd7e2fc69c4983c2cdb712c6a368
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1134(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1134(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[11]=g_SourceCharacterEnvironmentColor;source[12]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 3: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 4: mad r1.xyz, cb0[7].xxxx, r1.xyzx, r0.xyzx
    r1.xyz = ((source[7].xxxx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r2.xyz, cb0[4].xyzx, cb0[7].yyyy
    r2.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 6: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 7: mul r2.xyz, cb0[5].xyzx, cb0[7].zzzz
    r2.xyz = ((source[5].xyzx)*(source[7].zzzz)).xyz;
    // 8: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 10: mul r0.w, r2.z, cb0[7].w
    r0.w = ((r2.zzzz)*(source[7].wwww)).w;
    // 11: mul r2.xy, r2.yxyy, cb0[9].ywyy
    r2.xy = ((r2.yxyy)*(source[9].ywyy)).xy;
    // 12: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 13: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.w, r1.w, cb0[8].x
    r1.w = ((r1.wwww)*(source[8].xxxx)).w;
    // 15: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 16: movc r0.w, r0.w, l(0), r1.w
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 17: min r1.w, r0.w, l(1.000000)
    r1.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: mul_sat r2.w, r0.w, cb2[3].w
    r2.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 19: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 21: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: mul r0.w, r1.z, cb0[6].z
    r0.w = ((r1.zzzz)*(source[6].zzzz)).w;
    // 23: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 24: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 25: mul r3.xy, r1.xyxx, v2.wwww
    r3.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 26: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 27: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 28: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 29: add r3.z, r1.x, l(0.000010)
    r3.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 30: dp3 r1.x, r3.xyzx, r3.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 31: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 32: div r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)/(r1.xxxx)).xyz;
    // 33: dp3 r3.x, r1.xyzx, r1.xyzx
    r3.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: rsq r3.x, r3.x
    r3.x = (rsqrt(r3.xxxx)).x;
    // 35: mul r3.xyz, r1.xyzx, r3.xxxx
    r3.xyz = ((r1.xyzx)*(r3.xxxx)).xyz;
    // 36: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 37: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 38: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 39: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 40: mul r5.xyz, r3.wwww, r3.xyzx
    r5.xyz = ((r3.wwww)*(r3.xyzx)).xyz;
    // 41: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 42: add r6.xyz, r5.xyzx, l(0.500000, 0.500000, 1.000000, 0.000000)
    r6.xyz = ((r5.xyzx)+(float4(0.500000,0.500000,1.000000,0.000000))).xyz;
    // 43: mad r6.xy, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), -v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(-(v4.xyxx))).xy;
    // 44: min r4.w, r6.z, l(1.000000)
    r4.w = (min(r6.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r6.xy, r6.xyxx, l(0.750000, 0.750000, 0.000000, 0.000000), v4.xyxx
    r6.xy = ((r6.xyxx)*(float4(0.750000,0.750000,0.000000,0.000000))+(v4.xyxx)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 47: mul r6.xyz, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 48: mad r6.xyz, cb0[6].yyyy, r6.xyzx, r6.xyzx
    r6.xyz = ((source[6].yyyy)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 49: add r6.xyz, r6.xyzx, -cb0[6].yyyy
    r6.xyz = ((r6.xyzx)+(-(source[6].yyyy))).xyz;
    // 50: mov_sat r7.xyz, r6.xyzx
    r7.xyz = (saturate(r6.xyzx)).xyz;
    // 51: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 52: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 53: mad r0.xyz, r0.wwww, r7.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r7.xyzx)+(r0.xyzx)).xyz;
    // 54: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 55: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 56: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 57: mul r6.xyz, r0.xyzx, cb0[8].yyyy
    r6.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 58: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 59: mad r0.xyz, r1.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 60: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 61: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 62: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 63: mov_sat r0.w, cb0[8].w
    r0.w = (saturate(source[8].wwww)).w;
    // 64: mad r6.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r6.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 65: mul r1.w, r0.w, l(0.080000)
    r1.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 66: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 67: mad r6.xyz, r2.wwww, r6.xyzx, r1.wwww
    r6.xyz = ((r2.wwww)*(r6.xyzx)+(r1.wwww)).xyz;
    // 68: log r7.xy, |r2.xyxx|
    r7.xy = (log2(abs(r2.xyxx))).xy;
    // 69: lt r2.xy, |r2.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((abs(r2.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 70: mul r0.w, r7.x, cb0[9].z
    r0.w = ((r7.xxxx)*(source[9].zzzz)).w;
    // 71: mul r1.w, r7.y, cb0[10].x
    r1.w = ((r7.yyyy)*(source[10].xxxx)).w;
    // 72: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 73: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 74: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 75: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 76: movc r0.w, r2.x, l(0), r0.w
    r0.w = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 77: max r0.w, r0.w, cb0[0].x
    r0.w = (max(r0.wwww,source[0].xxxx)).w;
    // 78: min r2.z, r0.w, l(1.000000)
    r2.z = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: deriv_rtx_coarse r2.x, r3.w
    r2.x = (ddx_coarse(r3.wwww)).x;
    // 80: deriv_rty_coarse r2.y, r3.w
    r2.y = (ddy_coarse(r3.wwww)).y;
    // 81: add r0.w, r3.w, l(1.000000)
    r0.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 82: add_sat r7.x, -r4.w, r0.w
    r7.x = (saturate((-(r4.wwww))+(r0.wwww))).x;
    // 83: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 84: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 85: mad_sat r7.y, r0.w, l(0.300000), r2.z
    r7.y = (saturate((r0.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.zzzz))).y;
    // 86: mov o2.zw, r2.zzzw
    output.targets[2].zw = (r2.zzzw).zw;
    // 87: add r0.w, -r7.y, l(1.000000)
    r0.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 88: max r2.xyz, r6.xyzx, r0.wwww
    r2.xyz = (max(r6.xyzx,r0.wwww)).xyz;
    // 89: add r2.xyz, -r6.xyzx, r2.xyzx
    r2.xyz = ((-(r6.xyzx))+(r2.xyzx)).xyz;
    // 90: mul_sat r0.w, r6.y, l(50.000000)
    r0.w = (saturate((r6.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 91: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 92: sample_indexable(texture2d)(float,float,float,float) r7.zw, r7.xyxx, t4.zwxy, s5
    r7.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 93: add r0.w, r1.w, r7.x
    r0.w = ((r1.wwww)+(r7.xxxx)).w;
    // 94: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 95: mul r8.xyz, r6.xyzx, r7.wwww
    r8.xyz = ((r6.xyzx)*(r7.wwww)).xyz;
    // 96: mad r2.xyz, r2.xyzx, r7.zzzz, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r7.zzzz)+(r8.xyzx)).xyz;
    // 97: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.w
    r3.w = r7.w != 0.f ? 1.f / r7.w : 0.f;
    // 98: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 99: mad r7.xzw, r6.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r7.xzw = ((r6.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 100: dp3 r3.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: mad r6.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r6.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 102: mad r8.xyz, -r2.xyzx, r7.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))*(r7.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 103: mul r2.xyz, r2.xyzx, r7.xzwx
    r2.xyz = ((r2.xyzx)*(r7.xzwx)).xyz;
    // 104: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 105: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 106: mul r7.xzw, r3.wwww, v1.xxyz
    r7.xzw = ((r3.wwww)*(v1.xxyz)).xzw;
    // 107: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 108: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 109: mul r9.xyz, r3.wwww, v0.xyzx
    r9.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 110: mul r10.xyz, r7.wxzw, r9.yzxy
    r10.xyz = ((r7.wxzw)*(r9.yzxy)).xyz;
    // 111: mad r10.xyz, r7.zwxz, r9.zxyz, -r10.xyzx
    r10.xyz = ((r7.zwxz)*(r9.zxyz)+(-(r10.xyzx))).xyz;
    // 112: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 113: dp3 r11.y, r10.xyzx, r3.xyzx
    r11.y = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 114: dp3 r10.y, r10.xyzx, r5.xyzx
    r10.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 115: dp3 r11.x, r9.xyzx, r3.xyzx
    r11.x = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 116: dp3 r10.x, r9.xyzx, r5.xyzx
    r10.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 117: dp2 r9.z, r11.xyxx, cb0[12].xyxx
    r9.z = (dot((r11.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 118: mul r10.zw, cb0[12].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r10.zw = ((source[12].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 119: dp2 r9.x, r11.xyxx, r10.zwzz
    r9.x = (dot((r11.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 120: dp2 r12.x, r10.xyxx, r10.zwzz
    r12.x = (dot((r10.xyxx).xy,(r10.zwzz).xy).xxxx).x;
    // 121: dp2 r12.z, r10.xyxx, cb0[12].xyxx
    r12.z = (dot((r10.xyxx).xy,(source[12].xyxx).xy).xxxx).z;
    // 122: dp3 r9.y, r7.xzwx, r3.xyzx
    r9.y = (dot((r7.xzwx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 123: dp3 r12.y, r7.xzwx, r5.xyzx
    r12.y = (dot((r7.xzwx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 124: mov r9.w, l(1.000000)
    r9.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 125: dp4 r10.x, cb0[13].xyzw, r9.xyzw
    r10.x = (dot((source[13].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).x;
    // 126: dp4 r10.y, cb0[14].xyzw, r9.xyzw
    r10.y = (dot((source[14].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).y;
    // 127: dp4 r10.z, cb0[15].xyzw, r9.xyzw
    r10.z = (dot((source[15].xyzw).xyzw,(r9.xyzw).xyzw).xxxx).z;
    // 128: mul r13.xyzw, r9.yzzx, r9.xyzz
    r13.xyzw = ((r9.yzzx)*(r9.xyzz)).xyzw;
    // 129: dp4 r14.x, cb0[16].xyzw, r13.xyzw
    r14.x = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 130: dp4 r14.y, cb0[17].xyzw, r13.xyzw
    r14.y = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 131: dp4 r14.z, cb0[18].xyzw, r13.xyzw
    r14.z = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 132: add r7.xzw, r10.xxyz, r14.xxyz
    r7.xzw = ((r10.xxyz)+(r14.xxyz)).xzw;
    // 133: mul r3.w, r9.y, r9.y
    r3.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 134: mov r11.z, r9.y
    r11.z = (r9.yyyy).z;
    // 135: mad r3.w, r9.x, r9.x, -r3.w
    r3.w = ((r9.xxxx)*(r9.xxxx)+(-(r3.wwww))).w;
    // 136: mad r7.xzw, cb0[19].xxyz, r3.wwww, r7.xxzw
    r7.xzw = ((source[19].xxyz)*(r3.wwww)+(r7.xxzw)).xzw;
    // 137: max r7.xzw, r7.xxzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xzw = (max(r7.xxzw,float4(0.000000,0.000000,0.000000,0.000000))).xzw;
    // 138: mul r7.xzw, r7.xxzw, cb0[11].xxyz
    r7.xzw = ((r7.xxzw)*(source[11].xxyz)).xzw;
    // 139: mad r7.xzw, r7.xxzw, l(3.141593, 0.000000, 3.141593, 3.141593), cb0[11].wwww
    r7.xzw = ((r7.xxzw)*(float4(3.141593,0.000000,3.141593,3.141593))+(source[11].wwww)).xzw;
    // 140: mul r7.xzw, r8.xxyz, r7.xxzw
    r7.xzw = ((r8.xxyz)*(r7.xxzw)).xzw;
    // 141: mad r8.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r8.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 142: mad r9.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r9.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 143: mad r10.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 144: mad r9.xyz, r1.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 145: mad r8.xyz, r9.xyzx, r1.wwww, r8.xyzx
    r8.xyz = ((r9.xyzx)*(r1.wwww)+(r8.xyzx)).xyz;
    // 146: mul r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 147: max r8.xyz, r1.wwww, r8.xyzx
    r8.xyz = (max(r1.wwww,r8.xyzx)).xyz;
    // 148: dp3 r3.w, v6.xyzx, v6.xyzx
    r3.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 149: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 150: mul r9.xyz, r3.wwww, v6.xyzx
    r9.xyz = ((r3.wwww)*(v6.xyzx)).xyz;
    // 151: dp3 r3.x, r9.xyzx, r3.xyzx
    r3.x = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 152: dp3 r3.y, r9.xyzx, r5.xyzx
    r3.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 153: mad r3.yz, r3.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r3.yz = ((r3.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 154: mad r3.xw, r3.xxxx, l(0.500000, 0.000000, 0.000000, -0.500000), l(0.500000, 0.000000, 0.000000, 0.500000)
    r3.xw = ((r3.xxxx)*(float4(0.500000,0.000000,0.000000,-0.500000))+(float4(0.500000,0.000000,0.000000,0.500000))).xw;
    // 155: mul r3.xyzw, r3.xyzw, r3.xyzw
    r3.xyzw = ((r3.xyzw)*(r3.xyzw)).xyzw;
    // 156: mul r5.xyz, r3.wwww, cb0[22].xyzx
    r5.xyz = ((r3.wwww)*(source[22].xyzx)).xyz;
    // 157: mad r5.xyz, r3.xxxx, cb0[21].xyzx, r5.xyzx
    r5.xyz = ((r3.xxxx)*(source[21].xyzx)+(r5.xyzx)).xyz;
    // 158: mul r5.xyz, r5.xyzx, cb0[23].wwww
    r5.xyz = ((r5.xyzx)*(source[23].wwww)).xyz;
    // 159: mul r5.xyz, r0.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 160: mul r5.xyz, r8.xyzx, r5.xyzx
    r5.xyz = ((r8.xyzx)*(r5.xyzx)).xyz;
    // 161: mul r5.xyz, r7.xzwx, r5.xyzx
    r5.xyz = ((r7.xzwx)*(r5.xyzx)).xyz;
    // 162: mad r5.xyz, -r5.xyzx, r2.wwww, r5.xyzx
    r5.xyz = ((-(r5.xyzx))*(r2.wwww)+(r5.xyzx)).xyz;
    // 163: mul r2.w, r7.y, r7.y
    r2.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 164: mul r3.x, r7.y, l(5.000000)
    r3.x = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 165: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r12.xyzx, t5.xyzw, s4, r3.x
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r12.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 166: mul r7.xyz, r7.xyzx, r7.wwww
    r7.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 167: mul r7.xyz, r7.xyzx, cb0[11].xyzx
    r7.xyz = ((r7.xyzx)*(source[11].xyzx)).xyz;
    // 168: mad r7.xyz, r7.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[11].wwww
    r7.xyz = ((r7.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[11].wwww)).xyz;
    // 169: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 170: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 171: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 172: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 173: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 174: mad r1.w, r0.w, r6.x, r6.y
    r1.w = ((r0.wwww)*(r6.xxxx)+(r6.yyyy)).w;
    // 175: mad r1.w, r1.w, r0.w, r6.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r6.zzzz)).w;
    // 176: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 177: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 178: mul r3.xzw, r3.zzzz, cb0[22].xxyz
    r3.xzw = ((r3.zzzz)*(source[22].xxyz)).xzw;
    // 179: mad r3.xyz, cb0[21].xyzx, r3.yyyy, r3.xzwx
    r3.xyz = ((source[21].xyzx)*(r3.yyyy)+(r3.xzwx)).xyz;
    // 180: mul r3.xyz, r3.xyzx, cb0[23].wwww
    r3.xyz = ((r3.xyzx)*(source[23].wwww)).xyz;
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
    // 199: mad o0.xyz, r0.xyzx, cb0[23].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[23].xyzx)+(r1.xyzx)).xyz;
    // 200: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 201: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 202: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 203: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
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
    // 213: ftou r0.x, cb0[20].z
    r0.x = (asfloat((uint4)(source[20].zzzz))).x;
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

// source.character.static-map-native-1135.v1 / source program f13096eab11ab545bee9deb94b587c98
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1135(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 4: mad r1.xyz, cb0[5].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[5].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r2.xyz, cb0[3].xyzx, cb0[5].wwww
    r2.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 6: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 7: mul r2.xyz, cb0[4].xyzx, cb0[6].xxxx
    r2.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // 8: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
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
    // 18: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
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

// source.character.static-map-native-1135.v1 / source program f8269fa49f1fa04e8e2be042565dcf27
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1135(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1135(input);
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
    // 4: mad r1.xyz, cb0[5].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[5].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 5: mul r2.xyz, cb0[3].xyzx, cb0[5].wwww
    r2.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 6: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 7: mul r2.xyz, cb0[4].xyzx, cb0[6].xxxx
    r2.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // 8: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
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
    // 18: mad r0.xyz, r1.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
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

// source.character.static-map-native-1136.v1 / source program dc5a4e17a9f45b4a8bd37757631079eb
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 1120u: return SourceCharacterBase1120(input);
    case 1121u: return SourceCharacterBase1121(input);
    case 1122u: return SourceCharacterBase1122(input);
    case 1123u: return SourceCharacterBase1123(input);
    case 1124u: return SourceCharacterBase1124(input);
    case 1125u: return SourceCharacterBase1125(input);
    case 1126u: return SourceCharacterBase1126(input);
    case 1127u: return SourceCharacterBase1127(input);
    case 1128u: return SourceCharacterBase1128(input);
    case 1129u: return SourceCharacterBase1129(input);
    case 1130u: return SourceCharacterBase1130(input);
    case 1131u: return SourceCharacterBase1131(input);
    case 1132u: return SourceCharacterBase1132(input);
    case 1133u: return SourceCharacterBase1133(input);
    case 1134u: return SourceCharacterBase1134(input);
    case 1135u: return SourceCharacterBase1135(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
