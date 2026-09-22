// source.character.static-map-native-237.v1 / source program aa9cc8cfd384c84b8c71cbec7d57b93f
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
