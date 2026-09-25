SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked237(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[7].x=(g_SourceCharacterTime.xxxx).x;
    source[13]=1.f; source[14]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = ((float4(0.0,0.0,0.0,0.0)).xyzw).x;
    // 4: min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 5: mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 6: mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // 7: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = (((r0.yyyy) != 0.f ? 1.f / (r0.yyyy) : 0.f)).y;
    // 8: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 9: add r0.x, r0.x, -v8.w
    r0.x = ((r0.xxxx)+(-(v8.wwww))).x;
    // 10: add r0.y, -cb0[8].z, l(1.000000)
    r0.y = ((-(source[8].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t1.yxzw, s5, l(0.000000)
    r0.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 14: mul_sat r0.y, r0.y, cb0[8].y
    r0.y = (saturate((r0.yyyy)*(source[8].yyyy))).y;
    // 15: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 16: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 17: mad r0.xy, -cb0[7].xxxx, cb0[2].zwzz, v4.xyxx
    r0.xy = ((-(source[7].xxxx))*(source[2].zwzz)+(v4.xyxx)).xy;
    // 18: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 19: mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mad r1.xy, cb0[7].xxxx, cb0[2].zwzz, v4.xyxx
    r1.xy = ((source[7].xxxx)*(source[2].zwzz)+(v4.xyxx)).xy;
    // 28: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 30: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 31: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 32: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 35: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 36: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 37: mul r1.xyzw, r0.xyxy, cb0[7].yyzz
    r1.xyzw = ((r0.xyxy)*(source[7].yyzz)).xyzw;
    // 38: mad r2.xy, cb0[7].xxxx, cb0[5].zwzz, v4.xyxx
    r2.xy = ((source[7].xxxx)*(source[5].zwzz)+(v4.xyxx)).xy;
    // 39: mad r2.xy, r2.xyxx, cb0[5].xyxx, r1.zwzz
    r2.xy = ((r2.xyxx)*(source[5].xyxx)+(r1.zwzz)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 42: mov r0.xy, r1.xyxx
    r0.xy = (r1.xyxx).xy;
    // 43: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 44: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 45: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 46: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 47: div r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)/(r1.xxxx)).xyz;
    // 48: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 51: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 52: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mad r0.x, -r0.x, l(0.500000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul_sat r0.x, r0.x, cb0[8].x
    r0.x = (saturate((r0.xxxx)*(source[8].xxxx))).x;
    // 55: dp3 r0.y, r3.xyzx, r4.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 56: mul r0.yzw, r0.yyyy, r3.xxyz
    r0.yzw = ((r0.yyyy)*(r3.xxyz)).yzw;
    // 57: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r4.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r4.xxyz))).yzw;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 59: mad r4.xyz, cb0[6].xyzx, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[6].xyzx)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 60: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 61: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 63: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 64: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 65: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 66: mul r4.xyz, r0.xxxx, v7.xyzx
    r4.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 67: dp3 r0.x, r4.xyzx, r3.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 68: mad r1.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 69: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 70: mul r4.xyz, r1.yyyy, cb0[11].xyzx
    r4.xyz = ((r1.yyyy)*(source[11].xyzx)).xyz;
    // 71: mad r4.xyz, r1.xxxx, cb0[10].xyzx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(source[10].xyzx)+(r4.xyzx)).xyz;
    // 72: mul r4.xyz, r4.xyzx, cb0[12].wwww
    r4.xyz = ((r4.xyzx)*(source[12].wwww)).xyz;
    // 73: mul r5.xyz, r2.xyzx, r4.xyzx
    r5.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 74: dp2_sat r6.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 75: dp3_sat r6.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 76: dp3_sat r6.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 77: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 78: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t7.xyzw, s6
    r7.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 79: mul r7.xyz, r7.xyzx, cb0[14].xyzx
    r7.xyz = ((r7.xyzx)*(source[14].xyzx)).xyz;
    // 80: dp3 r0.x, r7.xyzx, r6.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 81: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t6.xyzw, s6
    r6.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 82: mul r6.xyz, r6.xyzx, cb0[13].xyzx
    r6.xyz = ((r6.xyzx)*(source[13].xyzx)).xyz;
    // 83: mul r8.xyz, r0.xxxx, r6.xyzx
    r8.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 84: mad r4.xyz, r6.xyzx, r0.xxxx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 85: mul r6.xyz, r6.xyzx, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].xyzx)).xyz;
    // 86: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 87: div r4.xyz, r8.xyzx, r4.xyzx
    r4.xyz = ((r8.xyzx)/(r4.xyzx)).xyz;
    // 88: mad r5.xyz, r2.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((r2.xyzx)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 89: dp3 r0.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 90: dp2_sat r4.x, r0.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r0.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 91: dp3_sat r4.y, r0.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r0.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 92: dp3_sat r4.z, r0.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r0.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 93: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s2, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 94: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 95: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 96: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 97: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 98: dp3 r1.x, r7.xyzx, r4.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 99: mad r4.xyz, r6.xyzx, r1.xxxx, r5.xyzx
    r4.xyz = ((r6.xyzx)*(r1.xxxx)+(r5.xyzx)).xyz;
    // 100: mul r5.xyz, r1.xxxx, r6.xyzx
    r5.xyz = ((r1.xxxx)*(r6.xyzx)).xyz;
    // 101: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 102: mad r1.xy, cb0[7].xxxx, cb0[3].zwzz, v4.xyxx
    r1.xy = ((source[7].xxxx)*(source[3].zwzz)+(v4.xyxx)).xy;
    // 103: mad r1.xy, r1.xyxx, cb0[3].xyxx, r1.zwzz
    r1.xy = ((r1.xyxx)*(source[3].xyxx)+(r1.zwzz)).xy;
    // 104: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 105: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 106: mad r0.yzw, cb0[7].wwww, r0.yyzw, cb0[1].xxyz
    r0.yzw = ((source[7].wwww)*(r0.yyzw)+(source[1].xxyz)).yzw;
    // 107: add r0.yzw, r4.xxyz, r0.yyzw
    r0.yzw = ((r4.xxyz)+(r0.yyzw)).yzw;
    // 108: mad r0.yzw, r2.xxyz, cb0[12].xxyz, r0.yyzw
    r0.yzw = ((r2.xxyz)*(source[12].xxyz)+(r0.yyzw)).yzw;
    // 109: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 110: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 111: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 112: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 113: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 114: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 115: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 116: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 117: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 118: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 119: dp3 r5.z, r0.yzwy, r3.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 120: dp3 r5.x, r1.xyzx, r3.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 121: mul r0.yzw, r2.xxyz, v1.wwww
    r0.yzw = ((r2.xxyz)*(v1.wwww)).yzw;
    // 122: dp3 r5.y, r0.yzwy, r3.xyzx
    r5.y = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).y;
    // 123: dp3 r0.y, r5.xyzx, r5.xyzx
    r0.y = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 124: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 125: mul r0.yzw, r0.yyyy, r5.xxyz
    r0.yzw = ((r0.yyyy)*(r5.xxyz)).yzw;
    // 126: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 127: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 128: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 129: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 130: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 131: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 132: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 133: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 134: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 135: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 136: mul o4.z, r0.x, r4.x
    output.targets[4].z = ((r0.xxxx)*(r4.xxxx)).z;
    // 137: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 138: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 139: ftou r0.x, cb0[9].z
    r0.x = (asfloat((uint4)(source[9].zzzz))).x;
    // 140: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 141: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 142: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 143: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 144: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 145: ret
    return output;
}

// source.character.static-map-native-237.v1 / source program b4417bc2bf016c4488df53083375ac18
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase237(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7].x=g_SourceCharacterBaseConstants[7].x;
    source[7].y=g_SourceCharacterBaseConstants[7].y;
    source[7].z=g_SourceCharacterBaseConstants[7].z;
    source[7].w=g_SourceCharacterBaseConstants[7].w;
    source[8].x=g_SourceCharacterBaseConstants[8].x;
    source[8].y=g_SourceCharacterBaseConstants[8].y;
    source[8].z=g_SourceCharacterBaseConstants[8].z;
    source[8].w=g_SourceCharacterBaseConstants[8].w;
    source[7].x=(g_SourceCharacterTime.xxxx).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 3: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s0, l(0.000000)
    r0.x = ((float4(0.0,0.0,0.0,0.0)).xyzw).x;
    // 4: min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 5: mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 6: mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // 7: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = (((r0.yyyy) != 0.f ? 1.f / (r0.yyyy) : 0.f)).y;
    // 8: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 9: add r0.x, r0.x, -v8.w
    r0.x = ((r0.xxxx)+(-(v8.wwww))).x;
    // 10: add r0.y, -cb0[8].z, l(1.000000)
    r0.y = ((-(source[8].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t1.yxzw, s5, l(0.000000)
    r0.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 14: mul_sat r0.y, r0.y, cb0[8].y
    r0.y = (saturate((r0.yyyy)*(source[8].yyyy))).y;
    // 15: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 16: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 17: mad r0.xy, -cb0[7].xxxx, cb0[2].zwzz, v4.xyxx
    r0.xy = ((-(source[7].xxxx))*(source[2].zwzz)+(v4.xyxx)).xy;
    // 18: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 19: mul r0.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 21: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 22: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 23: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 24: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 25: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 26: add r0.z, r0.w, l(0.000010)
    r0.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 27: mad r1.xy, cb0[7].xxxx, cb0[2].zwzz, v4.xyxx
    r1.xy = ((source[7].xxxx)*(source[2].zwzz)+(v4.xyxx)).xy;
    // 28: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 30: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 31: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 32: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 35: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 36: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 37: mul r1.xyzw, r0.xyxy, cb0[7].yyzz
    r1.xyzw = ((r0.xyxy)*(source[7].yyzz)).xyzw;
    // 38: mad r2.xy, cb0[7].xxxx, cb0[5].zwzz, v4.xyxx
    r2.xy = ((source[7].xxxx)*(source[5].zwzz)+(v4.xyxx)).xy;
    // 39: mad r2.xy, r2.xyxx, cb0[5].xyxx, r1.zwzz
    r2.xy = ((r2.xyxx)*(source[5].xyxx)+(r1.zwzz)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: mul r2.xyz, r2.xyzx, cb0[4].xyzx
    r2.xyz = ((r2.xyzx)*(source[4].xyzx)).xyz;
    // 42: mov r0.xy, r1.xyxx
    r0.xy = (r1.xyxx).xy;
    // 43: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 44: sqrt r1.x, r0.w
    r1.x = (sqrt(r0.wwww)).x;
    // 45: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 46: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 47: div r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)/(r1.xxxx)).xyz;
    // 48: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 51: dp3 r0.x, r0.xyzx, r4.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 52: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mad r0.x, -r0.x, l(0.500000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 54: mul_sat r0.x, r0.x, cb0[8].x
    r0.x = (saturate((r0.xxxx)*(source[8].xxxx))).x;
    // 55: dp3 r0.y, r3.xyzx, r4.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 56: mul r0.yz, r0.yyyy, r3.xxyx
    r0.yz = ((r0.yyyy)*(r3.xxyx)).yz;
    // 57: mad r0.yz, r0.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), -r4.xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(-(r4.xxyx))).yz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 59: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t5.wxyz, s2, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 60: mad r4.xyz, cb0[6].xyzx, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[6].xyzx)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 61: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 62: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 63: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 64: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 65: mad r1.xy, cb0[7].xxxx, cb0[3].zwzz, v4.xyxx
    r1.xy = ((source[7].xxxx)*(source[3].zwzz)+(v4.xyxx)).xy;
    // 66: mad r1.xy, r1.xyxx, cb0[3].xyxx, r1.zwzz
    r1.xy = ((r1.xyxx)*(source[3].xyxx)+(r1.zwzz)).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r0.xyz, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.yzwy)*(r1.xyzx)).xyz;
    // 69: mad r0.xyz, cb0[7].wwww, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((source[7].wwww)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 70: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 71: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 72: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 73: dp3 r0.w, r1.xyzx, r3.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 74: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 76: mul r1.yzw, r1.yyyy, cb0[11].xxyz
    r1.yzw = ((r1.yyyy)*(source[11].xxyz)).yzw;
    // 77: mad r1.xyz, r1.xxxx, cb0[10].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[10].xyzx)+(r1.yzwy)).xyz;
    // 78: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 79: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 80: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 81: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad r0.xyz, r2.xyzx, cb0[12].xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 83: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 84: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 86: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 87: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 88: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 91: mul r2.xyz, r0.zxyz, r1.yzxy
    r2.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 92: mad r2.xyz, r0.yzxy, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 93: dp3 r0.z, r0.xyzx, r3.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 94: dp3 r0.x, r1.xyzx, r3.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 95: mul r1.xyz, r2.xyzx, v1.wwww
    r1.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 96: dp3 r0.y, r1.xyzx, r3.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 97: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 98: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 99: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 100: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 101: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 102: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 103: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 104: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 105: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 106: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 107: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 108: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 109: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 110: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 111: ftou r0.x, cb0[9].z
    r0.x = (asfloat((uint4)(source[9].zzzz))).x;
    // 112: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 113: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 114: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 115: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 116: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 117: ret
    return output;
}

// source.character.kouku-bazooka-dead.v1 / source program 231a7f149fd8054589dc4baf0825beb0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase238(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[17]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[18]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[23].z=(g_SourceCharacterTime.xxxx).x;
    source[24].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[25].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[25].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[25].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[25].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[26].xxxx
    r0.xy = ((v4.xyxx)*(source[26].xxxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s7, l(0.000000)
    r0.x = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 3: add r0.x, r0.x, -cb0[26].y
    r0.x = ((r0.xxxx)+(-(source[26].yyyy))).x;
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
    // 11: add r0.x, cb0[2].y, cb0[2].x
    r0.x = ((source[2].yyyy)+(source[2].xxxx)).x;
    // 12: add r0.x, r0.x, cb0[2].z
    r0.x = ((r0.xxxx)+(source[2].zzzz)).x;
    // 13: add r0.y, -r0.x, l(1000.000000)
    r0.y = ((-(r0.xxxx))+(float4(1000.000000,1000.000000,1000.000000,1000.000000))).y;
    // 14: mad r0.x, cb0[23].w, r0.y, r0.x
    r0.x = ((source[23].wwww)*(r0.yyyy)+(r0.xxxx)).x;
    // 15: mul r0.x, r0.x, l(0.010000)
    r0.x = ((r0.xxxx)*(float4(0.010000,0.010000,0.010000,0.010000))).x;
    // 16: mad r0.x, cb0[23].y, cb0[23].z, r0.x
    r0.x = ((source[23].yyyy)*(source[23].zzzz)+(r0.xxxx)).x;
    // 17: mul r0.y, r0.x, l(3.524534)
    r0.y = ((r0.xxxx)*(float4(3.524534,3.524534,3.524534,3.524534))).y;
    // 18: sincos null, r0.y, r0.y
    r0.y = (cos(r0.yyyy)).y;
    // 19: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 20: mul r0.x, r0.x, l(1.328987)
    r0.x = ((r0.xxxx)*(float4(1.328987,1.328987,1.328987,1.328987))).x;
    // 21: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 22: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mad r0.x, r0.x, l(0.500000), cb0[23].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[23].xxxx)).x;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, v4.xyxx, t7.wxyz, s5, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 25: mul r2.xyz, cb0[10].xyzx, cb0[22].wwww
    r2.xyz = ((source[10].xyzx)*(source[22].wwww)).xyz;
    // 26: mul r0.yzw, r0.yyzw, r2.xxyz
    r0.yzw = ((r0.yyzw)*(r2.xxyz)).yzw;
    // 27: mul r0.xyz, r0.yzwy, r0.xxxx
    r0.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 28: mul r2.xyz, v7.yyyy, cb1[1].xywx
    r2.xyz = ((v7.yyyy)*(projection[1].xywx)).xyz;
    // 29: mad r2.xyz, cb1[0].xywx, v7.xxxx, r2.xyzx
    r2.xyz = ((projection[0].xywx)*(v7.xxxx)+(r2.xyzx)).xyz;
    // 30: mad r2.xyz, cb1[2].xywx, v7.zzzz, r2.xyzx
    r2.xyz = ((projection[2].xywx)*(v7.zzzz)+(r2.xyzx)).xyz;
    // 31: mad r2.xyz, cb1[3].xywx, v7.wwww, r2.xyzx
    r2.xyz = ((projection[3].xywx)*(v7.wwww)+(r2.xyzx)).xyz;
    // 32: div r2.xy, r2.xyxx, r2.zzzz
    r2.xy = ((r2.xyxx)/(r2.zzzz)).xy;
    // 33: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 34: mul r2.xy, r2.xyxx, l(700.000000, 700.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(700.000000,700.000000,0.000000,0.000000))).xy;
    // 35: deriv_rtx_coarse r2.zw, r2.xxxy
    r2.zw = (ddx_coarse(r2.xxxy)).zw;
    // 36: deriv_rty_coarse r2.xy, r2.xyxx
    r2.xy = (ddy_coarse(r2.xyxx)).xy;
    // 37: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 38: dp2 r1.w, r2.zwzz, r2.zwzz
    r1.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 39: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 40: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 41: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 42: rcp r1.w, |r0.w|
    r1.w = (1.0/(abs(r0.wwww))).w;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: log r3.x, |r2.w|
    r3.x = (log2(abs(r2.wwww))).x;
    // 46: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 47: mul r3.x, r3.x, cb0[20].x
    r3.x = ((r3.xxxx)*(source[20].xxxx)).x;
    // 48: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 49: min r3.x, r3.x, l(1.000000)
    r3.x = (min(r3.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: movc r2.w, r2.w, l(0), r3.x
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).w;
    // 51: sqrt r3.x, r2.w
    r3.x = (sqrt(r2.wwww)).x;
    // 52: mul r3.x, r3.x, cb0[20].y
    r3.x = ((r3.xxxx)*(source[20].yyyy)).x;
    // 53: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 54: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 55: add r0.w, |r0.w|, r1.w
    r0.w = ((abs(r0.wwww))+(r1.wwww)).w;
    // 56: round_ni r0.w, r0.w
    r0.w = (floor(r0.wwww)).w;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 58: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 59: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 60: mul r3.xy, r3.xyxx, cb0[19].xxxx
    r3.xy = ((r3.xyxx)*(source[19].xxxx)).xy;
    // 61: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 62: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 63: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 64: add r3.z, r1.w, l(0.000010)
    r3.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 65: add r4.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 66: mad r4.xyz, cb0[19].wwww, r4.xyzx, r3.xyzx
    r4.xyz = ((source[19].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 67: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 68: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 69: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 70: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 73: dp3 r6.x, r5.xyzx, r4.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 74: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 75: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 76: mul r7.xyz, r1.wwww, v1.xyzx
    r7.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 77: mul r8.xyz, r5.yzxy, r7.zxyz
    r8.xyz = ((r5.yzxy)*(r7.zxyz)).xyz;
    // 78: mad r8.xyz, r7.yzxy, r5.zxyz, -r8.xyzx
    r8.xyz = ((r7.yzxy)*(r5.zxyz)+(-(r8.xyzx))).xyz;
    // 79: mul r8.xyz, r8.xyzx, v1.wwww
    r8.xyz = ((r8.xyzx)*(v1.wwww)).xyz;
    // 80: dp3 r6.y, r8.xyzx, r4.xyzx
    r6.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 81: dp3 r6.z, r7.xyzx, r4.xyzx
    r6.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 82: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 83: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 84: mul r4.xyz, r1.wwww, v5.xyzx
    r4.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 85: mad r9.xyz, v5.xyzx, r1.wwww, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((v5.xyzx)*(r1.wwww)+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 86: dp3 r10.y, r8.xyzx, r4.xyzx
    r10.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 87: dp3 r10.x, r5.xyzx, r4.xyzx
    r10.x = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 88: dp3 r10.z, r7.xyzx, r4.xyzx
    r10.z = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 89: dp3 r1.w, r6.xyzx, r10.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 90: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 91: mad r6.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r10.xyzx
    r6.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r10.xyzx))).xyz;
    // 92: mov r6.w, -r6.x
    r6.w = (-(r6.xxxx)).w;
    // 93: dp2 r1.w, r6.ywyy, r6.ywyy
    r1.w = (dot((r6.ywyy).xy,(r6.ywyy).xy).xxxx).w;
    // 94: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 95: div r6.xy, r6.ywyy, r1.wwww
    r6.xy = ((r6.ywyy)/(r1.wwww)).xy;
    // 96: mad r1.w, -r6.z, l(0.250000), l(0.250000)
    r1.w = ((-(r6.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).w;
    // 97: add r3.w, r6.z, l(1.000000)
    r3.w = ((r6.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 98: mul r3.w, r3.w, l(0.500000)
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 99: mad r6.xy, r1.wwww, r6.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(r6.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 100: sample_l_indexable(texture2d)(float,float,float,float) r6.xyz, r6.xyxx, t4.xyzw, s3, r0.w
    r6.xyz = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r6.xyxx).xy, (r0.wwww).x)).xyzw).xyz;
    // 101: log r10.xyz, r6.xyzx
    r10.xyz = (log2(r6.xyzx)).xyz;
    // 102: rcp r0.w, cb0[20].z
    r0.w = (1.0/(source[20].zzzz)).w;
    // 103: mul r11.xyz, r10.xyzx, r0.wwww
    r11.xyz = ((r10.xyzx)*(r0.wwww)).xyz;
    // 104: mul r10.xyz, r10.xyzx, cb0[20].zzzz
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)).xyz;
    // 105: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 106: exp r11.xyz, r11.xyzx
    r11.xyz = (exp2(r11.xyzx)).xyz;
    // 107: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 108: mad r10.xyz, r10.xyzx, cb0[20].zzzz, r11.xyzx
    r10.xyz = ((r10.xyzx)*(source[20].zzzz)+(r11.xyzx)).xyz;
    // 109: add r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)+(r10.xyzx)).xyz;
    // 110: mul r6.xyz, r6.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.333333,0.333333,0.333333,0.000000))).xyz;
    // 111: add r0.w, cb0[20].z, l(1.000000)
    r0.w = ((source[20].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 112: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 113: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r6.xyz, -cb0[8].xyzx, cb0[9].xyzx
    r6.xyz = ((-(source[8].xyzx))+(source[9].xyzx)).xyz;
    // 115: mad r6.xyz, r3.wwww, r6.xyzx, cb0[8].xyzx
    r6.xyz = ((r3.wwww)*(r6.xyzx)+(source[8].xyzx)).xyz;
    // 116: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 117: mul r6.xyz, r6.xyzx, cb0[20].wwww
    r6.xyz = ((r6.xyzx)*(source[20].wwww)).xyz;
    // 118: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 120: mad r2.xyz, cb0[19].yyyy, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].yyyy)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 121: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 122: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 123: mad r2.xyz, cb0[19].zzzz, r10.xyzx, r2.xyzx
    r2.xyz = ((source[19].zzzz)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 124: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 125: add r10.xyz, -r2.xyzx, r0.wwww
    r10.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 126: mul r10.xyz, r10.xyzx, cb0[21].xxxx
    r10.xyz = ((r10.xyzx)*(source[21].xxxx)).xyz;
    // 127: sample_b_indexable(texture2d)(float,float,float,float) r11.xyzw, v4.xyxx, t5.xyzw, s4, l(0.000000)
    r11.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 128: add r0.w, r11.y, r11.x
    r0.w = ((r11.yyyy)+(r11.xxxx)).w;
    // 129: add r0.w, r11.z, r0.w
    r0.w = ((r11.zzzz)+(r0.wwww)).w;
    // 130: add_sat r0.w, r11.w, r0.w
    r0.w = (saturate((r11.wwww)+(r0.wwww))).w;
    // 131: mad r2.xyz, r0.wwww, r10.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r10.xyzx)+(r2.xyzx)).xyz;
    // 132: max r10.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r10.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 133: log r10.xyz, r10.xyzx
    r10.xyz = (log2(r10.xyzx)).xyz;
    // 134: mul r10.xyz, r10.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r10.xyz = ((r10.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 135: exp r10.xyz, r10.xyzx
    r10.xyz = (exp2(r10.xyzx)).xyz;
    // 136: dp3 r0.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 137: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 138: mul r0.w, r0.w, cb0[22].x
    r0.w = ((r0.wwww)*(source[22].xxxx)).w;
    // 139: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 140: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: mad r1.w, -r0.w, r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 143: div r1.w, cb0[22].y, r1.w
    r1.w = ((source[22].yyyy)/(r1.wwww)).w;
    // 144: dp3 r3.w, r3.xyzx, r3.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 145: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 146: div r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)/(r3.wwww)).xyz;
    // 147: dp3 r3.w, r3.xyzx, r4.xyzx
    r3.w = (dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 148: mul_sat r4.w, r3.w, cb0[21].y
    r4.w = (saturate((r3.wwww)*(source[21].yyyy))).w;
    // 149: add r3.w, -|r3.w|, l(1.000000)
    r3.w = ((-(abs(r3.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 150: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: mul_sat r5.w, r4.z, cb0[21].y
    r5.w = (saturate((r4.zzzz)*(source[21].yyyy))).w;
    // 152: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add_sat r5.w, r5.w, -cb0[21].z
    r5.w = (saturate((r5.wwww)+(-(source[21].zzzz)))).w;
    // 154: log r6.w, r5.w
    r6.w = (log2(r5.wwww)).w;
    // 155: lt r5.w, r5.w, l(0.000001)
    r5.w = (asfloat((uint4)((r5.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 156: mul r6.w, r6.w, cb0[21].w
    r6.w = ((r6.wwww)*(source[21].wwww)).w;
    // 157: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 158: mul r4.w, r4.w, r6.w
    r4.w = ((r4.wwww)*(r6.wwww)).w;
    // 159: movc r4.w, r5.w, l(0), r4.w
    r4.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 160: mul r1.w, r1.w, r4.w
    r1.w = ((r1.wwww)*(r4.wwww)).w;
    // 161: mul r10.xyz, r6.xyzx, r1.wwww
    r10.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 162: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 164: mad r1.xyz, cb0[19].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 165: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 166: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 167: mad r1.xyz, cb0[19].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 168: mul r11.xyz, cb0[5].xyzx, cb0[5].wwww
    r11.xyz = ((source[5].xyzx)*(source[5].wwww)).xyz;
    // 169: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 170: mad r12.xyz, -cb0[5].wwww, cb0[5].xyzx, r1.wwww
    r12.xyz = ((-(source[5].wwww))*(source[5].xyzx)+(r1.wwww)).xyz;
    // 171: mad r11.xyz, cb0[19].yyyy, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].yyyy)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 172: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: add r12.xyz, -r11.xyzx, r1.wwww
    r12.xyz = ((-(r11.xyzx))+(r1.wwww)).xyz;
    // 174: mad r11.xyz, cb0[19].zzzz, r12.xyzx, r11.xyzx
    r11.xyz = ((source[19].zzzz)*(r12.xyzx)+(r11.xyzx)).xyz;
    // 175: mad r12.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 176: mad r13.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 178: mul r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)*(r12.xyzx)).xyz;
    // 179: mul r13.xyz, r1.xyzx, r11.xyzx
    r13.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 180: mad r1.xyz, r11.xyzx, r1.xyzx, l(0.010000, 0.010000, 0.010000, 0.000000)
    r1.xyz = ((r11.xyzx)*(r1.xyzx)+(float4(0.010000,0.010000,0.010000,0.000000))).xyz;
    // 181: mul r6.xyz, r6.xyzx, r13.xyzx
    r6.xyz = ((r6.xyzx)*(r13.xyzx)).xyz;
    // 182: mad r2.xyz, r2.xyzx, r10.xyzx, -r6.xyzx
    r2.xyz = ((r2.xyzx)*(r10.xyzx)+(-(r6.xyzx))).xyz;
    // 183: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 184: mul r1.w, r1.w, cb0[22].z
    r1.w = ((r1.wwww)*(source[22].zzzz)).w;
    // 185: mad r2.xyz, r1.wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 186: frc r1.w, cb0[4].x
    r1.w = (frac(source[4].xxxx)).w;
    // 187: add r5.w, -r1.w, l(1.000000)
    r5.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 188: mad r0.xyz, r5.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r5.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 189: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 190: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 191: mad r0.xyz, cb0[19].yyyy, r6.xyzx, r0.xyzx
    r0.xyz = ((source[19].yyyy)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 192: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 193: add r6.xyz, -r0.xyzx, r5.wwww
    r6.xyz = ((-(r0.xyzx))+(r5.wwww)).xyz;
    // 194: mad r0.xyz, cb0[19].zzzz, r6.xyzx, r0.xyzx
    r0.xyz = ((source[19].zzzz)*(r6.xyzx)+(r0.xyzx)).xyz;
    // 195: dp3 r5.w, r1.xyzx, r1.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 196: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 197: div r1.xyz, r1.xyzx, r5.wwww
    r1.xyz = ((r1.xyzx)/(r5.wwww)).xyz;
    // 198: dp3 r5.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 199: add r6.xyz, -r1.xyzx, r5.wwww
    r6.xyz = ((-(r1.xyzx))+(r5.wwww)).xyz;
    // 200: add r1.xyz, r1.xyzx, -r6.xyzx
    r1.xyz = ((r1.xyzx)+(-(r6.xyzx))).xyz;
    // 201: mul r6.xyz, cb0[13].xyzx, cb0[24].yyyy
    r6.xyz = ((source[13].xyzx)*(source[24].yyyy)).xyz;
    // 202: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 203: mul r6.xyz, r4.wwww, r6.xyzx
    r6.xyz = ((r4.wwww)*(r6.xyzx)).xyz;
    // 204: mad r10.xyz, r4.wwww, cb0[12].xyzx, -cb0[12].xyzx
    r10.xyz = ((r4.wwww)*(source[12].xyzx)+(-(source[12].xyzx))).xyz;
    // 205: add r4.w, r4.w, l(-1.000000)
    r4.w = ((r4.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 206: mad r4.w, cb0[11].w, r4.w, l(1.000000)
    r4.w = ((source[11].wwww)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 207: mad r10.xyz, cb0[12].wwww, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((source[12].wwww)*(r10.xyzx)+(source[12].xyzx)).xyz;
    // 208: mad r1.xyz, r1.xyzx, r6.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r10.xyzx)).xyz;
    // 209: mad r1.xyz, r4.wwww, cb0[11].xyzx, r1.xyzx
    r1.xyz = ((r4.wwww)*(source[11].xyzx)+(r1.xyzx)).xyz;
    // 210: mad r0.xyz, r0.xyzx, r12.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r12.xyzx)+(r1.xyzx)).xyz;
    // 211: add r1.x, -|r4.z|, l(1.000000)
    r1.x = ((-(abs(r4.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 212: mul r1.x, r3.w, r1.x
    r1.x = ((r3.wwww)*(r1.xxxx)).x;
    // 213: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 214: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 215: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 216: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 217: mul r6.xyz, r1.yyyy, cb0[14].xyzx
    r6.xyz = ((r1.yyyy)*(source[14].xyzx)).xyz;
    // 218: movc r1.xyz, r1.xxxx, l(0,0,0,0), r6.xyzx
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xyzx)).xyz;
    // 219: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 220: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 221: dp3 r1.x, r9.xyzx, r9.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 222: sqrt r1.y, r1.x
    r1.y = (sqrt(r1.xxxx)).y;
    // 223: div r6.xyz, r9.xyzx, r1.yyyy
    r6.xyz = ((r9.xyzx)/(r1.yyyy)).xyz;
    // 224: dp3 r1.y, r6.xyzx, r4.xyzx
    r1.y = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 225: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 226: mul r1.z, |r1.y|, |r1.y|
    r1.z = ((abs(r1.yyyy))*(abs(r1.yyyy))).z;
    // 227: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 228: mul r1.z, r1.z, |r1.y|
    r1.z = ((r1.zzzz)*(abs(r1.yyyy))).z;
    // 229: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 230: movc r1.y, r1.y, l(0), r1.z
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 231: add r1.z, r1.y, l(-0.027778)
    r1.z = ((r1.yyyy)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).z;
    // 232: mad r1.y, r1.y, r1.z, l(0.027778)
    r1.y = ((r1.yyyy)*(r1.zzzz)+(float4(0.027778,0.027778,0.027778,0.027778))).y;
    // 233: div_sat r1.x, r1.y, r1.x
    r1.x = (saturate((r1.yyyy)/(r1.xxxx))).x;
    // 234: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: mul r1.x, r1.x, r2.w
    r1.x = ((r1.xxxx)*(r2.wwww)).x;
    // 236: mad r1.xyz, r1.xxxx, r2.xyzx, -r13.xyzx
    r1.xyz = ((r1.xxxx)*(r2.xyzx)+(-(r13.xyzx))).xyz;
    // 237: mad r1.xyz, r0.wwww, r1.xyzx, r13.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r13.xyzx)).xyz;
    // 238: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 239: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 240: mad r1.xyz, cb0[19].yyyy, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].yyyy)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 241: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 242: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 243: mad r1.xyz, cb0[19].zzzz, r2.xyzx, r1.xyzx
    r1.xyz = ((source[19].zzzz)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 244: mul r1.xyz, r12.xyzx, r1.xyzx
    r1.xyz = ((r12.xyzx)*(r1.xyzx)).xyz;
    // 245: add r0.w, -cb0[4].w, l(1.000000)
    r0.w = ((-(source[4].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 246: mul r0.w, r0.w, cb0[23].z
    r0.w = ((r0.wwww)*(source[23].zzzz)).w;
    // 247: mul r0.w, r0.w, l(6.283185)
    r0.w = ((r0.wwww)*(float4(6.283185,6.283185,6.283185,6.283185))).w;
    // 248: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 249: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 250: mul r2.x, cb0[4].z, l(1.500000)
    r2.x = ((source[4].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 251: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 252: mad r0.w, r0.w, l(0.500000), cb0[4].z
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[4].zzzz)).w;
    // 253: add r2.x, -r1.w, cb0[4].x
    r2.x = ((-(r1.wwww))+(source[4].xxxx)).x;
    // 254: mul r2.z, r2.x, l(0.125000)
    r2.z = ((r2.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 255: frc r3.w, v4.x
    r3.w = (frac(v4.xxxx)).w;
    // 256: mul r4.x, r3.w, l(0.125000)
    r4.x = ((r3.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 257: mul r2.y, cb0[4].y, cb0[15].y
    r2.y = ((source[4].yyyy)*(source[15].yyyy)).y;
    // 258: mov r4.y, v4.y
    r4.y = (v4.yyyy).y;
    // 259: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 260: add r2.xy, r2.xyxx, r4.xyxx
    r2.xy = ((r2.xyxx)+(r4.xyxx)).xy;
    // 261: add r2.xy, r2.xyxx, r2.zwzz
    r2.xy = ((r2.xyxx)+(r2.zwzz)).xy;
    // 262: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 263: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 264: mul r0.w, r1.w, r2.w
    r0.w = ((r1.wwww)*(r2.wwww)).w;
    // 265: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 266: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 267: add r2.xyzw, v7.yzxy, cb0[0].yzxy
    r2.xyzw = ((v7.yzxy)+(source[0].yzxy)).xyzw;
    // 268: add r2.xyzw, r2.xyzw, -cb0[1].yzxy
    r2.xyzw = ((r2.xyzw)+(-(source[1].yzxy))).xyzw;
    // 269: add r2.xy, -r2.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r2.xy = ((-(r2.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 270: add r2.xy, -r2.zwzz, r2.xyxx
    r2.xy = ((-(r2.zwzz))+(r2.xyxx)).xy;
    // 271: mad r2.xy, cb0[16].wwww, r2.xyxx, r2.zwzz
    r2.xy = ((source[16].wwww)*(r2.xyxx)+(r2.zwzz)).xy;
    // 272: mul r0.w, cb0[16].y, cb0[23].z
    r0.w = ((source[16].yyyy)*(source[23].zzzz)).w;
    // 273: mul r0.w, r0.w, l(0.628319)
    r0.w = ((r0.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 274: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 275: mul r4.y, r0.w, l(0.020000)
    r4.y = ((r0.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 276: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 277: mul r0.w, r0.w, l(0.500000)
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 278: mul r1.w, cb0[16].x, l(0.001000)
    r1.w = ((source[16].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 279: mov r4.x, l(0)
    r4.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 280: mad r2.xy, r1.wwww, r2.xyxx, r4.xyxx
    r2.xy = ((r1.wwww)*(r2.xyxx)+(r4.xyxx)).xy;
    // 281: dp2 r1.w, cb0[17].xyxx, r2.xyxx
    r1.w = (dot((source[17].xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 282: dp2 r2.y, cb0[18].xyxx, r2.xyxx
    r2.y = (dot((source[18].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 283: frc r1.w, r1.w
    r1.w = (frac(r1.wwww)).w;
    // 284: mul r2.x, r1.w, l(0.125000)
    r2.x = ((r1.wwww)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 285: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r2.xyxx, t6.xyzw, s6, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 286: mad r2.xyz, r2.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r2.xyz = ((r2.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 287: mul r1.w, r2.w, l(0.900000)
    r1.w = ((r2.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 288: mad r2.xyz, r1.wwww, r2.xyzx, r1.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 289: mul_sat r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (saturate((r0.wwww)*(r2.xyzx))).xyz;
    // 290: mad r4.xyz, cb0[16].zzzz, r2.xyzx, -r1.xyzx
    r4.xyz = ((source[16].zzzz)*(r2.xyzx)+(-(r1.xyzx))).xyz;
    // 291: mul r2.xyz, r2.xyzx, cb0[16].zzzz
    r2.xyz = ((r2.xyzx)*(source[16].zzzz)).xyz;
    // 292: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 293: mul r0.w, r0.w, l(3.000000)
    r0.w = ((r0.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 294: mad r1.xyz, r0.wwww, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 295: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 296: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 297: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 298: mul r2.xyz, r0.wwww, r3.xyzx
    r2.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 299: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 300: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 301: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 302: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 303: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 304: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 305: mul r3.yzw, r3.yyyy, cb0[28].xxyz
    r3.yzw = ((r3.yyyy)*(source[28].xxyz)).yzw;
    // 306: mad r3.xyz, r3.xxxx, cb0[27].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[27].xyzx)+(r3.yzwy)).xyz;
    // 307: mul r3.xyz, r3.xyzx, cb0[29].wwww
    r3.xyz = ((r3.xyzx)*(source[29].wwww)).xyz;
    // 308: mad r0.xyz, r3.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 309: mul r3.xyz, r1.xyzx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 310: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 311: mad o0.xyz, r1.xyzx, cb0[29].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[29].xyzx)+(r0.xyzx)).xyz;
    // 312: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 313: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 314: dp3 r0.x, r5.xyzx, r2.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 315: dp3 r0.z, r7.xyzx, r2.xyzx
    r0.z = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 316: dp3 r0.y, r8.xyzx, r2.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 317: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 318: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 319: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 320: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 321: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 322: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 323: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 324: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 325: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 326: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 327: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 328: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 329: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 330: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 331: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 332: ret
    return output;
}

// source.character.selection-native-600.v1 / source program 040a63ec2e3e5e42a8f2194c6622723a
