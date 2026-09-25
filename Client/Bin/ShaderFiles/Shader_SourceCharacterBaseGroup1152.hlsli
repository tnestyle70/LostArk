SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1152(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
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
    // 26: mul r3.xyz, cb0[3].xyzx, cb0[5].zzzz
    r3.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xzwy, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 28: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.yyyy
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 29: mul r6.xyz, r3.xyzx, r4.yyyy
    r6.xyz = ((r3.xyzx)*(r4.yyyy)).xyz;
    // 30: mad r1.yzw, r6.xxyz, r5.xxyz, r1.yyyy
    r1.yzw = ((r6.xxyz)*(r5.xxyz)+(r1.yyyy)).yzw;
    // 31: mul r1.yzw, r1.yyzw, cb0[7].xxyz
    r1.yzw = ((r1.yyzw)*(source[7].xxyz)).yzw;
    // 32: mad r5.xyz, r4.yyyy, r3.xyzx, -r1.xxxx
    r5.xyz = ((r4.yyyy)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 33: mad r5.xyz, r6.xyzx, r5.xyzx, r1.xxxx
    r5.xyz = ((r6.xyzx)*(r5.xyzx)+(r1.xxxx)).xyz;
    // 34: mad r1.xyz, r5.xyzx, cb0[6].xyzx, r1.yzwy
    r1.xyz = ((r5.xyzx)*(source[6].xyzx)+(r1.yzwy)).xyz;
    // 35: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 36: mad r3.xyz, -r4.yyyy, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(r4.yyyy))*(r3.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 38: add r5.xyz, -r0.xyzx, r0.wwww
    r5.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 39: mul r0.w, r4.z, cb0[4].z
    r0.w = ((r4.zzzz)*(source[4].zzzz)).w;
    // 40: mad r0.xyz, r0.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 41: mad r5.xyz, cb0[4].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[4].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: mad r4.yzw, r4.zzzz, r5.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r4.yzw = ((r4.zzzz)*(r5.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 43: mul r5.xyz, r4.xxxx, cb0[2].xyzx
    r5.xyz = ((r4.xxxx)*(source[2].xyzx)).xyz;
    // 44: mul r5.xyz, r5.xyzx, cb0[5].xxxx
    r5.xyz = ((r5.xyzx)*(source[5].xxxx)).xyz;
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
    // 54: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t4.xyzw, s3
    r6.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 55: mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // 56: dp3 r0.w, r6.xyzx, r3.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 57: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
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
    // 76: add r1.w, cb0[5].y, l(1.000000)
    r1.w = ((source[5].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 83: add r3.xyz, r1.yzwy, cb0[0].xyzx
    r3.xyz = ((r1.yzwy)+(source[0].xyzx)).xyz;
    // 84: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 85: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 86: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 87: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 88: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 89: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 90: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 91: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 92: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 93: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 94: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 95: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 96: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 97: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 98: dp3 r0.y, r3.xyzx, r2.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 99: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 100: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 101: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 102: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 103: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 104: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 105: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 106: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 107: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 108: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 109: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 110: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 111: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 113: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 114: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 115: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 116: ret
    return output;
}

// source.character.static-map-native-1152.v1 / source program 6449403c587a784295a7fad9dd9ae108
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1152(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1152(input);
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

// source.character.static-map-native-1153.v1 / source program 5055c1632d1e6e4486b175a1b021b0a3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1153(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[13]=1.f;
    source[14]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[4].xyzx, cb0[8].xxxx
    r1.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 8: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 11: mad r2.xy, r2.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 13: mul r2.xy, r2.xyxx, cb0[7].wwww
    r2.xy = ((r2.xyxx)*(source[7].wwww)).xy;
    // 14: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 15: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 16: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 17: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 18: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 25: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 26: mul r4.xyz, r0.wwww, r2.xyzx
    r4.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 27: mad r1.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 28: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 29: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 30: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 31: log r1.xyz, r4.xyzx
    r1.xyz = (log2(r4.xyzx)).xyz;
    // 32: add r0.w, cb0[8].w, l(1.000000)
    r0.w = ((source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 34: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 35: sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t4.xyzw, s3
    r4.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 36: mul r4.xyz, r4.xyzx, cb0[14].xyzx
    r4.xyz = ((r4.xyzx)*(source[14].xyzx)).xyz;
    // 37: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 38: dp2_sat r1.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 39: dp3_sat r1.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 40: dp3_sat r1.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 41: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 42: dp3 r1.x, r4.xyzx, r1.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 43: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t3.wxyz, s3
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 44: mul r1.yzw, r1.yyzw, cb0[13].xxyz
    r1.yzw = ((r1.yyzw)*(source[13].xxyz)).yzw;
    // 45: mul r4.xyz, r1.xxxx, r1.yzwy
    r4.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 46: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 47: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 48: mul r5.xyz, r2.wwww, v7.xyzx
    r5.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 49: dp3 r2.w, r5.xyzx, r2.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 50: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 51: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 52: mul r5.yzw, r5.yyyy, cb0[11].xxyz
    r5.yzw = ((r5.yyyy)*(source[11].xxyz)).yzw;
    // 53: mad r5.xyz, r5.xxxx, cb0[10].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[10].xyzx)+(r5.yzwy)).xyz;
    // 54: mul r5.xyz, r5.xyzx, cb0[12].wwww
    r5.xyz = ((r5.xyzx)*(source[12].wwww)).xyz;
    // 55: mul r6.xyz, cb0[5].xyzx, cb0[8].yyyy
    r6.xyz = ((source[5].xyzx)*(source[8].yyyy)).xyz;
    // 56: mul r6.xyz, r3.xyzx, r6.xyzx
    r6.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 57: mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 58: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 59: mad r5.xyz, r1.yzwy, r1.xxxx, r5.xyzx
    r5.xyz = ((r1.yzwy)*(r1.xxxx)+(r5.xyzx)).xyz;
    // 60: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 61: div r5.xyz, r4.xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)/(r5.xyzx)).xyz;
    // 62: mad r4.xyz, r6.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 63: dp3 r1.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 64: mul r5.xyz, cb0[6].xyzx, cb0[8].zzzz
    r5.xyz = ((source[6].xyzx)*(source[8].zzzz)).xyz;
    // 65: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 66: mul_sat r2.w, r3.w, cb0[9].x
    r2.w = (saturate((r3.wwww)*(source[9].xxxx))).w;
    // 67: mul o0.w, r2.w, cb0[0].x
    output.targets[0].w = ((r2.wwww)*(source[0].xxxx)).w;
    // 68: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 69: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 70: mad r3.xyz, r1.yzwy, r0.wwww, r4.xyzx
    r3.xyz = ((r1.yzwy)*(r0.wwww)+(r4.xyzx)).xyz;
    // 71: mul r1.yzw, r0.wwww, r1.yyzw
    r1.yzw = ((r0.wwww)*(r1.yyzw)).yzw;
    // 72: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 73: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 74: mad r0.xyz, r6.xyzx, cb0[12].xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(source[12].xyzx)+(r0.xyzx)).xyz;
    // 75: mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // 76: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 77: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 78: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 79: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 80: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 81: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 82: mul r1.yzw, r0.wwww, v0.xxyz
    r1.yzw = ((r0.wwww)*(v0.xxyz)).yzw;
    // 83: mul r4.xyz, r0.zxyz, r1.zwyz
    r4.xyz = ((r0.zxyz)*(r1.zwyz)).xyz;
    // 84: mad r4.xyz, r0.yzxy, r1.wyzw, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r1.wyzw)+(-(r4.xyzx))).xyz;
    // 85: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 86: dp3 r0.x, r1.yzwy, r2.xyzx
    r0.x = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).x;
    // 87: mul r1.yzw, r4.xxyz, v1.wwww
    r1.yzw = ((r4.xxyz)*(v1.wwww)).yzw;
    // 88: dp3 r0.y, r1.yzwy, r2.xyzx
    r0.y = (dot((r1.yzwy).xyz,(r2.xyzx).xyz).xxxx).y;
    // 89: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 90: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 91: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 92: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 93: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 94: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 95: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 96: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 97: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 98: movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 99: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 100: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 101: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 102: mul o4.z, r1.x, r3.x
    output.targets[4].z = ((r1.xxxx)*(r3.xxxx)).z;
    // 103: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 104: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 106: ret
    return output;
}

// source.character.static-map-native-1153.v1 / source program 84885c256921b44da3b51c41f462e064
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1153(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1153(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=g_SourceCharacterBaseConstants[8];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.xy, v4.xyxx, cb0[3].xyxx
    r0.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[4].xyzx, cb0[7].xxxx
    r1.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 8: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.xyxx, t0.zwxy, s0, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 11: mad r2.xy, r2.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 12: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 13: mul r2.xy, r2.xyxx, cb0[6].wwww
    r2.xy = ((r2.xyxx)*(source[6].wwww)).xy;
    // 14: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 15: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 16: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 17: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 18: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 19: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 20: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 21: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 22: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 25: dp3 r0.w, r1.xyzx, r2.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 26: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 27: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 28: mul r1.yzw, r1.yyyy, cb0[10].xxyz
    r1.yzw = ((r1.yyyy)*(source[10].xxyz)).yzw;
    // 29: mad r1.xyz, r1.xxxx, cb0[9].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[9].xyzx)+(r1.yzwy)).xyz;
    // 30: mul r1.xyz, r1.xyzx, cb0[11].wwww
    r1.xyz = ((r1.xyzx)*(source[11].wwww)).xyz;
    // 31: mul r4.xyz, cb0[5].xyzx, cb0[7].yyyy
    r4.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 32: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 33: mul_sat r0.w, r3.w, cb0[8].x
    r0.w = (saturate((r3.wwww)*(source[8].xxxx))).w;
    // 34: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 35: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 36: mad r0.xyz, r1.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 37: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 38: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 39: mad r0.xyz, r3.xyzx, cb0[11].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[11].xyzx)+(r0.xyzx)).xyz;
    // 40: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 41: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 42: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 43: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 44: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 45: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 46: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 47: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 48: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 49: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 50: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 51: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 52: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 53: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 54: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 55: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 56: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 57: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 58: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 59: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 60: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 61: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 62: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 63: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 64: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 66: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 67: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 68: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 69: ret
    return output;
}

// source.character.static-map-native-1154.v1 / source program 0efbe6e280ae864d97e95c30383ac16d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1154(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
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
    // 34: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 35: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[9].zzzz
    r0.zw = ((v4.xxxy)*(source[9].zzzz)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
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
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 51: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: add r0.yzw, -r4.xxyz, r0.yyyy
    r0.yzw = ((-(r4.xxyz))+(r0.yyyy)).yzw;
    // 56: mad r0.yzw, cb0[10].xxxx, r0.yyzw, r4.xxyz
    r0.yzw = ((source[10].xxxx)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 57: mul r4.xyz, cb0[6].xyzx, cb0[10].zzzz
    r4.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 58: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 59: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 60: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 61: mad r4.xyz, cb0[11].xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((source[11].xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 62: mul r7.xyz, cb0[5].xyzx, cb0[10].yyyy
    r7.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 63: mad r4.xyz, -r0.yzwy, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.yzwy))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 64: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 65: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 66: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 67: mul r4.xyz, r0.yzwy, cb0[11].yyyy
    r4.xyz = ((r0.yzwy)*(source[11].yyyy)).xyz;
    // 68: mad r0.yzw, cb0[11].zzzz, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].zzzz)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 69: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 70: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 71: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 73: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 74: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 75: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 77: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 78: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 80: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 81: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 82: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 83: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 84: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 85: mul r1.y, r1.y, cb0[12].w
    r1.y = ((r1.yyyy)*(source[12].wwww)).y;
    // 86: log r1.z, |r1.x|
    r1.z = (log2(abs(r1.xxxx))).z;
    // 87: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 88: mul r1.z, r1.z, cb0[13].z
    r1.z = ((r1.zzzz)*(source[13].zzzz)).z;
    // 89: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 90: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 91: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 92: mad r8.xyz, r1.xxxx, r8.xyzx, r9.xyzx
    r8.xyz = ((r1.xxxx)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 93: mad r0.yzw, r8.xxyz, r1.xxxx, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r1.xxxx)+(r0.yyzw)).yzw;
    // 94: mul r0.yzw, r1.xxxx, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r0.yyzw)).yzw;
    // 95: max r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = (max(r0.yyzw,r1.xxxx)).yzw;
    // 96: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 97: mad r2.xyz, r0.xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 98: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 99: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 100: mul r8.xyz, r0.xxxx, r2.xyzx
    r8.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 101: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 102: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 103: mul r9.xyz, r0.xxxx, v6.xyzx
    r9.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 104: dp3 r0.x, r9.xyzx, r8.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 105: mad r1.zw, r0.xxxx, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.xxxx)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 106: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 107: mul r9.xyz, r1.wwww, cb0[25].xyzx
    r9.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 108: mad r9.xyz, r1.zzzz, cb0[24].xyzx, r9.xyzx
    r9.xyz = ((r1.zzzz)*(source[24].xyzx)+(r9.xyzx)).xyz;
    // 109: mul r9.xyz, r9.xyzx, cb0[26].wwww
    r9.xyz = ((r9.xyzx)*(source[26].wwww)).xyz;
    // 110: mul r10.xyz, r4.xyzx, r9.xyzx
    r10.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 111: dp2_sat r11.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r11.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 112: dp3_sat r11.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r11.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 113: dp3_sat r11.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r11.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 114: mul r11.xyz, r11.xyzx, r11.xyzx
    r11.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 115: sample_indexable(texture2d)(float,float,float,float) r12.xyz, v3.zwzz, t8.xyzw, s5
    r12.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 116: mul r12.xyz, r12.xyzx, cb0[28].xyzx
    r12.xyz = ((r12.xyzx)*(source[28].xyzx)).xyz;
    // 117: dp3 r0.x, r12.xyzx, r11.xyzx
    r0.x = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 118: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t7.xyzw, s5
    r11.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 119: mul r11.xyz, r11.xyzx, cb0[27].xyzx
    r11.xyz = ((r11.xyzx)*(source[27].xyzx)).xyz;
    // 120: mul r13.xyz, r0.xxxx, r11.xyzx
    r13.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 121: mad r10.xyz, r4.xyzx, r13.xyzx, r10.xyzx
    r10.xyz = ((r4.xyzx)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 122: mul r0.yzw, r0.yyzw, r10.xxyz
    r0.yzw = ((r0.yyzw)*(r10.xxyz)).yzw;
    // 123: dp3 r10.x, r3.xyzx, r8.xyzx
    r10.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 124: dp3 r10.y, r6.xyzx, r8.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 125: dp2 r13.z, r10.xyxx, cb0[15].xyxx
    r13.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 126: dp3 r13.y, r5.xyzx, r8.xyzx
    r13.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 127: mul r1.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r1.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 128: dp2 r13.x, r10.xyxx, r1.zwzz
    r13.x = (dot((r10.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 129: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 130: dp4 r14.x, cb0[16].xyzw, r13.xyzw
    r14.x = (dot((source[16].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 131: dp4 r14.y, cb0[17].xyzw, r13.xyzw
    r14.y = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 132: dp4 r14.z, cb0[18].xyzw, r13.xyzw
    r14.z = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 133: mul r15.xyzw, r13.yzzx, r13.xyzz
    r15.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 134: dp4 r16.x, cb0[19].xyzw, r15.xyzw
    r16.x = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 135: dp4 r16.y, cb0[20].xyzw, r15.xyzw
    r16.y = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 136: dp4 r16.z, cb0[21].xyzw, r15.xyzw
    r16.z = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 137: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 138: mul r2.w, r13.y, r13.y
    r2.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 139: mov r10.z, r13.y
    r10.z = (r13.yyyy).z;
    // 140: mad r2.w, r13.x, r13.x, -r2.w
    r2.w = ((r13.xxxx)*(r13.xxxx)+(-(r2.wwww))).w;
    // 141: mad r13.xyz, cb0[22].xyzx, r2.wwww, r14.xyzx
    r13.xyz = ((source[22].xyzx)*(r2.wwww)+(r14.xyzx)).xyz;
    // 142: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 143: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 144: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 145: mov_sat r4.w, cb0[12].y
    r4.w = (saturate(source[12].yyyy)).w;
    // 146: mad r14.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r14.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 147: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 148: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 149: mad r14.xyz, r7.wwww, r14.xyzx, r2.wwww
    r14.xyz = ((r7.wwww)*(r14.xyzx)+(r2.wwww)).xyz;
    // 150: mul_sat r2.w, r14.y, l(50.000000)
    r2.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 151: log r3.w, |r1.y|
    r3.w = (log2(abs(r1.yyyy))).w;
    // 152: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 153: mul r3.w, r3.w, cb0[13].x
    r3.w = ((r3.wwww)*(source[13].xxxx)).w;
    // 154: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 155: movc r1.y, r1.y, l(0), r3.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).y;
    // 156: max r1.y, r1.y, cb0[0].x
    r1.y = (max(r1.yyyy,source[0].xxxx)).y;
    // 157: min r7.z, r1.y, l(1.000000)
    r7.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 158: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 159: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 160: mul r15.xyz, r1.yyyy, v5.xyzx
    r15.xyz = ((r1.yyyy)*(v5.xyzx)).xyz;
    // 161: dp3 r1.y, r8.xyzx, r15.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r15.xyzx).xyz).xxxx).y;
    // 162: mul r8.xyz, r1.yyyy, r8.xyzx
    r8.xyz = ((r1.yyyy)*(r8.xyzx)).xyz;
    // 163: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 164: deriv_rtx_coarse r7.x, r1.y
    r7.x = (ddx_coarse(r1.yyyy)).x;
    // 165: deriv_rty_coarse r7.y, r1.y
    r7.y = (ddy_coarse(r1.yyyy)).y;
    // 166: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: dp2 r3.w, r7.xyxx, r7.xyxx
    r3.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 168: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 169: mad_sat r7.y, r3.w, l(0.300000), r7.z
    r7.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 170: add r3.w, -r7.y, l(1.000000)
    r3.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: max r16.xyz, r14.xyzx, r3.wwww
    r16.xyz = (max(r14.xyzx,r3.wwww)).xyz;
    // 172: add r16.xyz, -r14.xyzx, r16.xyzx
    r16.xyz = ((-(r14.xyzx))+(r16.xyzx)).xyz;
    // 173: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 174: add r2.w, r8.z, l(1.000000)
    r2.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: add_sat r7.x, r1.y, -r2.w
    r7.x = (saturate((r1.yyyy)+(-(r2.wwww)))).x;
    // 177: sample_indexable(texture2d)(float,float,float,float) r17.xy, r7.xyxx, t5.xyzw, s7
    r17.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 178: add r1.y, r1.x, r7.x
    r1.y = ((r1.xxxx)+(r7.xxxx)).y;
    // 179: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 180: mul r18.xyz, r14.xyzx, r17.yyyy
    r18.xyz = ((r14.xyzx)*(r17.yyyy)).xyz;
    // 181: mad r16.xyz, r16.xyzx, r17.xxxx, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xxxx)+(r18.xyzx)).xyz;
    // 182: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r17.y
    r2.w = r17.y != 0.f ? 1.f / r17.y : 0.f;
    // 183: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 184: mad r17.xyz, r14.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r14.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: dp3 r2.w, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: mad r14.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 187: mad r18.xyz, -r16.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r16.xyz, r16.xyzx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xyzx)).xyz;
    // 189: mul r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)*(r18.xyzx)).xyz;
    // 190: mul r0.yzw, r0.yyzw, r13.xxyz
    r0.yzw = ((r0.yyzw)*(r13.xxyz)).yzw;
    // 191: mad r0.yzw, -r0.yyzw, r7.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r7.wwww)+(r0.yyzw)).yzw;
    // 192: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 193: dp3 r3.x, r3.xyzx, r8.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 194: dp3 r3.y, r6.xyzx, r8.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 195: dp2 r6.x, r3.xyxx, r1.zwzz
    r6.x = (dot((r3.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 196: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 197: mul r1.z, r7.y, l(5.000000)
    r1.z = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 198: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 199: mul r1.y, r1.y, r1.w
    r1.y = ((r1.yyyy)*(r1.wwww)).y;
    // 200: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 201: add r1.y, r1.x, r1.y
    r1.y = ((r1.xxxx)+(r1.yyyy)).y;
    // 202: mov o5.y, r1.x
    output.targets[5].y = (r1.xxxx).y;
    // 203: add_sat r1.x, r1.y, l(-1.000000)
    r1.x = (saturate((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 204: dp3 r6.y, r5.xyzx, r8.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 205: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s6, r1.z
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r1.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 206: mul r1.yzw, r3.xxyz, r3.wwww
    r1.yzw = ((r3.xxyz)*(r3.wwww)).yzw;
    // 207: mul r1.yzw, r1.yyzw, cb0[14].xxyz
    r1.yzw = ((r1.yyzw)*(source[14].xxyz)).yzw;
    // 208: mad r1.yzw, r1.yyzw, l(0.000000, 6.000000, 6.000000, 6.000000), cb0[14].wwww
    r1.yzw = ((r1.yyzw)*(float4(0.000000,6.000000,6.000000,6.000000))+(source[14].wwww)).yzw;
    // 209: dp2_sat r3.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 210: dp3_sat r3.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 211: dp3_sat r3.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 212: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 213: dp3 r2.w, r12.xyzx, r3.xyzx
    r2.w = (dot((r12.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 214: add r0.x, r0.x, -r2.w
    r0.x = ((r0.xxxx)+(-(r2.wwww))).x;
    // 215: mad r0.x, r7.z, r0.x, r2.w
    r0.x = ((r7.zzzz)*(r0.xxxx)+(r2.wwww)).x;
    // 216: mad r3.xyz, r11.xyzx, r0.xxxx, r9.xyzx
    r3.xyz = ((r11.xyzx)*(r0.xxxx)+(r9.xyzx)).xyz;
    // 217: mul r5.xyz, r0.xxxx, r11.xyzx
    r5.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 218: mad r0.x, r1.x, r14.x, r14.y
    r0.x = ((r1.xxxx)*(r14.xxxx)+(r14.yyyy)).x;
    // 219: mad r0.x, r0.x, r1.x, r14.z
    r0.x = ((r0.xxxx)*(r1.xxxx)+(r14.zzzz)).x;
    // 220: mul r0.x, r1.x, r0.x
    r0.x = ((r1.xxxx)*(r0.xxxx)).x;
    // 221: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 222: mul r6.xyz, r0.xxxx, r3.xyzx
    r6.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 223: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 224: div r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)/(r3.xyzx)).xyz;
    // 225: dp3 r0.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 226: mul r1.xyz, r1.yzwy, r6.xyzx
    r1.xyz = ((r1.yzwy)*(r6.xyzx)).xyz;
    // 227: mad r0.yzw, r1.xxyz, r16.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r16.xxyz)+(r0.yyzw)).yzw;
    // 228: mul r1.xyz, r16.xyzx, r1.xyzx
    r1.xyz = ((r16.xyzx)*(r1.xyzx)).xyz;
    // 229: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 230: dp3 r1.x, r2.xyzx, r15.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r15.xyzx).xyz).xxxx).x;
    // 231: add r1.y, -|r15.z|, l(1.000000)
    r1.y = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 232: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 233: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 234: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 235: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 236: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 237: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 238: mul r1.xzw, r1.xxxx, cb0[4].xxyz
    r1.xzw = ((r1.xxxx)*(source[4].xxyz)).xzw;
    // 239: movc r1.xyz, r1.yyyy, l(0,0,0,0), r1.xzwx
    r1.xyz = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xzwx)).xyz;
    // 240: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 241: add r1.xyz, r0.yzwy, r1.xyzx
    r1.xyz = ((r0.yzwy)+(r1.xyzx)).xyz;
    // 242: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 243: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 244: dp3 r1.x, r10.xyzx, r10.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 245: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 246: mul r1.xyz, r1.xxxx, r10.xyzx
    r1.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 247: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 248: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 249: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 250: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 251: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 252: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 253: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 254: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 255: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 256: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 257: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 258: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
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

// source.character.static-map-native-1154.v1 / source program f9b6ae28628ff7439beac24675e2984a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1154(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1154(input);
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
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
    // 34: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 35: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r0.zw, v4.xxxy, cb0[9].zzzz
    r0.zw = ((v4.xxxy)*(source[9].zzzz)).zw;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
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
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 51: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: add r0.yzw, -r4.xxyz, r0.yyyy
    r0.yzw = ((-(r4.xxyz))+(r0.yyyy)).yzw;
    // 56: mad r0.yzw, cb0[10].xxxx, r0.yyzw, r4.xxyz
    r0.yzw = ((source[10].xxxx)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 57: mul r4.xyz, cb0[6].xyzx, cb0[10].zzzz
    r4.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 58: mul r8.xyz, r7.xyzx, r4.xyzx
    r8.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 59: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 60: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 61: mad r4.xyz, cb0[11].xxxx, r4.xyzx, r8.xyzx
    r4.xyz = ((source[11].xxxx)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 62: mul r7.xyz, cb0[5].xyzx, cb0[10].yyyy
    r7.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 63: mad r4.xyz, -r0.yzwy, r7.xyzx, r4.xyzx
    r4.xyz = ((-(r0.yzwy))*(r7.xyzx)+(r4.xyzx)).xyz;
    // 64: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 65: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 66: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 67: mul r4.xyz, r0.yzwy, cb0[11].yyyy
    r4.xyz = ((r0.yzwy)*(source[11].yyyy)).xyz;
    // 68: mad r0.yzw, cb0[11].zzzz, r0.yyzw, -r4.xxyz
    r0.yzw = ((source[11].zzzz)*(r0.yyzw)+(-(r4.xxyz))).yzw;
    // 69: mul r1.z, r1.z, cb0[11].w
    r1.z = ((r1.zzzz)*(source[11].wwww)).z;
    // 70: log r1.w, |r1.z|
    r1.w = (log2(abs(r1.zzzz))).w;
    // 71: lt r1.z, |r1.z|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 72: mul r1.w, r1.w, cb0[12].x
    r1.w = ((r1.wwww)*(source[12].xxxx)).w;
    // 73: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 74: movc r1.z, r1.z, l(0), r1.w
    r1.z = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).z;
    // 75: min r1.w, r1.z, l(1.000000)
    r1.w = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: mul_sat r7.w, r1.z, cb2[3].w
    r7.w = (saturate((r1.zzzz)*(passValues[3].wwww))).w;
    // 77: mad r0.yzw, r1.wwww, r0.yyzw, r4.xxyz
    r0.yzw = ((r1.wwww)*(r0.yyzw)+(r4.xxyz)).yzw;
    // 78: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 80: mad_sat r4.xyz, r0.yzwy, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r0.yzwy)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 81: mad r0.yzw, r4.xxyz, l(0.000000, 2.755200, 2.755200, 2.755200), l(0.000000, 0.690300, 0.690300, 0.690300)
    r0.yzw = ((r4.xxyz)*(float4(0.000000,2.755200,2.755200,2.755200))+(float4(0.000000,0.690300,0.690300,0.690300))).yzw;
    // 82: mad r8.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r8.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 83: mad r9.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r9.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 84: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 85: mul r1.y, r1.y, cb0[12].w
    r1.y = ((r1.yyyy)*(source[12].wwww)).y;
    // 86: log r1.z, |r1.x|
    r1.z = (log2(abs(r1.xxxx))).z;
    // 87: lt r1.x, |r1.x|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 88: mul r1.z, r1.z, cb0[13].z
    r1.z = ((r1.zzzz)*(source[13].zzzz)).z;
    // 89: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 90: min r1.z, r1.z, l(1.000000)
    r1.z = (min(r1.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 91: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 92: mad r8.xyz, r1.xxxx, r8.xyzx, r9.xyzx
    r8.xyz = ((r1.xxxx)*(r8.xyzx)+(r9.xyzx)).xyz;
    // 93: mad r0.yzw, r8.xxyz, r1.xxxx, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r1.xxxx)+(r0.yyzw)).yzw;
    // 94: mul r0.yzw, r1.xxxx, r0.yyzw
    r0.yzw = ((r1.xxxx)*(r0.yyzw)).yzw;
    // 95: max r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = (max(r0.yyzw,r1.xxxx)).yzw;
    // 96: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 97: mad r2.xyz, r0.xxxx, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 98: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 99: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 100: mul r8.xyz, r0.xxxx, r2.xyzx
    r8.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 101: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 102: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 103: mul r9.xyz, r0.xxxx, v6.xyzx
    r9.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 104: dp3 r0.x, r9.xyzx, r8.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 105: mad r1.zw, r0.xxxx, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.xxxx)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 106: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 107: mul r10.xyz, r1.wwww, cb0[25].xyzx
    r10.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 108: mad r10.xyz, r1.zzzz, cb0[24].xyzx, r10.xyzx
    r10.xyz = ((r1.zzzz)*(source[24].xyzx)+(r10.xyzx)).xyz;
    // 109: mul r10.xyz, r10.xyzx, cb0[26].wwww
    r10.xyz = ((r10.xyzx)*(source[26].wwww)).xyz;
    // 110: mul r10.xyz, r4.xyzx, r10.xyzx
    r10.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 111: mul r0.xyz, r0.yzwy, r10.xyzx
    r0.xyz = ((r0.yzwy)*(r10.xyzx)).xyz;
    // 112: dp3 r10.x, r3.xyzx, r8.xyzx
    r10.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 113: dp3 r10.y, r6.xyzx, r8.xyzx
    r10.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 114: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 115: dp3 r11.y, r5.xyzx, r8.xyzx
    r11.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 116: mul r1.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r1.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 117: dp2 r11.x, r10.xyxx, r1.zwzz
    r11.x = (dot((r10.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 118: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 119: dp4 r12.x, cb0[16].xyzw, r11.xyzw
    r12.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 120: dp4 r12.y, cb0[17].xyzw, r11.xyzw
    r12.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 121: dp4 r12.z, cb0[18].xyzw, r11.xyzw
    r12.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 122: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 123: dp4 r14.x, cb0[19].xyzw, r13.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 124: dp4 r14.y, cb0[20].xyzw, r13.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 125: dp4 r14.z, cb0[21].xyzw, r13.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 126: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 127: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 128: mov r10.z, r11.y
    r10.z = (r11.yyyy).z;
    // 129: mad r0.w, r11.x, r11.x, -r0.w
    r0.w = ((r11.xxxx)*(r11.xxxx)+(-(r0.wwww))).w;
    // 130: mad r11.xyz, cb0[22].xyzx, r0.wwww, r12.xyzx
    r11.xyz = ((source[22].xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 131: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 132: mul r11.xyz, r11.xyzx, cb0[14].xyzx
    r11.xyz = ((r11.xyzx)*(source[14].xyzx)).xyz;
    // 133: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 134: mov_sat r4.w, cb0[12].y
    r4.w = (saturate(source[12].yyyy)).w;
    // 135: mad r12.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r12.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 136: mul r0.w, r4.w, l(0.080000)
    r0.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 137: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 138: mad r12.xyz, r7.wwww, r12.xyzx, r0.wwww
    r12.xyz = ((r7.wwww)*(r12.xyzx)+(r0.wwww)).xyz;
    // 139: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 140: log r2.w, |r1.y|
    r2.w = (log2(abs(r1.yyyy))).w;
    // 141: lt r1.y, |r1.y|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 142: mul r2.w, r2.w, cb0[13].x
    r2.w = ((r2.wwww)*(source[13].xxxx)).w;
    // 143: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 144: movc r1.y, r1.y, l(0), r2.w
    r1.y = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).y;
    // 145: max r1.y, r1.y, cb0[0].x
    r1.y = (max(r1.yyyy,source[0].xxxx)).y;
    // 146: min r7.z, r1.y, l(1.000000)
    r7.z = (min(r1.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 147: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 148: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 149: mul r13.xyz, r1.yyyy, v5.xyzx
    r13.xyz = ((r1.yyyy)*(v5.xyzx)).xyz;
    // 150: dp3 r1.y, r8.xyzx, r13.xyzx
    r1.y = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 151: mul r8.xyz, r1.yyyy, r8.xyzx
    r8.xyz = ((r1.yyyy)*(r8.xyzx)).xyz;
    // 152: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 153: deriv_rtx_coarse r7.x, r1.y
    r7.x = (ddx_coarse(r1.yyyy)).x;
    // 154: deriv_rty_coarse r7.y, r1.y
    r7.y = (ddy_coarse(r1.yyyy)).y;
    // 155: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 156: dp2 r2.w, r7.xyxx, r7.xyxx
    r2.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 157: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 158: mad_sat r7.y, r2.w, l(0.300000), r7.z
    r7.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 159: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 160: add r2.w, -r7.y, l(1.000000)
    r2.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: max r14.xyz, r12.xyzx, r2.wwww
    r14.xyz = (max(r12.xyzx,r2.wwww)).xyz;
    // 162: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 163: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 164: add r0.w, r8.z, l(1.000000)
    r0.w = ((r8.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 165: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: add_sat r7.x, -r0.w, r1.y
    r7.x = (saturate((-(r0.wwww))+(r1.yyyy))).x;
    // 167: sample_indexable(texture2d)(float,float,float,float) r15.xy, r7.xyxx, t5.xyzw, s6
    r15.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 168: add r0.w, r1.x, r7.x
    r0.w = ((r1.xxxx)+(r7.xxxx)).w;
    // 169: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 170: mul r16.xyz, r12.xyzx, r15.yyyy
    r16.xyz = ((r12.xyzx)*(r15.yyyy)).xyz;
    // 171: mad r14.xyz, r14.xyzx, r15.xxxx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xxxx)+(r16.xyzx)).xyz;
    // 172: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r15.y
    r1.y = r15.y != 0.f ? 1.f / r15.y : 0.f;
    // 173: add r1.y, r1.y, l(-1.000000)
    r1.y = ((r1.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 174: mad r15.xyz, r12.xyzx, r1.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r12.xyzx)*(r1.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 175: dp3 r1.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 176: mad r12.xyz, r1.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r1.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 177: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 179: mul r11.xyz, r11.xyzx, r16.xyzx
    r11.xyz = ((r11.xyzx)*(r16.xyzx)).xyz;
    // 180: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 181: mad r0.xyz, -r0.xyzx, r7.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r7.wwww)+(r0.xyzx)).xyz;
    // 182: dp3 r3.x, r3.xyzx, r8.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 183: dp3 r3.y, r6.xyzx, r8.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 184: dp2 r6.x, r3.xyxx, r1.zwzz
    r6.x = (dot((r3.xyxx).xy,(r1.zwzz).xy).xxxx).x;
    // 185: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 186: mul r1.y, r7.y, l(5.000000)
    r1.y = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 187: mul r1.z, r7.y, r7.y
    r1.z = ((r7.yyyy)*(r7.yyyy)).z;
    // 188: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 189: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 190: add r0.w, r1.x, r0.w
    r0.w = ((r1.xxxx)+(r0.wwww)).w;
    // 191: mov o5.y, r1.x
    output.targets[5].y = (r1.xxxx).y;
    // 192: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 193: dp3 r6.y, r5.xyzx, r8.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 194: dp3 r1.x, r9.xyzx, r8.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 195: mad r1.xz, r1.xxxx, l(0.500000, 0.000000, -0.500000, 0.000000), l(0.500000, 0.000000, 0.500000, 0.000000)
    r1.xz = ((r1.xxxx)*(float4(0.500000,0.000000,-0.500000,0.000000))+(float4(0.500000,0.000000,0.500000,0.000000))).xz;
    // 196: mul r1.xz, r1.xxzx, r1.xxzx
    r1.xz = ((r1.xxzx)*(r1.xxzx)).xz;
    // 197: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s5, r1.y
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r1.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 198: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 199: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 200: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 201: mad r1.y, r0.w, r12.x, r12.y
    r1.y = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).y;
    // 202: mad r1.y, r1.y, r0.w, r12.z
    r1.y = ((r1.yyyy)*(r0.wwww)+(r12.zzzz)).y;
    // 203: mul r1.y, r0.w, r1.y
    r1.y = ((r0.wwww)*(r1.yyyy)).y;
    // 204: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 205: mul r1.yzw, r1.zzzz, cb0[25].xxyz
    r1.yzw = ((r1.zzzz)*(source[25].xxyz)).yzw;
    // 206: mad r1.xyz, cb0[24].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[24].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 207: mul r1.xyz, r1.xyzx, cb0[26].wwww
    r1.xyz = ((r1.xyzx)*(source[26].wwww)).xyz;
    // 208: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 209: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 210: mad r0.xyz, r1.xyzx, r14.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r14.xyzx)+(r0.xyzx)).xyz;
    // 211: mul r1.xyz, r14.xyzx, r1.xyzx
    r1.xyz = ((r14.xyzx)*(r1.xyzx)).xyz;
    // 212: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 213: dp3 r0.w, r2.xyzx, r13.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 214: add r1.x, -|r13.z|, l(1.000000)
    r1.x = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 215: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 216: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 217: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 218: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 219: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 220: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 221: mul r1.yzw, r0.wwww, cb0[4].xxyz
    r1.yzw = ((r0.wwww)*(source[4].xxyz)).yzw;
    // 222: movc r1.xyz, r1.xxxx, l(0,0,0,0), r1.yzwy
    r1.xyz = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yzwy)).xyz;
    // 223: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 224: add r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 225: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 226: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r1.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r1.xyzx)).xyz;
    // 227: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 228: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 229: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 230: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 231: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 232: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 233: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 234: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 235: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 236: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 237: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 238: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 239: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
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

// source.character.static-map-native-1155.v1 / source program 838b121532c46b46a1ca0129cb56267a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1155(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 56: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r8.xyz, -r4.xyzx, r0.yyyy
    r8.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 58: mad r4.xyz, cb0[10].yyyy, r8.xyzx, r4.xyzx
    r4.xyz = ((source[10].yyyy)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 59: mul r8.xyz, cb0[6].xyzx, cb0[10].wwww
    r8.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 60: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 61: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 62: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 63: mad r7.xyz, cb0[11].yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((source[11].yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 64: mul r8.xyz, cb0[5].xyzx, cb0[10].zzzz
    r8.xyz = ((source[5].xyzx)*(source[10].zzzz)).xyz;
    // 65: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 66: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 67: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 68: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 69: mul r7.xyz, r4.xyzx, cb0[11].zzzz
    r7.xyz = ((r4.xyzx)*(source[11].zzzz)).xyz;
    // 70: mad r4.xyz, cb0[11].wwww, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 71: mul r0.y, r1.z, cb0[12].x
    r0.y = ((r1.zzzz)*(source[12].xxxx)).y;
    // 72: mul r1.xy, r1.yxyy, cb0[13].xzxx
    r1.xy = ((r1.yxyy)*(source[13].xzxx)).xy;
    // 73: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 74: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 75: mul r1.z, r1.z, cb0[12].y
    r1.z = ((r1.zzzz)*(source[12].yyyy)).z;
    // 76: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 77: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 78: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 80: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 81: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 83: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 85: mul r7.xy, r0.zwzz, cb0[8].wwww
    r7.xy = ((r0.zwzz)*(source[8].wwww)).xy;
    // 86: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 87: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 88: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 89: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 90: add r0.yzw, -r2.xxyz, r7.xxyz
    r0.yzw = ((-(r2.xxyz))+(r7.xxyz)).yzw;
    // 91: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 92: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 93: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 94: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 95: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 96: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 97: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 98: dp3 r0.w, r7.xyzx, r2.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 99: mad r1.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 100: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 101: mul r7.xyz, r1.wwww, cb0[25].xyzx
    r7.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 102: mad r7.xyz, r1.zzzz, cb0[24].xyzx, r7.xyzx
    r7.xyz = ((r1.zzzz)*(source[24].xyzx)+(r7.xyzx)).xyz;
    // 103: mul r7.xyz, r7.xyzx, cb0[26].wwww
    r7.xyz = ((r7.xyzx)*(source[26].wwww)).xyz;
    // 104: mul r9.xyz, r4.xyzx, r7.xyzx
    r9.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 105: dp2_sat r10.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 106: dp3_sat r10.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 107: dp3_sat r10.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 108: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 109: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t9.xyzw, s6
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 110: mul r11.xyz, r11.xyzx, cb0[28].xyzx
    r11.xyz = ((r11.xyzx)*(source[28].xyzx)).xyz;
    // 111: dp3 r0.w, r11.xyzx, r10.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 112: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s6
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 113: mul r10.xyz, r10.xyzx, cb0[27].xyzx
    r10.xyz = ((r10.xyzx)*(source[27].xyzx)).xyz;
    // 114: mul r12.xyz, r0.wwww, r10.xyzx
    r12.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 115: mad r9.xyz, r4.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 116: mad r12.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r12.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 117: mad r13.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 118: mad r14.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 119: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 120: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 121: mul r1.zw, r1.zzzw, cb0[13].yyyw
    r1.zw = ((r1.zzzw)*(source[13].yyyw)).zw;
    // 122: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 123: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 125: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 126: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 127: mad r1.xzw, r1.yyyy, r13.xxyz, r14.xxyz
    r1.xzw = ((r1.yyyy)*(r13.xxyz)+(r14.xxyz)).xzw;
    // 128: mad r1.xzw, r1.xxzw, r1.yyyy, r12.xxyz
    r1.xzw = ((r1.xxzw)*(r1.yyyy)+(r12.xxyz)).xzw;
    // 129: mul r1.xzw, r1.yyyy, r1.xxzw
    r1.xzw = ((r1.yyyy)*(r1.xxzw)).xzw;
    // 130: max r1.xzw, r1.xxzw, r1.yyyy
    r1.xzw = (max(r1.xxzw,r1.yyyy)).xzw;
    // 131: mul r1.xzw, r1.xxzw, r9.xxyz
    r1.xzw = ((r1.xxzw)*(r9.xxyz)).xzw;
    // 132: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 133: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 134: dp2 r12.z, r9.xyxx, cb0[15].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 135: dp3 r12.y, r5.xyzx, r2.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 136: mul r8.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 137: dp2 r12.x, r9.xyxx, r8.xyxx
    r12.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 138: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 139: dp4 r13.x, cb0[16].xyzw, r12.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 140: dp4 r13.y, cb0[17].xyzw, r12.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 141: dp4 r13.z, cb0[18].xyzw, r12.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 142: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 143: dp4 r15.x, cb0[19].xyzw, r14.xyzw
    r15.x = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 144: dp4 r15.y, cb0[20].xyzw, r14.xyzw
    r15.y = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 145: dp4 r15.z, cb0[21].xyzw, r14.xyzw
    r15.z = (dot((source[21].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 146: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 147: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 148: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 149: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 150: mad r12.xyz, cb0[22].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[22].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 151: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 152: mul r12.xyz, r12.xyzx, cb0[14].xyzx
    r12.xyz = ((r12.xyzx)*(source[14].xyzx)).xyz;
    // 153: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 154: mov_sat r4.w, cb0[12].z
    r4.w = (saturate(source[12].zzzz)).w;
    // 155: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 156: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 157: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 158: mad r13.xyz, r8.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r8.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 159: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 160: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 161: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 162: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 163: dp3 r3.w, r2.xyzx, r14.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 164: mul r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // 165: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 166: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 167: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 168: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 169: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 170: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 171: mad_sat r15.y, r4.w, l(0.300000), r8.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 172: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 174: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 175: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 176: add r2.w, r2.z, l(1.000000)
    r2.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 177: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 178: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 179: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t6.zwxy, s8
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 180: add r2.w, r1.y, r15.x
    r2.w = ((r1.yyyy)+(r15.xxxx)).w;
    // 181: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 182: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 183: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 184: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 185: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 186: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 187: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 188: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 189: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 190: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 191: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 192: mul r1.xzw, r1.xxzw, r12.xxyz
    r1.xzw = ((r1.xxzw)*(r12.xxyz)).xzw;
    // 193: mad r1.xzw, -r1.xxzw, r8.wwww, r1.xxzw
    r1.xzw = ((-(r1.xxzw))*(r8.wwww)+(r1.xxzw)).xzw;
    // 194: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 195: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 196: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 197: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 198: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 199: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 200: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 201: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 202: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 203: add r2.w, r1.y, r2.w
    r2.w = ((r1.yyyy)+(r2.wwww)).w;
    // 204: mov o5.y, r1.y
    output.targets[5].y = (r1.yyyy).y;
    // 205: add_sat r1.y, r2.w, l(-1.000000)
    r1.y = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 206: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 207: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t7.xyzw, s7, r3.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 208: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 209: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 210: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 211: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 212: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 213: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 214: mul r2.xyz, r5.xyzx, r5.xyzx
    r2.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 215: dp3 r2.x, r11.xyzx, r2.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 216: add r0.w, r0.w, -r2.x
    r0.w = ((r0.wwww)+(-(r2.xxxx))).w;
    // 217: mad r0.w, r8.z, r0.w, r2.x
    r0.w = ((r8.zzzz)*(r0.wwww)+(r2.xxxx)).w;
    // 218: mad r2.xyz, r10.xyzx, r0.wwww, r7.xyzx
    r2.xyz = ((r10.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 219: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 220: mad r0.w, r1.y, r13.x, r13.y
    r0.w = ((r1.yyyy)*(r13.xxxx)+(r13.yyyy)).w;
    // 221: mad r0.w, r0.w, r1.y, r13.z
    r0.w = ((r0.wwww)*(r1.yyyy)+(r13.zzzz)).w;
    // 222: mul r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)*(r0.wwww)).w;
    // 223: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 224: mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 225: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 226: div r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)/(r2.xyzx)).xyz;
    // 227: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 228: mul r2.xyz, r3.xyzx, r6.xyzx
    r2.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 229: mad r1.xyz, r2.xyzx, r15.xzwx, r1.xzwx
    r1.xyz = ((r2.xyzx)*(r15.xzwx)+(r1.xzwx)).xyz;
    // 230: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 231: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 232: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 233: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 234: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 236: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 237: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 238: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 239: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 240: mul r2.xyz, r0.xxxx, cb0[4].xyzx
    r2.xyz = ((r0.xxxx)*(source[4].xyzx)).xyz;
    // 241: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 242: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 243: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 244: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r0.xyzx)).xyz;
    // 245: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 246: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 247: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 248: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 249: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 250: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 251: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 252: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 253: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 254: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 255: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 256: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 257: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 258: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 259: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 260: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
    // 261: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 262: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 263: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 264: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 265: ret
    return output;
}

// source.character.static-map-native-1155.v1 / source program 976d96eebd8a7142a39aab70011a503c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1155(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1155(input);
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
    // 56: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 57: add r8.xyz, -r4.xyzx, r0.yyyy
    r8.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 58: mad r4.xyz, cb0[10].yyyy, r8.xyzx, r4.xyzx
    r4.xyz = ((source[10].yyyy)*(r8.xyzx)+(r4.xyzx)).xyz;
    // 59: mul r8.xyz, cb0[6].xyzx, cb0[10].wwww
    r8.xyz = ((source[6].xyzx)*(source[10].wwww)).xyz;
    // 60: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 61: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 62: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 63: mad r7.xyz, cb0[11].yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((source[11].yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 64: mul r8.xyz, cb0[5].xyzx, cb0[10].zzzz
    r8.xyz = ((source[5].xyzx)*(source[10].zzzz)).xyz;
    // 65: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 66: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 67: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 68: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 69: mul r7.xyz, r4.xyzx, cb0[11].zzzz
    r7.xyz = ((r4.xyzx)*(source[11].zzzz)).xyz;
    // 70: mad r4.xyz, cb0[11].wwww, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 71: mul r0.y, r1.z, cb0[12].x
    r0.y = ((r1.zzzz)*(source[12].xxxx)).y;
    // 72: mul r1.xy, r1.yxyy, cb0[13].xzxx
    r1.xy = ((r1.yxyy)*(source[13].xzxx)).xy;
    // 73: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 74: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 75: mul r1.z, r1.z, cb0[12].y
    r1.z = ((r1.zzzz)*(source[12].yyyy)).z;
    // 76: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 77: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 78: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 79: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 80: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 81: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 82: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 83: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 84: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 85: mul r7.xy, r0.zwzz, cb0[8].wwww
    r7.xy = ((r0.zwzz)*(source[8].wwww)).xy;
    // 86: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 87: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 88: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 89: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 90: add r0.yzw, -r2.xxyz, r7.xxyz
    r0.yzw = ((-(r2.xxyz))+(r7.xxyz)).yzw;
    // 91: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 92: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 93: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 94: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 95: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 96: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 97: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 98: dp3 r0.w, r7.xyzx, r2.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 99: mad r1.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 100: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 101: mul r9.xyz, r1.wwww, cb0[25].xyzx
    r9.xyz = ((r1.wwww)*(source[25].xyzx)).xyz;
    // 102: mad r9.xyz, r1.zzzz, cb0[24].xyzx, r9.xyzx
    r9.xyz = ((r1.zzzz)*(source[24].xyzx)+(r9.xyzx)).xyz;
    // 103: mul r9.xyz, r9.xyzx, cb0[26].wwww
    r9.xyz = ((r9.xyzx)*(source[26].wwww)).xyz;
    // 104: mul r9.xyz, r4.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 105: mad r10.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 106: mad r11.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 107: mad r12.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 108: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 109: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 110: mul r1.zw, r1.zzzw, cb0[13].yyyw
    r1.zw = ((r1.zzzw)*(source[13].yyyw)).zw;
    // 111: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 112: min r0.w, r1.w, l(1.000000)
    r0.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 113: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 114: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 115: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 116: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 117: mad r1.xyz, r0.wwww, r11.xyzx, r12.xyzx
    r1.xyz = ((r0.wwww)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 118: mad r1.xyz, r1.xyzx, r0.wwww, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r0.wwww)+(r10.xyzx)).xyz;
    // 119: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 120: max r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (max(r0.wwww,r1.xyzx)).xyz;
    // 121: mul r1.xyz, r1.xyzx, r9.xyzx
    r1.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 122: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 123: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 124: dp2 r10.z, r9.xyxx, cb0[15].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 125: dp3 r10.y, r5.xyzx, r2.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 126: mul r8.xy, cb0[15].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[15].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 127: dp2 r10.x, r9.xyxx, r8.xyxx
    r10.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 128: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r11.x, cb0[16].xyzw, r10.xyzw
    r11.x = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 130: dp4 r11.y, cb0[17].xyzw, r10.xyzw
    r11.y = (dot((source[17].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 131: dp4 r11.z, cb0[18].xyzw, r10.xyzw
    r11.z = (dot((source[18].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 132: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 133: dp4 r13.x, cb0[19].xyzw, r12.xyzw
    r13.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 134: dp4 r13.y, cb0[20].xyzw, r12.xyzw
    r13.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 135: dp4 r13.z, cb0[21].xyzw, r12.xyzw
    r13.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 136: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 137: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 138: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 139: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 140: mad r10.xyz, cb0[22].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[22].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 141: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 143: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 144: mov_sat r4.w, cb0[12].z
    r4.w = (saturate(source[12].zzzz)).w;
    // 145: mad r11.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r11.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 146: mul r1.w, r4.w, l(0.080000)
    r1.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 148: mad r11.xyz, r8.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r8.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 149: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 151: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 152: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 153: dp3 r2.w, r2.xyzx, r12.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 154: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 155: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
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
    // 166: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 167: add r1.w, r2.z, l(1.000000)
    r1.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
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
    // 183: mul r1.xyz, r1.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 184: mad r1.xyz, -r1.xyzx, r8.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r8.wwww)+(r1.xyzx)).xyz;
    // 185: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 186: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 187: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 188: dp2 r6.z, r3.xyxx, cb0[15].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 189: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 190: mul r3.x, r13.y, r13.y
    r3.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 191: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 192: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 193: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 194: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 195: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 196: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 197: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 198: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 199: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t7.xyzw, s6, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 202: mul r3.xyz, r3.xyzx, cb0[14].xyzx
    r3.xyz = ((r3.xyzx)*(source[14].xyzx)).xyz;
    // 203: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 204: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 205: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 206: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 207: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 208: mul r2.yzw, r2.yyyy, cb0[25].xxyz
    r2.yzw = ((r2.yyyy)*(source[25].xxyz)).yzw;
    // 209: mad r2.xyz, cb0[24].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[24].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[26].wwww
    r2.xyz = ((r2.xyzx)*(source[26].wwww)).xyz;
    // 211: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 212: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 213: mad r1.xyz, r2.xyzx, r13.xzwx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r13.xzwx)+(r1.xyzx)).xyz;
    // 214: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 215: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
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
    // 224: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 225: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 226: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 227: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 228: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 229: mad o0.xyz, r4.xyzx, cb0[26].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[26].xyzx)+(r0.xyzx)).xyz;
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
    // 243: ftou r0.x, cb0[23].z
    r0.x = (asfloat((uint4)(source[23].zzzz))).x;
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

// source.character.static-map-native-1156.v1 / source program d6937fdf673c9745b36c6e88ee4e9880
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1156(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.x, -v4.y, cb0[5].x
    r0.x = ((-(v4.yyyy))+(source[5].xxxx)).x;
    // 2: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 3: mul r0.x, r0.x, cb0[5].y
    r0.x = ((r0.xxxx)*(source[5].yyyy)).x;
    // 4: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 5: mul r1.xyz, r0.yzwy, r0.xxxx
    r1.xyz = ((r0.yzwy)*(r0.xxxx)).xyz;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 7: mul r2.xyz, r2.xyzx, cb0[2].xyzx
    r2.xyz = ((r2.xyzx)*(source[2].xyzx)).xyz;
    // 8: mul o0.w, r2.w, cb0[0].x
    output.targets[0].w = ((r2.wwww)*(source[0].xxxx)).w;
    // 9: mad r0.xyz, r0.xxxx, r0.yzwy, -r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(-(r2.xyzx))).xyz;
    // 10: mad r0.xyz, r1.xyzx, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 11: mul r1.xyz, r0.xyzx, cb0[5].zzzz
    r1.xyz = ((r0.xyzx)*(source[5].zzzz)).xyz;
    // 12: mad r0.xyz, -cb0[5].zzzz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((-(source[5].zzzz))*(r0.xyzx)+(source[4].xyzx)).xyz;
    // 13: mad r0.xyz, cb0[4].wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((source[4].wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 14: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 15: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 16: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 17: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 18: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 22: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 23: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 24: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 25: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 26: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 27: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 28: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 29: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 30: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 31: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 32: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 33: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 34: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 35: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 36: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 37: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 38: mov o3.xyz, cb2[3].xyzx
    output.targets[3].xyz = (passValues[3].xyzx).xyz;
    // 39: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 40: mov o4.xyzw, l(0,0,0,0)
    output.targets[4].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 41: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 42: ret
    return output;
}

// source.character.static-map-native-1157.v1 / source program b4da241ed0cf314e97f10148cb4af807
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1157(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[6]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[10]=g_SourceCharacterBaseConstants[9];
    source[10].y=(g_SourceCharacterTime.xxxx).x;
    source[10].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[11]=g_SourceCharacterBaseConstants[12];
    source[11].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    source[15]=1.f;
    source[16]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[3].xyzx, cb0[3].xyzx
    r0.w = (dot((source[3].xyzx).xyz,(source[3].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[3].xyzx, r0.wwww
    r1.xyz = ((source[3].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 11: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 12: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: mul r0.yz, v4.xxyx, cb0[4].xxyx
    r0.yz = ((v4.xxyx)*(source[4].xxyx)).yz;
    // 17: mad r0.yz, r0.yyzy, l(0.000000, -1.000000, 2.000000, 0.000000), cb0[6].xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,-1.000000,2.000000,0.000000))+(source[6].xxyx)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t2.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.zxyw, s1, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 20: mad r2.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mov r3.xw, l(1.000000,0,0,2.000000)
    r3.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 22: mov r3.yz, cb0[4].yyxy
    r3.yz = (source[4].yyxy).yz;
    // 23: mul r0.yz, r3.xxyx, v4.xxyx
    r0.yz = ((r3.xxyx)*(v4.xxyx)).yz;
    // 24: mad r3.xy, r3.zwzz, r0.yzyy, cb0[5].xyxx
    r3.xy = ((r3.zwzz)*(r0.yzyy)+(source[5].xyxx)).xy;
    // 25: mad r0.yz, r3.zzwz, r0.yyzy, cb0[9].xxyx
    r0.yz = ((r3.zzwz)*(r0.yyzy)+(source[9].xxyx)).yz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s2, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 29: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: mad r1.yzw, r4.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r1.xxyz
    r1.yzw = ((r4.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r1.xxyz)).yzw;
    // 31: mul r1.yzw, r1.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 32: mad_sat r0.yzw, r0.yyzw, r1.yyzw, r1.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r1.yyzw)+(r1.yyzw))).yzw;
    // 33: dp2 r1.y, r2.xyxx, r2.xyxx
    r1.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 34: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 36: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 37: add r2.z, r1.y, l(0.000010)
    r2.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: dp2 r1.y, r3.xyxx, r3.xyxx
    r1.y = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 39: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 41: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 42: add r3.z, r1.y, l(0.000010)
    r3.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: add r1.y, r2.z, -r3.z
    r1.y = ((r2.zzzz)+(-(r3.zzzz))).y;
    // 44: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 45: mad r1.x, r1.x, r1.y, r3.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r3.zzzz)).x;
    // 46: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 48: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 49: mul r1.xyz, r0.yzwy, r1.xxxx
    r1.xyz = ((r0.yzwy)*(r1.xxxx)).xyz;
    // 50: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 51: add r0.z, cb0[8].w, l(-0.030000)
    r0.z = ((source[8].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 52: mad r1.xyz, r1.xyzx, r0.zzzz, l(0.030000, 0.030000, 0.030000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r0.zzzz)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[8].xyzx
    r1.xyz = ((r1.xyzx)*(source[8].xyzx)).xyz;
    // 54: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 55: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 56: div r2.xyz, r2.xyzx, r0.zzzz
    r2.xyz = ((r2.xyzx)/(r0.zzzz)).xyz;
    // 57: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 58: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 59: mul r3.xyz, r0.zzzz, v5.xyzx
    r3.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 60: dp3 r0.z, r3.xyzx, r2.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 61: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: mad r0.z, -r0.z, l(0.500000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mul r2.xyz, r0.zzzz, cb0[7].xyzx
    r2.xyz = ((r0.zzzz)*(source[7].xyzx)).xyz;
    // 64: mul r2.xyz, r2.xyzx, cb0[7].wwww
    r2.xyz = ((r2.xyzx)*(source[7].wwww)).xyz;
    // 65: mad r0.xzw, r0.xxxx, r2.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)+(r1.xxyz)).xzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 67: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 68: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 69: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 70: add r1.w, -r1.w, l(0.200000)
    r1.w = ((-(r1.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 71: mul_sat r1.w, r1.w, l(5.000000)
    r1.w = (saturate((r1.wwww)*(float4(5.000000,5.000000,5.000000,5.000000)))).w;
    // 72: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 73: add r0.xzw, r0.xxzw, -r1.xxyz
    r0.xzw = ((r0.xxzw)+(-(r1.xxyz))).xzw;
    // 74: mad r0.xyz, r0.yyyy, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r1.xyzx)).xyz;
    // 75: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 76: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 77: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 80: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 81: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 82: mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // 83: mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // 84: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 85: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 86: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s3
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 87: mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // 88: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 89: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 90: mul r3.xyz, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((r3.xyzx)*(source[15].xyzx)).xyz;
    // 91: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 92: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 93: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 94: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 95: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 96: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 97: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 98: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 99: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 100: mad o0.xyz, r0.xyzx, cb0[14].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)+(r1.xyzx)).xyz;
    // 101: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 102: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 103: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 104: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 105: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 106: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 107: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 108: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 109: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 110: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 111: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 112: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 113: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 114: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 115: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 116: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 117: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 118: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 119: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 120: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 121: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 122: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 123: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 124: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 125: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 126: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 127: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 128: ret
    return output;
}

// source.character.static-map-native-1157.v1 / source program 581d40c52668a847a6bea5c07e85ffaa
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1157(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1157(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[6]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[10]=g_SourceCharacterBaseConstants[9];
    source[10].y=(g_SourceCharacterTime.xxxx).x;
    source[10].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[11]=g_SourceCharacterBaseConstants[12];
    source[11].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: add r0.xyz, v7.xyzx, cb0[0].xyzx
    r0.xyz = ((v7.xyzx)+(source[0].xyzx)).xyz;
    // 2: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 3: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 4: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 5: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 6: dp3 r0.w, cb0[3].xyzx, cb0[3].xyzx
    r0.w = (dot((source[3].xyzx).xyz,(source[3].xyzx).xyz).xxxx).w;
    // 7: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 8: div r1.xyz, cb0[3].xyzx, r0.wwww
    r1.xyz = ((source[3].xyzx)/(r0.wwww)).xyz;
    // 9: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 10: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 11: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 12: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 13: mul r0.y, r0.y, cb0[10].x
    r0.y = ((r0.yyyy)*(source[10].xxxx)).y;
    // 14: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: mul r0.yz, v4.xxyx, cb0[4].xxyx
    r0.yz = ((v4.xxyx)*(source[4].xxyx)).yz;
    // 17: mad r0.yz, r0.yyzy, l(0.000000, -1.000000, 2.000000, 0.000000), cb0[6].xxyx
    r0.yz = ((r0.yyzy)*(float4(0.000000,-1.000000,2.000000,0.000000))+(source[6].xxyx)).yz;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t2.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, r0.yzyy, t1.zxyw, s1, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 20: mad r2.xy, r0.yzyy, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r0.yzyy)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: mov r3.xw, l(1.000000,0,0,2.000000)
    r3.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 22: mov r3.yz, cb0[4].yyxy
    r3.yz = (source[4].yyxy).yz;
    // 23: mul r0.yz, r3.xxyx, v4.xxyx
    r0.yz = ((r3.xxyx)*(v4.xxyx)).yz;
    // 24: mad r3.xy, r3.zwzz, r0.yzyy, cb0[5].xyxx
    r3.xy = ((r3.zwzz)*(r0.yzyy)+(source[5].xyxx)).xy;
    // 25: mad r0.yz, r3.zzwz, r0.yyzy, cb0[9].xxyx
    r0.yz = ((r3.zzwz)*(r0.yyzy)+(source[9].xxyx)).yz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s2, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r3.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 29: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 30: mad r1.yzw, r4.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r1.xxyz
    r1.yzw = ((r4.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r1.xxyz)).yzw;
    // 31: mul r1.yzw, r1.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r1.yzw = ((r1.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 32: mad_sat r0.yzw, r0.yyzw, r1.yyzw, r1.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r1.yyzw)+(r1.yyzw))).yzw;
    // 33: dp2 r1.y, r2.xyxx, r2.xyxx
    r1.y = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 34: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 36: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 37: add r2.z, r1.y, l(0.000010)
    r2.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 38: dp2 r1.y, r3.xyxx, r3.xyxx
    r1.y = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 39: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: max r1.y, r1.y, l(0.000000)
    r1.y = (max(r1.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 41: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 42: add r3.z, r1.y, l(0.000010)
    r3.z = ((r1.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: add r1.y, r2.z, -r3.z
    r1.y = ((r2.zzzz)+(-(r3.zzzz))).y;
    // 44: add r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)+(r3.xyzx)).xyz;
    // 45: mad r1.x, r1.x, r1.y, r3.z
    r1.x = ((r1.xxxx)*(r1.yyyy)+(r3.zzzz)).x;
    // 46: add r1.x, r1.x, l(1.000000)
    r1.x = ((r1.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 47: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 48: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 49: mul r1.xyz, r0.yzwy, r1.xxxx
    r1.xyz = ((r0.yzwy)*(r1.xxxx)).xyz;
    // 50: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 51: add r0.z, cb0[8].w, l(-0.030000)
    r0.z = ((source[8].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 52: mad r1.xyz, r1.xyzx, r0.zzzz, l(0.030000, 0.030000, 0.030000, 0.000000)
    r1.xyz = ((r1.xyzx)*(r0.zzzz)+(float4(0.030000,0.030000,0.030000,0.000000))).xyz;
    // 53: mul r1.xyz, r1.xyzx, cb0[8].xyzx
    r1.xyz = ((r1.xyzx)*(source[8].xyzx)).xyz;
    // 54: dp3 r0.z, r2.xyzx, r2.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 55: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 56: div r2.xyz, r2.xyzx, r0.zzzz
    r2.xyz = ((r2.xyzx)/(r0.zzzz)).xyz;
    // 57: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 58: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 59: mul r3.xyz, r0.zzzz, v5.xyzx
    r3.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 60: dp3 r0.z, r3.xyzx, r2.xyzx
    r0.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 61: add r0.z, r0.z, l(1.000000)
    r0.z = ((r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 62: mad r0.z, -r0.z, l(0.500000), l(1.000000)
    r0.z = ((-(r0.zzzz))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 63: mul r2.xyz, r0.zzzz, cb0[7].xyzx
    r2.xyz = ((r0.zzzz)*(source[7].xyzx)).xyz;
    // 64: mul r2.xyz, r2.xyzx, cb0[7].wwww
    r2.xyz = ((r2.xyzx)*(source[7].wwww)).xyz;
    // 65: mad r0.xzw, r0.xxxx, r2.xxyz, r1.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)+(r1.xxyz)).xzw;
    // 66: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 67: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 68: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 69: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 70: add r1.w, -r1.w, l(0.200000)
    r1.w = ((-(r1.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 71: mul_sat r1.w, r1.w, l(5.000000)
    r1.w = (saturate((r1.wwww)*(float4(5.000000,5.000000,5.000000,5.000000)))).w;
    // 72: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 73: add r0.xzw, r0.xxzw, -r1.xxyz
    r0.xzw = ((r0.xxzw)+(-(r1.xxyz))).xzw;
    // 74: mad r0.xyz, r0.yyyy, r0.xzwx, r1.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r1.xyzx)).xyz;
    // 75: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 76: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 77: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 80: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 81: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 82: mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // 83: mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // 84: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 85: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 86: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 87: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 88: mad o0.xyz, r0.xyzx, cb0[14].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)+(r2.xyzx)).xyz;
    // 89: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 90: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 91: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 92: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 93: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 94: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 97: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 98: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 99: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 100: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 101: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 102: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 103: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 104: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 105: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 106: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 107: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 108: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 109: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 110: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 111: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 112: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 113: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 114: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 115: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 116: ret
    return output;
}

// source.character.static-map-native-1158.v1 / source program e25dc2835a47ae4ea177e6225abcb926
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1158(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[14].y=(g_SourceCharacterTime.xxxx).x;
    source[14].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15]=g_SourceCharacterBaseConstants[14];
    source[15].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[15].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[15].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[15].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=g_SourceCharacterBaseConstants[15];
    source[17]=g_SourceCharacterBaseConstants[16];
    source[18]=g_SourceCharacterBaseConstants[17];
    source[19]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[20]=g_SourceCharacterEnvironmentColor;source[21]=g_SourceCharacterEnvironmentRotation;}
    source[33]=1.f;
    source[34]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 4: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 5: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 6: mul r1.xy, v4.xyxx, cb0[3].xyxx
    r1.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 8: mul r2.xyz, cb0[4].xyzx, cb0[12].yyyy
    r2.xyz = ((source[4].xyzx)*(source[12].yyyy)).xyz;
    // 9: mul r3.xyz, r1.xyzx, r2.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 10: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 11: mad r1.xyz, -r1.xyzx, r2.xyzx, r0.wwww
    r1.xyz = ((-(r1.xyzx))*(r2.xyzx)+(r0.wwww)).xyz;
    // 12: mad r1.xyz, cb0[12].zzzz, r1.xyzx, r3.xyzx
    r1.xyz = ((source[12].zzzz)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 13: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 14: add r2.xyz, -r1.xyzx, r0.wwww
    r2.xyz = ((-(r1.xyzx))+(r0.wwww)).xyz;
    // 15: mad r1.xyz, cb0[12].wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((source[12].wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: mul r0.w, r2.z, cb0[16].w
    r0.w = ((r2.zzzz)*(source[16].wwww)).w;
    // 19: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 20: mul r2.xy, r2.xyxx, cb0[12].xxxx
    r2.xy = ((r2.xyxx)*(source[12].xxxx)).xy;
    // 21: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 22: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: add r2.z, r1.w, l(0.000010)
    r2.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 27: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 28: div r2.xyz, r2.xyzx, r1.wwww
    r2.xyz = ((r2.xyzx)/(r1.wwww)).xyz;
    // 29: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 30: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 31: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 32: dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 33: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: add r2.w, -|r3.z|, l(1.000000)
    r2.w = ((-(abs(r3.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 36: mad_sat r2.w, r1.w, cb0[13].x, -cb0[13].y
    r2.w = (saturate((r1.wwww)*(source[13].xxxx)+(-(source[13].yyyy)))).w;
    // 37: log r3.w, r2.w
    r3.w = (log2(r2.wwww)).w;
    // 38: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 39: mul r3.w, r3.w, cb0[13].z
    r3.w = ((r3.wwww)*(source[13].zzzz)).w;
    // 40: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 41: mul r4.xyz, r3.wwww, cb0[7].xyzx
    r4.xyz = ((r3.wwww)*(source[7].xyzx)).xyz;
    // 42: movc r4.xyz, r2.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 43: add r4.xyz, r4.xyzx, -cb0[7].xyzx
    r4.xyz = ((r4.xyzx)+(-(source[7].xyzx))).xyz;
    // 44: mad r4.xyz, cb0[7].wwww, r4.xyzx, cb0[7].xyzx
    r4.xyz = ((source[7].wwww)*(r4.xyzx)+(source[7].xyzx)).xyz;
    // 45: mul r5.xyz, cb0[8].xyzx, cb0[14].xxxx
    r5.xyz = ((source[8].xyzx)*(source[14].xxxx)).xyz;
    // 46: mul r5.xyz, r5.xyzx, cb0[15].wwww
    r5.xyz = ((r5.xyzx)*(source[15].wwww)).xyz;
    // 47: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 48: mul r5.xyz, r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 49: max r5.xyz, |r5.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r5.xyz = (max(abs(r5.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 50: log r5.xyz, r5.xyzx
    r5.xyz = (log2(r5.xyzx)).xyz;
    // 51: mul r5.xyz, r5.xyzx, cb0[16].xxxx
    r5.xyz = ((r5.xyzx)*(source[16].xxxx)).xyz;
    // 52: exp r5.xyz, r5.xyzx
    r5.xyz = (exp2(r5.xyzx)).xyz;
    // 53: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: add r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)+(r5.xyzx)).xyz;
    // 55: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 56: mad r5.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 57: mad r6.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 58: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 59: mad r1.xyz, r1.xyzx, r5.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 60: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 61: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 62: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 63: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 64: mul r4.xyz, r2.wwww, cb0[2].xyzx
    r4.xyz = ((r2.wwww)*(source[2].xyzx)).xyz;
    // 65: movc r4.xyz, r1.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 66: add r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)+(r4.xyzx)).xyz;
    // 67: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 68: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 69: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 70: mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 71: dp3 r1.w, r2.xyzx, r2.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 72: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 73: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 74: dp3 r1.w, r4.xyzx, r2.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 75: mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 76: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 77: mul r4.yzw, r4.yyyy, cb0[31].xxyz
    r4.yzw = ((r4.yyyy)*(source[31].xxyz)).yzw;
    // 78: mad r4.xyz, r4.xxxx, cb0[30].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[30].xyzx)+(r4.yzwy)).xyz;
    // 79: mul r4.xyz, r4.xyzx, cb0[32].wwww
    r4.xyz = ((r4.xyzx)*(source[32].wwww)).xyz;
    // 80: mul r5.xyz, cb0[10].xyzx, cb0[17].xxxx
    r5.xyz = ((source[10].xyzx)*(source[17].xxxx)).xyz;
    // 81: mul r5.xyz, r0.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 82: mul r6.xyz, cb0[11].xyzx, cb0[17].yyyy
    r6.xyz = ((source[11].xyzx)*(source[17].yyyy)).xyz;
    // 83: mad r0.xyz, r6.xyzx, r0.xyzx, -r5.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 85: mul r1.w, r6.z, cb0[17].z
    r1.w = ((r6.zzzz)*(source[17].zzzz)).w;
    // 86: mul r6.xy, r6.yxyy, cb0[19].xzxx
    r6.xy = ((r6.yxyy)*(source[19].xzxx)).xy;
    // 87: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 88: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 90: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 91: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 92: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 93: mul_sat r6.w, r1.w, cb2[3].w
    r6.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 94: mad r0.xyz, r2.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 95: mul r5.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r5.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 96: dp3 r1.w, r2.xyzx, r3.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 97: mul r7.xyz, r1.wwww, r2.xyzx
    r7.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 98: mad r3.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 99: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 100: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 101: mul r7.xyz, r3.wwww, v1.xyzx
    r7.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 102: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 103: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 104: mul r8.xyz, r3.wwww, v0.xyzx
    r8.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 105: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 106: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 107: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 108: dp3 r10.y, r9.xyzx, r3.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 109: dp3 r9.y, r9.xyzx, r2.xyzx
    r9.y = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 110: dp3 r10.x, r8.xyzx, r3.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 111: dp3 r9.x, r8.xyzx, r2.xyzx
    r9.x = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 112: mad r5.xy, cb0[16].yyyy, r10.xyxx, r5.xyxx
    r5.xy = ((source[16].yyyy)*(r10.xyxx)+(r5.xyxx)).xy;
    // 113: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 114: mul r5.xyz, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r5.xyzx)*(source[9].xyzx)).xyz;
    // 115: mad r5.xyz, cb0[16].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[16].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 116: add r5.xyz, r5.xyzx, -cb0[16].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[16].zzzz))).xyz;
    // 117: mov_sat r8.xyz, r5.xyzx
    r8.xyz = (saturate(r5.xyzx)).xyz;
    // 118: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 119: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 120: mad r0.xyz, r0.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 121: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 122: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 123: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 124: mul r5.xyz, r0.xyzx, cb0[18].xxxx
    r5.xyz = ((r0.xyzx)*(source[18].xxxx)).xyz;
    // 125: mad r0.xyz, cb0[18].yyyy, r0.xyzx, -r5.xyzx
    r0.xyz = ((source[18].yyyy)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 126: mad r0.xyz, r2.wwww, r0.xyzx, r5.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r5.xyzx)).xyz;
    // 127: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 128: mul r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 129: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 130: mul r5.xyz, r0.xyzx, r4.xyzx
    r5.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 131: dp2_sat r8.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 132: dp3_sat r8.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 133: dp3_sat r8.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 134: dp3 r2.y, r7.xyzx, r2.xyzx
    r2.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 135: dp3 r7.y, r7.xyzx, r3.xyzx
    r7.y = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 136: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 137: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 138: mul r11.xyz, r11.xyzx, cb0[34].xyzx
    r11.xyz = ((r11.xyzx)*(source[34].xyzx)).xyz;
    // 139: dp3 r3.w, r11.xyzx, r8.xyzx
    r3.w = (dot((r11.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 140: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 141: mul r8.xyz, r8.xyzx, cb0[33].xyzx
    r8.xyz = ((r8.xyzx)*(source[33].xyzx)).xyz;
    // 142: mul r12.xyz, r3.wwww, r8.xyzx
    r12.xyz = ((r3.wwww)*(r8.xyzx)).xyz;
    // 143: mad r5.xyz, r0.xyzx, r12.xyzx, r5.xyzx
    r5.xyz = ((r0.xyzx)*(r12.xyzx)+(r5.xyzx)).xyz;
    // 144: mad r12.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r12.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 145: mad r13.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 146: mad r14.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 147: log r10.zw, |r6.xxxy|
    r10.zw = (log2(abs(r6.xxxy))).zw;
    // 148: lt r6.xy, |r6.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r6.xy = (asfloat((uint4)((abs(r6.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 149: mul r10.zw, r10.zzzw, cb0[19].yyyw
    r10.zw = ((r10.zzzw)*(source[19].yyyw)).zw;
    // 150: exp r10.zw, r10.zzzw
    r10.zw = (exp2(r10.zzzw)).zw;
    // 151: min r4.w, r10.w, l(1.000000)
    r4.w = (min(r10.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: movc r5.w, r6.x, l(0), r10.z
    r5.w = ((asuint(r6.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r10.zzzz)).w;
    // 153: movc r4.w, r6.y, l(0), r4.w
    r4.w = ((asuint(r6.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 154: max r5.w, r5.w, cb0[0].w
    r5.w = (max(r5.wwww,source[0].wwww)).w;
    // 155: min r6.z, r5.w, l(1.000000)
    r6.z = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 156: mad r13.xyz, r4.wwww, r13.xyzx, r14.xyzx
    r13.xyz = ((r4.wwww)*(r13.xyzx)+(r14.xyzx)).xyz;
    // 157: mad r12.xyz, r13.xyzx, r4.wwww, r12.xyzx
    r12.xyz = ((r13.xyzx)*(r4.wwww)+(r12.xyzx)).xyz;
    // 158: mul r12.xyz, r4.wwww, r12.xyzx
    r12.xyz = ((r4.wwww)*(r12.xyzx)).xyz;
    // 159: max r12.xyz, r4.wwww, r12.xyzx
    r12.xyz = (max(r4.wwww,r12.xyzx)).xyz;
    // 160: mul r5.xyz, r5.xyzx, r12.xyzx
    r5.xyz = ((r5.xyzx)*(r12.xyzx)).xyz;
    // 161: dp2 r2.z, r9.xyxx, cb0[21].xyxx
    r2.z = (dot((r9.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 162: mul r6.xy, cb0[21].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((source[21].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 163: dp2 r2.x, r9.xyxx, r6.xyxx
    r2.x = (dot((r9.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 164: dp2 r7.x, r10.xyxx, r6.xyxx
    r7.x = (dot((r10.xyxx).xy,(r6.xyxx).xy).xxxx).x;
    // 165: dp2 r7.z, r10.xyxx, cb0[21].xyxx
    r7.z = (dot((r10.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 166: mov r2.w, l(1.000000)
    r2.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 167: dp4 r10.x, cb0[22].xyzw, r2.xyzw
    r10.x = (dot((source[22].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).x;
    // 168: dp4 r10.y, cb0[23].xyzw, r2.xyzw
    r10.y = (dot((source[23].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).y;
    // 169: dp4 r10.z, cb0[24].xyzw, r2.xyzw
    r10.z = (dot((source[24].xyzw).xyzw,(r2.xyzw).xyzw).xxxx).z;
    // 170: mul r12.xyzw, r2.yzzx, r2.xyzz
    r12.xyzw = ((r2.yzzx)*(r2.xyzz)).xyzw;
    // 171: dp4 r13.x, cb0[25].xyzw, r12.xyzw
    r13.x = (dot((source[25].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 172: dp4 r13.y, cb0[26].xyzw, r12.xyzw
    r13.y = (dot((source[26].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 173: dp4 r13.z, cb0[27].xyzw, r12.xyzw
    r13.z = (dot((source[27].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 174: add r10.xyz, r10.xyzx, r13.xyzx
    r10.xyz = ((r10.xyzx)+(r13.xyzx)).xyz;
    // 175: mul r2.z, r2.y, r2.y
    r2.z = ((r2.yyyy)*(r2.yyyy)).z;
    // 176: mov r9.z, r2.y
    r9.z = (r2.yyyy).z;
    // 177: mad r2.x, r2.x, r2.x, -r2.z
    r2.x = ((r2.xxxx)*(r2.xxxx)+(-(r2.zzzz))).x;
    // 178: mad r2.xyz, cb0[28].xyzx, r2.xxxx, r10.xyzx
    r2.xyz = ((source[28].xyzx)*(r2.xxxx)+(r10.xyzx)).xyz;
    // 179: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 180: mul r2.xyz, r2.xyzx, cb0[20].xyzx
    r2.xyz = ((r2.xyzx)*(source[20].xyzx)).xyz;
    // 181: mad r2.xyz, r2.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r2.xyz = ((r2.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 182: deriv_rtx_coarse r6.x, r1.w
    r6.x = (ddx_coarse(r1.wwww)).x;
    // 183: deriv_rty_coarse r6.y, r1.w
    r6.y = (ddy_coarse(r1.wwww)).y;
    // 184: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: dp2 r2.w, r6.xyxx, r6.xyxx
    r2.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 186: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 187: mad_sat r6.y, r2.w, l(0.300000), r6.z
    r6.y = (saturate((r2.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 188: add r2.w, -r6.y, l(1.000000)
    r2.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 189: mov_sat r0.w, cb0[18].z
    r0.w = (saturate(source[18].zzzz)).w;
    // 190: mad r10.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r10.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 191: mul r5.w, r0.w, l(0.080000)
    r5.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 192: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 193: mad r10.xyz, r6.wwww, r10.xyzx, r5.wwww
    r10.xyz = ((r6.wwww)*(r10.xyzx)+(r5.wwww)).xyz;
    // 194: max r12.xyz, r2.wwww, r10.xyzx
    r12.xyz = (max(r2.wwww,r10.xyzx)).xyz;
    // 195: add r12.xyz, -r10.xyzx, r12.xyzx
    r12.xyz = ((-(r10.xyzx))+(r12.xyzx)).xyz;
    // 196: mul_sat r0.w, r10.y, l(50.000000)
    r0.w = (saturate((r10.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 197: mul r12.xyz, r0.wwww, r12.xyzx
    r12.xyz = ((r0.wwww)*(r12.xyzx)).xyz;
    // 198: add r0.w, r3.z, l(1.000000)
    r0.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 199: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 200: add_sat r6.x, -r0.w, r1.w
    r6.x = (saturate((-(r0.wwww))+(r1.wwww))).x;
    // 201: sample_indexable(texture2d)(float,float,float,float) r13.xy, r6.xyxx, t5.xyzw, s7
    r13.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 202: add r0.w, r4.w, r6.x
    r0.w = ((r4.wwww)+(r6.xxxx)).w;
    // 203: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 204: mul r14.xyz, r10.xyzx, r13.yyyy
    r14.xyz = ((r10.xyzx)*(r13.yyyy)).xyz;
    // 205: mad r12.xyz, r12.xyzx, r13.xxxx, r14.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xxxx)+(r14.xyzx)).xyz;
    // 206: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.y
    r1.w = r13.y != 0.f ? 1.f / r13.y : 0.f;
    // 207: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 208: mad r13.xyz, r10.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r10.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 209: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: mad r10.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r10.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 211: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 212: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 213: mul r2.xyz, r2.xyzx, r14.xyzx
    r2.xyz = ((r2.xyzx)*(r14.xyzx)).xyz;
    // 214: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 215: mad r2.xyz, -r2.xyzx, r6.wwww, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(r6.wwww)+(r2.xyzx)).xyz;
    // 216: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 217: dp2_sat r5.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 218: dp3_sat r5.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 219: dp3_sat r5.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 220: mul r3.xyz, r5.xyzx, r5.xyzx
    r3.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 221: dp3 r1.w, r11.xyzx, r3.xyzx
    r1.w = (dot((r11.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 222: add r2.w, -r1.w, r3.w
    r2.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 223: mad r1.w, r6.z, r2.w, r1.w
    r1.w = ((r6.zzzz)*(r2.wwww)+(r1.wwww)).w;
    // 224: mad r3.xyz, r8.xyzx, r1.wwww, r4.xyzx
    r3.xyz = ((r8.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 225: mul r4.xyz, r1.wwww, r8.xyzx
    r4.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 226: mul r1.w, r6.y, r6.y
    r1.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 227: mul r2.w, r6.y, l(5.000000)
    r2.w = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 228: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r7.xyzx, t6.xyzw, s6, r2.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r7.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 229: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 230: mul r5.xyz, r5.xyzx, cb0[20].xyzx
    r5.xyz = ((r5.xyzx)*(source[20].xyzx)).xyz;
    // 231: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 232: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 233: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 234: add r0.w, r4.w, r0.w
    r0.w = ((r4.wwww)+(r0.wwww)).w;
    // 235: mov o5.y, r4.w
    output.targets[5].y = (r4.wwww).y;
    // 236: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 237: mad r1.w, r0.w, r10.x, r10.y
    r1.w = ((r0.wwww)*(r10.xxxx)+(r10.yyyy)).w;
    // 238: mad r1.w, r1.w, r0.w, r10.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r10.zzzz)).w;
    // 239: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 240: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 241: mul r6.xyz, r0.wwww, r3.xyzx
    r6.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 242: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 243: div r3.xyz, r4.xyzx, r3.xyzx
    r3.xyz = ((r4.xyzx)/(r3.xyzx)).xyz;
    // 244: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 245: mul r3.xyz, r5.xyzx, r6.xyzx
    r3.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 246: mad r2.xyz, r3.xyzx, r12.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r12.xyzx)+(r2.xyzx)).xyz;
    // 247: mul r3.xyz, r12.xyzx, r3.xyzx
    r3.xyz = ((r12.xyzx)*(r3.xyzx)).xyz;
    // 248: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 249: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 250: mad o0.xyz, r0.xyzx, cb0[32].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[32].xyzx)+(r1.xyzx)).xyz;
    // 251: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 252: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 253: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 254: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 255: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 256: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 257: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 258: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 259: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 260: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 261: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 262: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 263: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 264: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 265: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 266: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
    // 267: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 268: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 269: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 270: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 271: ret
    return output;
}

// source.character.static-map-native-1158.v1 / source program d1fd3eb3ee27444fa1c25343201ba381
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1158(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1158(input);
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
    source[14].y=(g_SourceCharacterTime.xxxx).x;
    source[14].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15]=g_SourceCharacterBaseConstants[14];
    source[15].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[15].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[15].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[15].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=g_SourceCharacterBaseConstants[15];
    source[17]=g_SourceCharacterBaseConstants[16];
    source[18]=g_SourceCharacterBaseConstants[17];
    source[19]=g_SourceCharacterBaseConstants[18];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[20]=g_SourceCharacterEnvironmentColor;source[21]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 8: mul r0.w, r1.z, cb0[16].w
    r0.w = ((r1.zzzz)*(source[16].wwww)).w;
    // 9: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 10: mul r1.xy, r1.xyxx, cb0[12].xxxx
    r1.xy = ((r1.xyxx)*(source[12].xxxx)).xy;
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
    // 33: dp2 r7.z, r6.xyxx, cb0[21].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 34: mul r8.xy, cb0[21].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[21].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 35: dp2 r7.x, r6.xyxx, r8.xyxx
    r7.x = (dot((r6.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 36: dp3 r7.y, r3.xyzx, r2.xyzx
    r7.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 37: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 38: dp4 r9.x, cb0[22].xyzw, r7.xyzw
    r9.x = (dot((source[22].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 39: dp4 r9.y, cb0[23].xyzw, r7.xyzw
    r9.y = (dot((source[23].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 40: dp4 r9.z, cb0[24].xyzw, r7.xyzw
    r9.z = (dot((source[24].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 41: mul r10.xyzw, r7.yzzx, r7.xyzz
    r10.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 42: dp4 r11.x, cb0[25].xyzw, r10.xyzw
    r11.x = (dot((source[25].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 43: dp4 r11.y, cb0[26].xyzw, r10.xyzw
    r11.y = (dot((source[26].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 44: dp4 r11.z, cb0[27].xyzw, r10.xyzw
    r11.z = (dot((source[27].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 45: add r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)+(r11.xyzx)).xyz;
    // 46: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 47: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 48: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 49: mad r7.xyz, cb0[28].xyzx, r1.wwww, r9.xyzx
    r7.xyz = ((source[28].xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 50: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: mul r7.xyz, r7.xyzx, cb0[20].xyzx
    r7.xyz = ((r7.xyzx)*(source[20].xyzx)).xyz;
    // 52: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 53: mul r9.xyz, cb0[10].xyzx, cb0[17].xxxx
    r9.xyz = ((source[10].xyzx)*(source[17].xxxx)).xyz;
    // 54: mul r9.xyz, r0.xyzx, r9.xyzx
    r9.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 55: mul r10.xyz, cb0[11].xyzx, cb0[17].yyyy
    r10.xyz = ((source[11].xyzx)*(source[17].yyyy)).xyz;
    // 56: mad r0.xyz, r10.xyzx, r0.xyzx, -r9.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)+(-(r9.xyzx))).xyz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 58: mul r1.w, r10.z, cb0[17].z
    r1.w = ((r10.zzzz)*(source[17].zzzz)).w;
    // 59: mul r8.zw, r10.yyyx, cb0[19].xxxz
    r8.zw = ((r10.yyyx)*(source[19].xxxz)).zw;
    // 60: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 61: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 62: mul r2.w, r2.w, cb0[17].w
    r2.w = ((r2.wwww)*(source[17].wwww)).w;
    // 63: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 64: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 65: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mul_sat r10.w, r1.w, cb2[3].w
    r10.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 67: mad r0.xyz, r2.wwww, r0.xyzx, r9.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r9.xyzx)).xyz;
    // 68: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 69: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 70: mul r9.xyz, r1.wwww, v5.xyzx
    r9.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 71: dp3 r1.w, r2.xyzx, r9.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 72: mul r11.xyz, r1.wwww, r2.xyzx
    r11.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 73: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 74: dp3 r5.y, r5.xyzx, r11.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 75: dp3 r5.x, r4.xyzx, r11.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 76: mul r4.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r4.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 77: mad r4.xy, cb0[16].yyyy, r5.xyxx, r4.xyxx
    r4.xy = ((source[16].yyyy)*(r5.xyxx)+(r4.xyxx)).xy;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: mul r4.xyz, r4.xyzx, cb0[9].xyzx
    r4.xyz = ((r4.xyzx)*(source[9].xyzx)).xyz;
    // 80: mad r4.xyz, cb0[16].zzzz, r4.xyzx, r4.xyzx
    r4.xyz = ((source[16].zzzz)*(r4.xyzx)+(r4.xyzx)).xyz;
    // 81: add r4.xyz, r4.xyzx, -cb0[16].zzzz
    r4.xyz = ((r4.xyzx)+(-(source[16].zzzz))).xyz;
    // 82: mov_sat r12.xyz, r4.xyzx
    r12.xyz = (saturate(r4.xyzx)).xyz;
    // 83: mov_sat r4.xyz, -r4.xyzx
    r4.xyz = (saturate(-(r4.xyzx))).xyz;
    // 84: mad r4.xyz, -r0.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mad r0.xyz, r0.wwww, r12.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r12.xyzx)+(r0.xyzx)).xyz;
    // 86: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 87: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 88: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 89: mul r4.xyz, r0.xyzx, cb0[18].xxxx
    r4.xyz = ((r0.xyzx)*(source[18].xxxx)).xyz;
    // 90: mad r0.xyz, cb0[18].yyyy, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[18].yyyy)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 91: mad r0.xyz, r2.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 92: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 93: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 94: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 95: mov_sat r0.w, cb0[18].z
    r0.w = (saturate(source[18].zzzz)).w;
    // 96: mad r4.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r4.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 97: mul r2.w, r0.w, l(0.080000)
    r2.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 98: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 99: mad r4.xyz, r10.wwww, r4.xyzx, r2.wwww
    r4.xyz = ((r10.wwww)*(r4.xyzx)+(r2.wwww)).xyz;
    // 100: deriv_rtx_coarse r10.x, r1.w
    r10.x = (ddx_coarse(r1.wwww)).x;
    // 101: deriv_rty_coarse r10.y, r1.w
    r10.y = (ddy_coarse(r1.wwww)).y;
    // 102: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 103: dp2 r1.w, r10.xyxx, r10.xyxx
    r1.w = (dot((r10.xyxx).xy,(r10.xyxx).xy).xxxx).w;
    // 104: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 105: log r5.zw, |r8.zzzw|
    r5.zw = (log2(abs(r8.zzzw))).zw;
    // 106: lt r8.zw, |r8.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r8.zw = (asfloat((uint4)((abs(r8.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 107: mul r5.zw, r5.zzzw, cb0[19].yyyw
    r5.zw = ((r5.zzzw)*(source[19].yyyw)).zw;
    // 108: exp r5.zw, r5.zzzw
    r5.zw = (exp2(r5.zzzw)).zw;
    // 109: movc r2.w, r8.z, l(0), r5.z
    r2.w = ((asuint(r8.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.zzzz)).w;
    // 110: min r3.w, r5.w, l(1.000000)
    r3.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: movc r3.w, r8.w, l(0), r3.w
    r3.w = ((asuint(r8.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 112: max r2.w, r2.w, cb0[0].w
    r2.w = (max(r2.wwww,source[0].wwww)).w;
    // 113: min r10.z, r2.w, l(1.000000)
    r10.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: mad_sat r10.y, r1.w, l(0.300000), r10.z
    r10.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r10.zzzz))).y;
    // 115: mov o2.zw, r10.zzzw
    output.targets[2].zw = (r10.zzzw).zw;
    // 116: add r1.w, -r10.y, l(1.000000)
    r1.w = ((-(r10.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: max r12.xyz, r4.xyzx, r1.wwww
    r12.xyz = (max(r4.xyzx,r1.wwww)).xyz;
    // 118: add r12.xyz, -r4.xyzx, r12.xyzx
    r12.xyz = ((-(r4.xyzx))+(r12.xyzx)).xyz;
    // 119: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 120: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 121: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add_sat r10.x, r0.w, -r1.w
    r10.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 124: sample_indexable(texture2d)(float,float,float,float) r5.zw, r10.xyxx, t5.zwxy, s6
    r5.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 125: add r0.w, r3.w, r10.x
    r0.w = ((r3.wwww)+(r10.xxxx)).w;
    // 126: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 127: mul r13.xyz, r4.xyzx, r5.wwww
    r13.xyz = ((r4.xyzx)*(r5.wwww)).xyz;
    // 128: mad r12.xyz, r12.xyzx, r5.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r5.zzzz)+(r13.xyzx)).xyz;
    // 129: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r5.w
    r1.w = r5.w != 0.f ? 1.f / r5.w : 0.f;
    // 130: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 131: mad r13.xyz, r4.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r4.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 132: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 133: mad r4.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 134: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 135: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 136: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 137: mad r13.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 138: mad r14.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 139: mad r15.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 140: mad r14.xyz, r3.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 141: mad r13.xyz, r14.xyzx, r3.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r3.wwww)+(r13.xyzx)).xyz;
    // 142: mul r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = ((r3.wwww)*(r13.xyzx)).xyz;
    // 143: max r13.xyz, r3.wwww, r13.xyzx
    r13.xyz = (max(r3.wwww,r13.xyzx)).xyz;
    // 144: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 145: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 146: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 147: dp3 r1.w, r14.xyzx, r2.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 148: dp3 r2.x, r14.xyzx, r11.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 149: dp3 r3.y, r3.xyzx, r11.xyzx
    r3.y = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 150: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 151: mad r2.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 152: mul r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)).xyzw;
    // 153: mul r11.xyz, r2.wwww, cb0[31].xyzx
    r11.xyz = ((r2.wwww)*(source[31].xyzx)).xyz;
    // 154: mad r11.xyz, r2.zzzz, cb0[30].xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(source[30].xyzx)+(r11.xyzx)).xyz;
    // 155: mul r11.xyz, r11.xyzx, cb0[32].wwww
    r11.xyz = ((r11.xyzx)*(source[32].wwww)).xyz;
    // 156: mul r11.xyz, r0.xyzx, r11.xyzx
    r11.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 157: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 158: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 159: mad r7.xyz, -r7.xyzx, r10.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r10.wwww)+(r7.xyzx)).xyz;
    // 160: dp2 r3.x, r5.xyxx, r8.xyxx
    r3.x = (dot((r5.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 161: dp2 r3.z, r5.xyxx, cb0[21].xyxx
    r3.z = (dot((r5.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 162: mul r1.w, r10.y, l(5.000000)
    r1.w = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 163: mul r2.z, r10.y, r10.y
    r2.z = ((r10.yyyy)*(r10.yyyy)).z;
    // 164: mul r0.w, r0.w, r2.z
    r0.w = ((r0.wwww)*(r2.zzzz)).w;
    // 165: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 166: add r0.w, r3.w, r0.w
    r0.w = ((r3.wwww)+(r0.wwww)).w;
    // 167: mov o5.y, r3.w
    output.targets[5].y = (r3.wwww).y;
    // 168: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 169: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t6.xyzw, s5, r1.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 170: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 171: mul r3.xyz, r3.xyzx, cb0[20].xyzx
    r3.xyz = ((r3.xyzx)*(source[20].xyzx)).xyz;
    // 172: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 173: mad r1.w, r0.w, r4.x, r4.y
    r1.w = ((r0.wwww)*(r4.xxxx)+(r4.yyyy)).w;
    // 174: mad r1.w, r1.w, r0.w, r4.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r4.zzzz)).w;
    // 175: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 176: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 177: mul r2.yzw, r2.yyyy, cb0[31].xxyz
    r2.yzw = ((r2.yyyy)*(source[31].xxyz)).yzw;
    // 178: mad r2.xyz, cb0[30].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[30].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 179: mul r2.xyz, r2.xyzx, cb0[32].wwww
    r2.xyz = ((r2.xyzx)*(source[32].wwww)).xyz;
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
    // 195: mad_sat r1.w, r0.w, cb0[13].x, -cb0[13].y
    r1.w = (saturate((r0.wwww)*(source[13].xxxx)+(-(source[13].yyyy)))).w;
    // 196: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 197: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 198: mul r2.x, r2.x, cb0[13].z
    r2.x = ((r2.xxxx)*(source[13].zzzz)).x;
    // 199: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 200: mul r2.xyz, r2.xxxx, cb0[7].xyzx
    r2.xyz = ((r2.xxxx)*(source[7].xyzx)).xyz;
    // 201: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 202: add r2.xyz, r2.xyzx, -cb0[7].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[7].xyzx))).xyz;
    // 203: mad r2.xyz, cb0[7].wwww, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((source[7].wwww)*(r2.xyzx)+(source[7].xyzx)).xyz;
    // 204: mul r4.xyz, cb0[8].xyzx, cb0[14].xxxx
    r4.xyz = ((source[8].xyzx)*(source[14].xxxx)).xyz;
    // 205: mul r4.xyz, r4.xyzx, cb0[15].wwww
    r4.xyz = ((r4.xyzx)*(source[15].wwww)).xyz;
    // 206: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 207: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 208: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 209: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 210: mul r4.xyz, r4.xyzx, cb0[16].xxxx
    r4.xyz = ((r4.xyzx)*(source[16].xxxx)).xyz;
    // 211: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 212: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 213: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 214: mul r2.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r2.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 215: mul r4.xy, v4.xyxx, cb0[3].xyxx
    r4.xy = ((v4.xyxx)*(source[3].xyxx)).xy;
    // 216: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t4.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 217: mul r5.xyz, cb0[4].xyzx, cb0[12].yyyy
    r5.xyz = ((source[4].xyzx)*(source[12].yyyy)).xyz;
    // 218: mul r7.xyz, r4.xyzx, r5.xyzx
    r7.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 219: dp3 r0.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 220: mad r4.xyz, -r4.xyzx, r5.xyzx, r0.wwww
    r4.xyz = ((-(r4.xyzx))*(r5.xyzx)+(r0.wwww)).xyz;
    // 221: mad r4.xyz, cb0[12].zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((source[12].zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 222: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 223: add r5.xyz, -r4.xyzx, r0.wwww
    r5.xyz = ((-(r4.xyzx))+(r0.wwww)).xyz;
    // 224: mad r4.xyz, cb0[12].wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((source[12].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 225: mad r5.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 226: mad r7.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 227: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 228: mad r2.xyz, r4.xyzx, r5.xyzx, r2.xyzx
    r2.xyz = ((r4.xyzx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 229: add r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)+(r2.xyzx)).xyz;
    // 230: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 231: add r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)+(r1.xyzx)).xyz;
    // 232: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 233: mad o0.xyz, r0.xyzx, cb0[32].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[32].xyzx)+(r1.xyzx)).xyz;
    // 234: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 235: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 236: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 237: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
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
    // 247: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
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

// source.character.static-map-native-1159.v1 / source program 8478e67365a6114f85a9e1f2857be843
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1159(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f;
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
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 14: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 16: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 20: mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // 21: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 22: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 23: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 24: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 25: mul r5.xy, v4.xyxx, cb0[5].yyyy
    r5.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t1.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 27: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 28: dp2 r0.w, r5.zwzz, r5.zwzz
    r0.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 29: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 31: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 32: add r6.z, r0.w, l(0.000010)
    r6.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: mul r6.xy, r5.zwzz, cb0[5].zzzz
    r6.xy = ((r5.zwzz)*(source[5].zzzz)).xy;
    // 34: max r0.w, cb0[5].w, l(0.000000)
    r0.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: min r0.w, r0.w, l(0.990000)
    r0.w = (min(r0.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 36: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 38: dp3 r7.x, r1.xyzx, r4.xyzx
    r7.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 39: dp3 r7.y, r2.xyzx, r4.xyzx
    r7.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 40: dp3 r7.z, r0.xyzx, r4.xyzx
    r7.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 41: max r8.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r8.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: min r8.xyz, r8.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: dp3 r2.w, r7.xyzx, r8.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 44: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r2.w, r2.w, l(0.500000), cb0[6].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r3.w, r4.z, r4.z
    r3.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 48: mul_sat r3.w, r3.w, r7.w
    r3.w = (saturate((r3.wwww)*(r7.wwww))).w;
    // 49: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r4.w, r5.w, r5.w
    r4.w = ((r5.wwww)*(r5.wwww)).w;
    // 52: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 53: mul r4.w, r0.w, r3.w
    r4.w = ((r0.wwww)*(r3.wwww)).w;
    // 54: mad r2.w, r2.w, r4.w, r2.w
    r2.w = ((r2.wwww)*(r4.wwww)+(r2.wwww)).w;
    // 55: add r0.w, -r0.w, r2.w
    r0.w = ((-(r0.wwww))+(r2.wwww)).w;
    // 56: mul r4.w, r0.w, r1.w
    r4.w = ((r0.wwww)*(r1.wwww)).w;
    // 57: mad r0.w, -r1.w, r0.w, r2.w
    r0.w = ((-(r1.wwww))*(r0.wwww)+(r2.wwww)).w;
    // 58: mad_sat r0.w, r3.w, r0.w, r4.w
    r0.w = (saturate((r3.wwww)*(r0.wwww)+(r4.wwww))).w;
    // 59: mul r1.w, r0.w, l(0.650000)
    r1.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 60: add r6.xyz, -r4.xyzx, r6.xyzx
    r6.xyz = ((-(r4.xyzx))+(r6.xyzx)).xyz;
    // 61: mad r4.xyz, r1.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 64: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 65: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 66: mul r6.xyz, r1.wwww, r4.xyzx
    r6.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 67: mad r3.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 68: add r2.w, -v2.w, cb0[9].w
    r2.w = ((-(v2.wwww))+(source[9].wwww)).w;
    // 69: add r2.w, r2.w, cb0[9].z
    r2.w = ((r2.wwww)+(source[9].zzzz)).w;
    // 70: add_sat r2.w, r2.w, l(1.000000)
    r2.w = (saturate((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 71: mul r3.w, r2.w, cb0[0].y
    r3.w = ((r2.wwww)*(source[0].yyyy)).w;
    // 72: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r4.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r8.xyz, -r7.xyzx, r4.wwww
    r8.xyz = ((-(r7.xyzx))+(r4.wwww)).xyz;
    // 75: mad r7.xyz, cb0[7].xxxx, r8.xyzx, r7.xyzx
    r7.xyz = ((source[7].xxxx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 76: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 77: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 78: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 79: mul r11.xyz, r5.xyzx, r10.xyzx
    r11.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 80: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: mad r5.xyz, -r10.xyzx, r5.xyzx, r4.wwww
    r5.xyz = ((-(r10.xyzx))*(r5.xyzx)+(r4.wwww)).xyz;
    // 82: mad r5.xyz, cb0[8].xxxx, r5.xyzx, r11.xyzx
    r5.xyz = ((source[8].xxxx)*(r5.xyzx)+(r11.xyzx)).xyz;
    // 83: mad r5.xyz, -r7.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r5.xyzx)).xyz;
    // 84: mad r5.xyz, r0.wwww, r5.xyzx, r9.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 85: mul r7.xyz, r5.xyzx, cb0[8].yyyy
    r7.xyz = ((r5.xyzx)*(source[8].yyyy)).xyz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 87: mul r0.w, r8.z, cb0[8].w
    r0.w = ((r8.zzzz)*(source[8].wwww)).w;
    // 88: lt r4.w, |r0.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 90: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 91: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 92: movc r0.w, r4.w, l(0), r0.w
    r0.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 93: min r4.w, r0.w, l(1.000000)
    r4.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mad r5.xyz, cb0[8].zzzz, r5.xyzx, -r7.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(-(r7.xyzx))).xyz;
    // 95: mad r5.xyz, r4.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 96: mul r5.xyz, r6.xyzx, r5.xyzx
    r5.xyz = ((r6.xyzx)*(r5.xyzx)).xyz;
    // 97: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 98: mul r6.xy, r8.yxyy, cb0[10].ywyy
    r6.xy = ((r8.yxyy)*(source[10].ywyy)).xy;
    // 99: lt r6.zw, |r6.xxxy|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r6.zw = (asfloat((uint4)((abs(r6.xxxy))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 100: log r6.xy, |r6.xyxx|
    r6.xy = (log2(abs(r6.xyxx))).xy;
    // 101: mul r4.w, r6.x, cb0[10].z
    r4.w = ((r6.xxxx)*(source[10].zzzz)).w;
    // 102: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 103: movc r4.w, r6.z, l(0), r4.w
    r4.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 104: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 105: min r7.z, r4.w, l(1.000000)
    r7.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 107: mov_sat r5.w, cb0[9].y
    r5.w = (saturate(source[9].yyyy)).w;
    // 108: mul r0.w, r6.y, cb0[11].x
    r0.w = ((r6.yyyy)*(source[11].xxxx)).w;
    // 109: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 110: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: movc r0.w, r6.w, l(0), r0.w
    r0.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 112: dp2_sat r6.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 113: dp3_sat r6.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 114: dp3_sat r6.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 115: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 116: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 117: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 118: mul r6.xyz, r6.xyzx, r6.xyzx
    r6.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 119: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 120: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s5
    r9.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 121: mul r9.xyz, r9.xyzx, cb0[25].xyzx
    r9.xyz = ((r9.xyzx)*(source[25].xyzx)).xyz;
    // 122: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s5
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 123: mul r10.xyz, r10.xyzx, cb0[26].xyzx
    r10.xyz = ((r10.xyzx)*(source[26].xyzx)).xyz;
    // 124: dp3 r4.w, r10.xyzx, r6.xyzx
    r4.w = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 125: mul r6.xyz, r4.wwww, r9.xyzx
    r6.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 126: dp3 r6.w, r10.xyzx, r8.xyzx
    r6.w = (dot((r10.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 127: add r4.w, r4.w, -r6.w
    r4.w = ((r4.wwww)+(-(r6.wwww))).w;
    // 128: mad r4.w, r7.z, r4.w, r6.w
    r4.w = ((r7.zzzz)*(r4.wwww)+(r6.wwww)).w;
    // 129: mul r8.xyz, r4.wwww, r9.xyzx
    r8.xyz = ((r4.wwww)*(r9.xyzx)).xyz;
    // 130: dp3 r6.w, v7.xyzx, v7.xyzx
    r6.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 131: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 132: mul r10.xyz, r6.wwww, v7.xyzx
    r10.xyz = ((r6.wwww)*(v7.xyzx)).xyz;
    // 133: dp3 r6.w, r10.xyzx, r4.xyzx
    r6.w = (dot((r10.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 134: mad r7.xy, r6.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r6.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 136: mul r10.xyz, r7.yyyy, cb0[23].xyzx
    r10.xyz = ((r7.yyyy)*(source[23].xyzx)).xyz;
    // 137: mad r10.xyz, r7.xxxx, cb0[22].xyzx, r10.xyzx
    r10.xyz = ((r7.xxxx)*(source[22].xyzx)+(r10.xyzx)).xyz;
    // 138: mul r10.xyz, r10.xyzx, cb0[24].wwww
    r10.xyz = ((r10.xyzx)*(source[24].wwww)).xyz;
    // 139: mul r11.xyz, r5.xyzx, r10.xyzx
    r11.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 140: mad r6.xyz, r5.xyzx, r6.xyzx, r11.xyzx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 141: mad r9.xyz, r9.xyzx, r4.wwww, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r4.wwww)+(r10.xyzx)).xyz;
    // 142: mul r4.w, r5.w, l(0.080000)
    r4.w = ((r5.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 143: mad r10.xyz, -r5.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r10.xyz = ((-(r5.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 144: mad r10.xyz, r7.wwww, r10.xyzx, r4.wwww
    r10.xyz = ((r7.wwww)*(r10.xyzx)+(r4.wwww)).xyz;
    // 145: deriv_rtx_coarse r7.x, r1.w
    r7.x = (ddx_coarse(r1.wwww)).x;
    // 146: deriv_rty_coarse r7.y, r1.w
    r7.y = (ddy_coarse(r1.wwww)).y;
    // 147: dp2 r4.w, r7.xyxx, r7.xyxx
    r4.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 148: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 149: mad_sat r7.y, r4.w, l(0.300000), r7.z
    r7.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 150: add r4.w, r3.z, l(1.000000)
    r4.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: add_sat r7.x, -r4.w, r1.w
    r7.x = (saturate((-(r4.wwww))+(r1.wwww))).x;
    // 154: sample_indexable(texture2d)(float,float,float,float) r11.xy, r7.xyxx, t5.xyzw, s7
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 155: add r1.w, -r7.y, l(1.000000)
    r1.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: max r12.xyz, r10.xyzx, r1.wwww
    r12.xyz = (max(r10.xyzx,r1.wwww)).xyz;
    // 157: add r12.xyz, -r10.xyzx, r12.xyzx
    r12.xyz = ((-(r10.xyzx))+(r12.xyzx)).xyz;
    // 158: mul_sat r1.w, r10.y, l(50.000000)
    r1.w = (saturate((r10.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 159: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 160: mul r13.xyz, r10.xyzx, r11.yyyy
    r13.xyz = ((r10.xyzx)*(r11.yyyy)).xyz;
    // 161: mad r11.xzw, r12.xxyz, r11.xxxx, r13.xxyz
    r11.xzw = ((r12.xxyz)*(r11.xxxx)+(r13.xxyz)).xzw;
    // 162: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r1.w = r11.y != 0.f ? 1.f / r11.y : 0.f;
    // 163: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 164: mad r12.xyz, r10.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((r10.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 165: mul r13.xyz, r11.xzwx, r12.xyzx
    r13.xyz = ((r11.xzwx)*(r12.xyzx)).xyz;
    // 166: dp3 r14.x, r1.xyzx, r3.xyzx
    r14.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 167: dp3 r14.y, r2.xyzx, r3.xyzx
    r14.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 168: dp3 r3.y, r0.xyzx, r3.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 169: mul r1.w, r7.y, l(5.000000)
    r1.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 170: mul r14.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r14.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 171: dp2 r3.x, r14.xyxx, r14.zwzz
    r3.x = (dot((r14.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 172: dp2 r3.z, r14.xyxx, cb0[13].xyxx
    r3.z = (dot((r14.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 173: sample_l_indexable(texturecube)(float,float,float,float) r15.xyzw, r3.xyzx, t6.xyzw, s6, r1.w
    r15.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 174: mul r3.xyz, r15.xyzx, r15.wwww
    r3.xyz = ((r15.xyzx)*(r15.wwww)).xyz;
    // 175: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 176: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 177: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 178: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 179: dp3 r4.y, r0.xyzx, r4.xyzx
    r4.y = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 180: dp2 r4.x, r1.xyxx, r14.zwzz
    r4.x = (dot((r1.xyxx).xy,(r14.zwzz).xy).xxxx).x;
    // 181: dp2 r4.z, r1.xyxx, cb0[13].xyxx
    r4.z = (dot((r1.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 182: mov r4.w, l(1.000000)
    r4.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 183: dp4 r0.x, cb0[14].xyzw, r4.xyzw
    r0.x = (dot((source[14].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).x;
    // 184: dp4 r0.y, cb0[15].xyzw, r4.xyzw
    r0.y = (dot((source[15].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).y;
    // 185: dp4 r0.z, cb0[16].xyzw, r4.xyzw
    r0.z = (dot((source[16].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).z;
    // 186: mul r14.xyzw, r4.yzzx, r4.xyzz
    r14.xyzw = ((r4.yzzx)*(r4.xyzz)).xyzw;
    // 187: dp4 r2.x, cb0[17].xyzw, r14.xyzw
    r2.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 188: dp4 r2.y, cb0[18].xyzw, r14.xyzw
    r2.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 189: dp4 r2.z, cb0[19].xyzw, r14.xyzw
    r2.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 190: mul r1.w, r4.y, r4.y
    r1.w = ((r4.yyyy)*(r4.yyyy)).w;
    // 191: mad r1.w, r4.x, r4.x, -r1.w
    r1.w = ((r4.xxxx)*(r4.xxxx)+(-(r1.wwww))).w;
    // 192: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 193: mad r0.xyz, cb0[20].xyzx, r1.wwww, r0.xyzx
    r0.xyz = ((source[20].xyzx)*(r1.wwww)+(r0.xyzx)).xyz;
    // 194: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 195: mul r0.xyz, r0.xyzx, cb0[12].xyzx
    r0.xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 196: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 197: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 198: dp3 r2.x, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 199: add r2.y, r0.w, r7.x
    r2.y = ((r0.wwww)+(r7.xxxx)).y;
    // 200: log r2.y, r2.y
    r2.y = (log2(r2.yyyy)).y;
    // 201: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 202: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 203: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 204: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 205: mad r2.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r2.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 206: mad r2.x, r1.w, r2.x, r2.y
    r2.x = ((r1.wwww)*(r2.xxxx)+(r2.yyyy)).x;
    // 207: mad r2.x, r2.x, r1.w, r2.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r2.zzzz)).x;
    // 208: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 209: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 210: mad r2.xyz, r5.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r5.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 211: mad r4.xzw, r5.xxyz, l(-4.795100, 0.000000, -4.795100, -4.795100), l(0.641700, 0.000000, 0.641700, 0.641700)
    r4.xzw = ((r5.xxyz)*(float4(-4.795100,0.000000,-4.795100,-4.795100))+(float4(0.641700,0.000000,0.641700,0.641700))).xzw;
    // 212: mad r10.xyz, r5.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r5.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 213: mad r2.xyz, r0.wwww, r2.xyzx, r4.xzwx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r4.xzwx)).xyz;
    // 214: mad r2.xyz, r2.xyzx, r0.wwww, r10.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r10.xyzx)).xyz;
    // 215: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 216: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 217: mul r4.xzw, r1.wwww, r9.xxyz
    r4.xzw = ((r1.wwww)*(r9.xxyz)).xzw;
    // 218: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 219: mul r3.xyz, r3.xyzx, r4.xzwx
    r3.xyz = ((r3.xyzx)*(r4.xzwx)).xyz;
    // 220: mul r4.xzw, r13.xxyz, r3.xxyz
    r4.xzw = ((r13.xxyz)*(r3.xxyz)).xzw;
    // 221: mad r6.xyz, -r11.xzwx, r12.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r11.xzwx))*(r12.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 222: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 223: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 224: mad r0.xyz, -r0.xyzx, r7.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r7.wwww)+(r0.xyzx)).xyz;
    // 225: mad r0.xyz, r3.xyzx, r13.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r13.xyzx)+(r0.xyzx)).xyz;
    // 226: add r2.xyz, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 227: mad r2.xyz, r5.xyzx, cb0[24].xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(source[24].xyzx)+(r2.xyzx)).xyz;
    // 228: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 229: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 230: eq r3.x, cb0[27].x, l(0.000000)
    r3.x = (asfloat((uint4)((source[27].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 231: not r3.y, r3.x
    r3.y = (asfloat(~asuint(r3.xxxx))).y;
    // 232: lt r3.z, r3.w, r1.w
    r3.z = (asfloat((uint4)((r3.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 233: and r3.y, r3.z, r3.y
    r3.y = (asfloat(asuint(r3.zzzz) & asuint(r3.yyyy))).y;
    // 234: discard_nz r3.y
    if ((asuint(r3.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 235: ge r1.w, r3.w, r1.w
    r1.w = (asfloat((uint4)((r3.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 236: mad r2.w, r2.w, cb0[0].y, l(-0.900000)
    r2.w = ((r2.wwww)*(source[0].yyyy)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 237: mul_sat r2.w, r2.w, l(9.999998)
    r2.w = (saturate((r2.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 238: mad r3.y, r2.w, l(-2.000000), l(3.000000)
    r3.y = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 239: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 240: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 241: mul r2.w, r3.w, r2.w
    r2.w = ((r3.wwww)*(r2.wwww)).w;
    // 242: movc r1.w, r1.w, r2.w, r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r2.wwww) : (r3.wwww)).w;
    // 243: movc o0.w, r3.x, r1.w, r3.w
    output.targets[0].w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r3.wwww)).w;
    // 244: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 245: mov r1.z, r4.y
    r1.z = (r4.yyyy).z;
    // 246: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 247: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 248: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 249: dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // 250: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 251: ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // 252: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 253: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 254: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 255: movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 256: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 257: dp3 o4.x, r4.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 258: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 259: add r1.xyz, r9.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r9.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 260: div r1.xyz, r8.xyzx, r1.xyzx
    r1.xyz = ((r8.xyzx)/(r1.xyzx)).xyz;
    // 261: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 262: mul o4.z, r0.y, r0.x
    output.targets[4].z = ((r0.yyyy)*(r0.xxxx)).z;
    // 263: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 264: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 265: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 266: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 267: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 268: mov o3.xyzw, r5.xyzw
    output.targets[3].xyzw = (r5.xyzw).xyzw;
    // 269: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 270: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 271: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 272: ret
    return output;
}

// source.character.static-map-native-1159.v1 / source program 195bcda7526b914d8519ed44ab2365bb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1159(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1159(input);
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
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
    // 10: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 14: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 16: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 19: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 20: mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // 21: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 22: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 23: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 24: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 25: mul r5.xy, v4.xyxx, cb0[5].yyyy
    r5.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t1.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 27: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 28: dp2 r0.w, r5.zwzz, r5.zwzz
    r0.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 29: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 31: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 32: add r6.z, r0.w, l(0.000010)
    r6.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: mul r6.xy, r5.zwzz, cb0[5].zzzz
    r6.xy = ((r5.zwzz)*(source[5].zzzz)).xy;
    // 34: max r0.w, cb0[5].w, l(0.000000)
    r0.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: min r0.w, r0.w, l(0.990000)
    r0.w = (min(r0.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 36: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 38: dp3 r7.x, r1.xyzx, r4.xyzx
    r7.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 39: dp3 r7.y, r2.xyzx, r4.xyzx
    r7.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 40: dp3 r7.z, r0.xyzx, r4.xyzx
    r7.z = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 41: max r8.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r8.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 42: min r8.xyz, r8.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = (min(r8.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 43: dp3 r2.w, r7.xyzx, r8.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 44: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r2.w, r2.w, l(0.500000), cb0[6].z
    r2.w = ((r2.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r3.w, r4.z, r4.z
    r3.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 48: mul_sat r3.w, r3.w, r7.w
    r3.w = (saturate((r3.wwww)*(r7.wwww))).w;
    // 49: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r4.w, r5.w, r5.w
    r4.w = ((r5.wwww)*(r5.wwww)).w;
    // 52: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 53: mul r4.w, r0.w, r3.w
    r4.w = ((r0.wwww)*(r3.wwww)).w;
    // 54: mad r2.w, r2.w, r4.w, r2.w
    r2.w = ((r2.wwww)*(r4.wwww)+(r2.wwww)).w;
    // 55: add r0.w, -r0.w, r2.w
    r0.w = ((-(r0.wwww))+(r2.wwww)).w;
    // 56: mul r4.w, r0.w, r1.w
    r4.w = ((r0.wwww)*(r1.wwww)).w;
    // 57: mad r0.w, -r1.w, r0.w, r2.w
    r0.w = ((-(r1.wwww))*(r0.wwww)+(r2.wwww)).w;
    // 58: mad_sat r0.w, r3.w, r0.w, r4.w
    r0.w = (saturate((r3.wwww)*(r0.wwww)+(r4.wwww))).w;
    // 59: mul r1.w, r0.w, l(0.650000)
    r1.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 60: add r6.xyz, -r4.xyzx, r6.xyzx
    r6.xyz = ((-(r4.xyzx))+(r6.xyzx)).xyz;
    // 61: mad r4.xyz, r1.wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((r1.wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 64: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 65: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 66: mul r6.xyz, r1.wwww, r4.xyzx
    r6.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 67: mad r3.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 68: add r2.w, -v2.w, cb0[9].w
    r2.w = ((-(v2.wwww))+(source[9].wwww)).w;
    // 69: add r2.w, r2.w, cb0[9].z
    r2.w = ((r2.wwww)+(source[9].zzzz)).w;
    // 70: add_sat r2.w, r2.w, l(1.000000)
    r2.w = (saturate((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 71: mul r3.w, r2.w, cb0[0].y
    r3.w = ((r2.wwww)*(source[0].yyyy)).w;
    // 72: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 73: dp3 r4.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: add r8.xyz, -r7.xyzx, r4.wwww
    r8.xyz = ((-(r7.xyzx))+(r4.wwww)).xyz;
    // 75: mad r7.xyz, cb0[7].xxxx, r8.xyzx, r7.xyzx
    r7.xyz = ((source[7].xxxx)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 76: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 77: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 78: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 79: mul r11.xyz, r5.xyzx, r10.xyzx
    r11.xyz = ((r5.xyzx)*(r10.xyzx)).xyz;
    // 80: dp3 r4.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: mad r5.xyz, -r10.xyzx, r5.xyzx, r4.wwww
    r5.xyz = ((-(r10.xyzx))*(r5.xyzx)+(r4.wwww)).xyz;
    // 82: mad r5.xyz, cb0[8].xxxx, r5.xyzx, r11.xyzx
    r5.xyz = ((source[8].xxxx)*(r5.xyzx)+(r11.xyzx)).xyz;
    // 83: mad r5.xyz, -r7.xyzx, r8.xyzx, r5.xyzx
    r5.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r5.xyzx)).xyz;
    // 84: mad r5.xyz, r0.wwww, r5.xyzx, r9.xyzx
    r5.xyz = ((r0.wwww)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 85: mul r7.xyz, r5.xyzx, cb0[8].yyyy
    r7.xyz = ((r5.xyzx)*(source[8].yyyy)).xyz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r8.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r8.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 87: mul r0.w, r8.z, cb0[8].w
    r0.w = ((r8.zzzz)*(source[8].wwww)).w;
    // 88: lt r4.w, |r0.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 89: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 90: mul r0.w, r0.w, cb0[9].x
    r0.w = ((r0.wwww)*(source[9].xxxx)).w;
    // 91: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 92: movc r0.w, r4.w, l(0), r0.w
    r0.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 93: min r4.w, r0.w, l(1.000000)
    r4.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 94: mad r5.xyz, cb0[8].zzzz, r5.xyzx, -r7.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(-(r7.xyzx))).xyz;
    // 95: mad r5.xyz, r4.wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 96: mul r5.xyz, r6.xyzx, r5.xyzx
    r5.xyz = ((r6.xyzx)*(r5.xyzx)).xyz;
    // 97: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 98: mul r6.xy, r8.yxyy, cb0[10].ywyy
    r6.xy = ((r8.yxyy)*(source[10].ywyy)).xy;
    // 99: lt r6.zw, |r6.xxxy|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r6.zw = (asfloat((uint4)((abs(r6.xxxy))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 100: log r6.xy, |r6.xyxx|
    r6.xy = (log2(abs(r6.xyxx))).xy;
    // 101: mul r4.w, r6.x, cb0[10].z
    r4.w = ((r6.xxxx)*(source[10].zzzz)).w;
    // 102: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 103: movc r4.w, r6.z, l(0), r4.w
    r4.w = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 104: max r4.w, r4.w, cb0[0].x
    r4.w = (max(r4.wwww,source[0].xxxx)).w;
    // 105: min r7.z, r4.w, l(1.000000)
    r7.z = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 106: mul_sat r7.w, r0.w, cb2[3].w
    r7.w = (saturate((r0.wwww)*(passValues[3].wwww))).w;
    // 107: mov_sat r5.w, cb0[9].y
    r5.w = (saturate(source[9].yyyy)).w;
    // 108: mul r0.w, r6.y, cb0[11].x
    r0.w = ((r6.yyyy)*(source[11].xxxx)).w;
    // 109: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 110: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: movc r0.w, r6.w, l(0), r0.w
    r0.w = ((asuint(r6.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 112: dp3 r4.w, v7.xyzx, v7.xyzx
    r4.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 113: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 114: mul r6.xyz, r4.wwww, v7.xyzx
    r6.xyz = ((r4.wwww)*(v7.xyzx)).xyz;
    // 115: dp3 r4.w, r6.xyzx, r4.xyzx
    r4.w = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 116: mad r7.xy, r4.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r4.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 117: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 118: mul r8.xyz, r7.yyyy, cb0[23].xyzx
    r8.xyz = ((r7.yyyy)*(source[23].xyzx)).xyz;
    // 119: mad r8.xyz, r7.xxxx, cb0[22].xyzx, r8.xyzx
    r8.xyz = ((r7.xxxx)*(source[22].xyzx)+(r8.xyzx)).xyz;
    // 120: mul r8.xyz, r8.xyzx, cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(source[24].wwww)).xyz;
    // 121: mul r8.xyz, r5.xyzx, r8.xyzx
    r8.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 122: dp3 r4.w, r6.xyzx, r3.xyzx
    r4.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 123: mad r6.xy, r4.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r4.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 124: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 125: mul r6.yzw, r6.yyyy, cb0[23].xxyz
    r6.yzw = ((r6.yyyy)*(source[23].xxyz)).yzw;
    // 126: mad r6.xyz, cb0[22].xyzx, r6.xxxx, r6.yzwy
    r6.xyz = ((source[22].xyzx)*(r6.xxxx)+(r6.yzwy)).xyz;
    // 127: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 128: mul r4.w, r5.w, l(0.080000)
    r4.w = ((r5.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 129: mad r9.xyz, -r5.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r9.xyz = ((-(r5.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 130: mad r9.xyz, r7.wwww, r9.xyzx, r4.wwww
    r9.xyz = ((r7.wwww)*(r9.xyzx)+(r4.wwww)).xyz;
    // 131: deriv_rtx_coarse r7.x, r1.w
    r7.x = (ddx_coarse(r1.wwww)).x;
    // 132: deriv_rty_coarse r7.y, r1.w
    r7.y = (ddy_coarse(r1.wwww)).y;
    // 133: dp2 r4.w, r7.xyxx, r7.xyxx
    r4.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 134: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 135: mad_sat r7.y, r4.w, l(0.300000), r7.z
    r7.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 136: add r4.w, r3.z, l(1.000000)
    r4.w = ((r3.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: min r4.w, r4.w, l(1.000000)
    r4.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 138: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: add_sat r7.x, -r4.w, r1.w
    r7.x = (saturate((-(r4.wwww))+(r1.wwww))).x;
    // 140: sample_indexable(texture2d)(float,float,float,float) r10.xy, r7.xyxx, t5.xyzw, s6
    r10.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 141: add r1.w, -r7.y, l(1.000000)
    r1.w = ((-(r7.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: max r11.xyz, r9.xyzx, r1.wwww
    r11.xyz = (max(r9.xyzx,r1.wwww)).xyz;
    // 143: add r11.xyz, -r9.xyzx, r11.xyzx
    r11.xyz = ((-(r9.xyzx))+(r11.xyzx)).xyz;
    // 144: mul_sat r1.w, r9.y, l(50.000000)
    r1.w = (saturate((r9.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 145: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 146: mul r12.xyz, r9.xyzx, r10.yyyy
    r12.xyz = ((r9.xyzx)*(r10.yyyy)).xyz;
    // 147: mad r10.xzw, r11.xxyz, r10.xxxx, r12.xxyz
    r10.xzw = ((r11.xxyz)*(r10.xxxx)+(r12.xxyz)).xzw;
    // 148: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r10.y
    r1.w = r10.y != 0.f ? 1.f / r10.y : 0.f;
    // 149: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 150: mad r11.xyz, r9.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r9.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r12.xyz, r10.xzwx, r11.xyzx
    r12.xyz = ((r10.xzwx)*(r11.xyzx)).xyz;
    // 152: dp3 r13.x, r1.xyzx, r3.xyzx
    r13.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 153: dp3 r13.y, r2.xyzx, r3.xyzx
    r13.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 154: dp3 r3.y, r0.xyzx, r3.xyzx
    r3.y = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 155: mul r1.w, r7.y, l(5.000000)
    r1.w = ((r7.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 156: mul r13.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r13.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 157: dp2 r3.x, r13.xyxx, r13.zwzz
    r3.x = (dot((r13.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 158: dp2 r3.z, r13.xyxx, cb0[13].xyxx
    r3.z = (dot((r13.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 159: sample_l_indexable(texturecube)(float,float,float,float) r14.xyzw, r3.xyzx, t6.xyzw, s5, r1.w
    r14.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 160: mul r3.xyz, r14.xyzx, r14.wwww
    r3.xyz = ((r14.xyzx)*(r14.wwww)).xyz;
    // 161: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 162: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 163: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 164: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 165: dp3 r4.y, r0.xyzx, r4.xyzx
    r4.y = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 166: dp2 r4.x, r1.xyxx, r13.zwzz
    r4.x = (dot((r1.xyxx).xy,(r13.zwzz).xy).xxxx).x;
    // 167: dp2 r4.z, r1.xyxx, cb0[13].xyxx
    r4.z = (dot((r1.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 168: mov r4.w, l(1.000000)
    r4.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 169: dp4 r0.x, cb0[14].xyzw, r4.xyzw
    r0.x = (dot((source[14].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).x;
    // 170: dp4 r0.y, cb0[15].xyzw, r4.xyzw
    r0.y = (dot((source[15].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).y;
    // 171: dp4 r0.z, cb0[16].xyzw, r4.xyzw
    r0.z = (dot((source[16].xyzw).xyzw,(r4.xyzw).xyzw).xxxx).z;
    // 172: mul r13.xyzw, r4.yzzx, r4.xyzz
    r13.xyzw = ((r4.yzzx)*(r4.xyzz)).xyzw;
    // 173: dp4 r2.x, cb0[17].xyzw, r13.xyzw
    r2.x = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 174: dp4 r2.y, cb0[18].xyzw, r13.xyzw
    r2.y = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 175: dp4 r2.z, cb0[19].xyzw, r13.xyzw
    r2.z = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 176: mul r1.w, r4.y, r4.y
    r1.w = ((r4.yyyy)*(r4.yyyy)).w;
    // 177: mad r1.w, r4.x, r4.x, -r1.w
    r1.w = ((r4.xxxx)*(r4.xxxx)+(-(r1.wwww))).w;
    // 178: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 179: mad r0.xyz, cb0[20].xyzx, r1.wwww, r0.xyzx
    r0.xyz = ((source[20].xyzx)*(r1.wwww)+(r0.xyzx)).xyz;
    // 180: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 181: mul r0.xyz, r0.xyzx, cb0[12].xyzx
    r0.xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 182: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 183: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 184: dp3 r2.x, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.x = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 185: add r2.y, r0.w, r7.x
    r2.y = ((r0.wwww)+(r7.xxxx)).y;
    // 186: log r2.y, r2.y
    r2.y = (log2(r2.yyyy)).y;
    // 187: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 188: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 189: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 190: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 191: mad r2.xyz, r2.xxxx, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r2.xyz = ((r2.xxxx)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 192: mad r2.x, r1.w, r2.x, r2.y
    r2.x = ((r1.wwww)*(r2.xxxx)+(r2.yyyy)).x;
    // 193: mad r2.x, r2.x, r1.w, r2.z
    r2.x = ((r2.xxxx)*(r1.wwww)+(r2.zzzz)).x;
    // 194: mul r2.x, r1.w, r2.x
    r2.x = ((r1.wwww)*(r2.xxxx)).x;
    // 195: max r1.w, r1.w, r2.x
    r1.w = (max(r1.wwww,r2.xxxx)).w;
    // 196: mad r2.xyz, r5.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r2.xyz = ((r5.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 197: mad r4.xzw, r5.xxyz, l(-4.795100, 0.000000, -4.795100, -4.795100), l(0.641700, 0.000000, 0.641700, 0.641700)
    r4.xzw = ((r5.xxyz)*(float4(-4.795100,0.000000,-4.795100,-4.795100))+(float4(0.641700,0.000000,0.641700,0.641700))).xzw;
    // 198: mad r9.xyz, r5.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r5.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 199: mad r2.xyz, r0.wwww, r2.xyzx, r4.xzwx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r4.xzwx)).xyz;
    // 200: mad r2.xyz, r2.xyzx, r0.wwww, r9.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 201: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 202: max r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = (max(r0.wwww,r2.xyzx)).xyz;
    // 203: mul r4.xzw, r1.wwww, r6.xxyz
    r4.xzw = ((r1.wwww)*(r6.xxyz)).xzw;
    // 204: mul r2.xyz, r2.xyzx, r8.xyzx
    r2.xyz = ((r2.xyzx)*(r8.xyzx)).xyz;
    // 205: mul r3.xyz, r3.xyzx, r4.xzwx
    r3.xyz = ((r3.xyzx)*(r4.xzwx)).xyz;
    // 206: mul r4.xzw, r12.xxyz, r3.xxyz
    r4.xzw = ((r12.xxyz)*(r3.xxyz)).xzw;
    // 207: mad r6.xyz, -r10.xzwx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r10.xzwx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 208: mul r0.xyz, r0.xyzx, r6.xyzx
    r0.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 209: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 210: mad r0.xyz, -r0.xyzx, r7.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r7.wwww)+(r0.xyzx)).xyz;
    // 211: mad r0.xyz, r3.xyzx, r12.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r12.xyzx)+(r0.xyzx)).xyz;
    // 212: add r2.xyz, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 213: mad r2.xyz, r5.xyzx, cb0[24].xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(source[24].xyzx)+(r2.xyzx)).xyz;
    // 214: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 215: mad r1.w, r1.w, l(-0.250000), l(0.400000)
    r1.w = ((r1.wwww)*(float4(-0.250000,-0.250000,-0.250000,-0.250000))+(float4(0.400000,0.400000,0.400000,0.400000))).w;
    // 216: eq r3.x, cb0[25].x, l(0.000000)
    r3.x = (asfloat((uint4)((source[25].xxxx)==(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).x;
    // 217: not r3.y, r3.x
    r3.y = (asfloat(~asuint(r3.xxxx))).y;
    // 218: lt r3.z, r3.w, r1.w
    r3.z = (asfloat((uint4)((r3.wwww)<(r1.wwww)) * 0xffffffffu)).z;
    // 219: and r3.y, r3.z, r3.y
    r3.y = (asfloat(asuint(r3.zzzz) & asuint(r3.yyyy))).y;
    // 220: discard_nz r3.y
    if ((asuint(r3.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 221: ge r1.w, r3.w, r1.w
    r1.w = (asfloat((uint4)((r3.wwww)>=(r1.wwww)) * 0xffffffffu)).w;
    // 222: mad r2.w, r2.w, cb0[0].y, l(-0.900000)
    r2.w = ((r2.wwww)*(source[0].yyyy)+(float4(-0.900000,-0.900000,-0.900000,-0.900000))).w;
    // 223: mul_sat r2.w, r2.w, l(9.999998)
    r2.w = (saturate((r2.wwww)*(float4(9.999998,9.999998,9.999998,9.999998)))).w;
    // 224: mad r3.y, r2.w, l(-2.000000), l(3.000000)
    r3.y = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).y;
    // 225: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 226: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 227: mul r2.w, r3.w, r2.w
    r2.w = ((r3.wwww)*(r2.wwww)).w;
    // 228: movc r1.w, r1.w, r2.w, r3.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (r2.wwww) : (r3.wwww)).w;
    // 229: movc o0.w, r3.x, r1.w, r3.w
    output.targets[0].w = ((asuint(r3.xxxx) != 0u) ? (r1.wwww) : (r3.wwww)).w;
    // 230: mad o0.xyz, r2.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 231: mov r1.z, r4.y
    r1.z = (r4.yyyy).z;
    // 232: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 233: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 234: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 235: dp3 r1.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).w;
    // 236: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 237: ge r1.z, l(0.000000), r1.z
    r1.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).z;
    // 238: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 239: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 240: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 241: movc r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.zzzz) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 242: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 243: dp3 o4.x, r4.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 244: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 245: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 246: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 247: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 248: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 249: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 250: mov o3.xyzw, r5.xyzw
    output.targets[3].xyzw = (r5.xyzw).xyzw;
    // 251: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 252: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 253: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 254: ret
    return output;
}

// source.character.static-map-native-1160.v1 / source program ba1b4d8ac94fe64e954a240b531f54f2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1160(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[12].z=(g_SourceCharacterTime.xxxx).x;
    source[13]=g_SourceCharacterBaseConstants[11];
    source[14]=g_SourceCharacterBaseConstants[12];
    source[14].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15]=g_SourceCharacterBaseConstants[13];
    source[15].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[15].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[15].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[15].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=g_SourceCharacterBaseConstants[14];
    source[17]=g_SourceCharacterBaseConstants[15];
    source[18]=g_SourceCharacterBaseConstants[16];
    source[19]=g_SourceCharacterBaseConstants[17];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[20]=g_SourceCharacterEnvironmentColor;source[21]=g_SourceCharacterEnvironmentRotation;}
    source[33]=1.f;
    source[34]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.wxyz, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).x;
    // 2: add r0.y, r0.x, -cb0[11].x
    r0.y = ((r0.xxxx)+(-(source[11].xxxx))).y;
    // 3: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 4: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 5: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 6: mul r1.xyz, r0.zzzz, v5.xyzx
    r1.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 7: mad r0.yz, r1.xxyx, r0.yyyy, v4.xxyx
    r0.yz = ((r1.xxyx)*(r0.yyyy)+(v4.xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 9: add_sat r0.w, r0.x, r2.w
    r0.w = (saturate((r0.xxxx)+(r2.wwww))).w;
    // 10: add_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)+(source[16].zzzz))).x;
    // 11: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 12: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 13: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 16: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 17: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 18: mad r0.w, cb0[12].y, cb0[12].z, r0.w
    r0.w = ((source[12].yyyy)*(source[12].zzzz)+(r0.wwww)).w;
    // 19: mul r1.w, r0.w, l(3.524534)
    r1.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 20: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 21: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 22: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 23: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 24: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: mad r0.w, r0.w, l(0.500000), cb0[12].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].xxxx)).w;
    // 26: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: mul r4.xyz, cb0[5].xyzx, cb0[11].wwww
    r4.xyz = ((source[5].xyzx)*(source[11].wwww)).xyz;
    // 29: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 30: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 31: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: mad r3.xyz, -r0.wwww, r3.xyzx, r1.wwww
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(r1.wwww)).xyz;
    // 33: mad r3.xyz, cb0[12].wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 34: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 36: mad r3.xyz, cb0[13].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[13].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 39: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 41: mul r4.xy, r4.xyxx, cb0[11].zzzz
    r4.xy = ((r4.xyxx)*(source[11].zzzz)).xy;
    // 42: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 50: dp3 r1.w, r4.xyzx, r1.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 51: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: add r2.w, -|r1.z|, l(1.000000)
    r2.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mad_sat r2.w, r1.w, cb0[13].y, -cb0[13].z
    r2.w = (saturate((r1.wwww)*(source[13].yyyy)+(-(source[13].zzzz)))).w;
    // 55: log r3.w, r2.w
    r3.w = (log2(r2.wwww)).w;
    // 56: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 57: mul r3.w, r3.w, cb0[13].w
    r3.w = ((r3.wwww)*(source[13].wwww)).w;
    // 58: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 59: mul r5.xyz, r3.wwww, cb0[8].xyzx
    r5.xyz = ((r3.wwww)*(source[8].xyzx)).xyz;
    // 60: movc r5.xyz, r2.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 61: add r5.xyz, r5.xyzx, -cb0[8].xyzx
    r5.xyz = ((r5.xyzx)+(-(source[8].xyzx))).xyz;
    // 62: mad r5.xyz, cb0[8].wwww, r5.xyzx, cb0[8].xyzx
    r5.xyz = ((source[8].wwww)*(r5.xyzx)+(source[8].xyzx)).xyz;
    // 63: mul r6.xyz, cb0[9].xyzx, cb0[14].yyyy
    r6.xyz = ((source[9].xyzx)*(source[14].yyyy)).xyz;
    // 64: mul r6.xyz, r6.xyzx, cb0[15].wwww
    r6.xyz = ((r6.xyzx)*(source[15].wwww)).xyz;
    // 65: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 66: mul r6.xyz, r6.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 67: max r6.xyz, |r6.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r6.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 68: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 69: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 70: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 71: min r6.xyz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 73: mul r5.xyz, r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 74: mad r6.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mad r7.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 77: mad r3.xyz, r3.xyzx, r6.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 78: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 79: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 80: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 81: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 82: mul r5.xyz, r2.wwww, cb0[3].xyzx
    r5.xyz = ((r2.wwww)*(source[3].xyzx)).xyz;
    // 83: movc r5.xyz, r1.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 84: add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 85: add r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)+(source[2].xyzx)).xyz;
    // 86: mul r5.xyz, cb0[10].xyzx, cb0[16].yyyy
    r5.xyz = ((source[10].xyzx)*(source[16].yyyy)).xyz;
    // 87: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 88: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 89: mul r5.xyz, r2.xyzx, cb0[16].wwww
    r5.xyz = ((r2.xyzx)*(source[16].wwww)).xyz;
    // 90: mad r2.xyz, cb0[17].xxxx, r2.xyzx, -r5.xyzx
    r2.xyz = ((source[17].xxxx)*(r2.xyzx)+(-(r5.xyzx))).xyz;
    // 91: mul r0.x, r0.w, cb0[17].y
    r0.x = ((r0.wwww)*(source[17].yyyy)).x;
    // 92: mul r0.yz, r0.zzyz, cb0[18].yywy
    r0.yz = ((r0.zzyz)*(source[18].yywy)).yz;
    // 93: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 94: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 95: mul r0.w, r0.w, cb0[17].z
    r0.w = ((r0.wwww)*(source[17].zzzz)).w;
    // 96: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 97: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 98: min r0.w, r0.x, l(1.000000)
    r0.w = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul_sat r6.w, r0.x, cb2[3].w
    r6.w = (saturate((r0.xxxx)*(passValues[3].wwww))).w;
    // 100: mad r2.xyz, r0.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 101: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 102: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 103: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 104: mad r5.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 105: mad r7.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 106: mad r8.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 107: log r0.xw, |r0.yyyz|
    r0.xw = (log2(abs(r0.yyyz))).xw;
    // 108: lt r0.yz, |r0.yyzy|, l(0.000000, 0.000001, 0.000001, 0.000000)
    r0.yz = (asfloat((uint4)((abs(r0.yyzy))<(float4(0.000000,0.000001,0.000001,0.000000))) * 0xffffffffu)).yz;
    // 109: mul r0.w, r0.w, cb0[19].x
    r0.w = ((r0.wwww)*(source[19].xxxx)).w;
    // 110: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 111: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 112: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 113: max r0.x, r0.x, cb0[1].x
    r0.x = (max(r0.xxxx,source[1].xxxx)).x;
    // 114: min r6.z, r0.x, l(1.000000)
    r6.z = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 115: exp r0.x, r0.w
    r0.x = (exp2(r0.wwww)).x;
    // 116: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 118: mad r0.yzw, r0.xxxx, r7.xxyz, r8.xxyz
    r0.yzw = ((r0.xxxx)*(r7.xxyz)+(r8.xxyz)).yzw;
    // 119: mad r0.yzw, r0.yyzw, r0.xxxx, r5.xxyz
    r0.yzw = ((r0.yyzw)*(r0.xxxx)+(r5.xxyz)).yzw;
    // 120: mul r0.yzw, r0.xxxx, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r0.yyzw)).yzw;
    // 121: max r0.yzw, r0.yyzw, r0.xxxx
    r0.yzw = (max(r0.yyzw,r0.xxxx)).yzw;
    // 122: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 123: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 124: mul r5.xyz, r1.wwww, v6.xyzx
    r5.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 125: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 126: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 127: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 128: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 129: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 130: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 131: mul r5.yzw, r5.yyyy, cb0[31].xxyz
    r5.yzw = ((r5.yyyy)*(source[31].xxyz)).yzw;
    // 132: mad r5.xyz, r5.xxxx, cb0[30].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[30].xyzx)+(r5.yzwy)).xyz;
    // 133: mul r5.xyz, r5.xyzx, cb0[32].wwww
    r5.xyz = ((r5.xyzx)*(source[32].wwww)).xyz;
    // 134: mul r7.xyz, r2.xyzx, r5.xyzx
    r7.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 135: dp2_sat r8.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 136: dp3_sat r8.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 137: dp3_sat r8.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 138: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 139: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 140: mul r9.xyz, r9.xyzx, cb0[34].xyzx
    r9.xyz = ((r9.xyzx)*(source[34].xyzx)).xyz;
    // 141: dp3 r1.w, r9.xyzx, r8.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).w;
    // 142: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t6.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 143: mul r8.xyz, r8.xyzx, cb0[33].xyzx
    r8.xyz = ((r8.xyzx)*(source[33].xyzx)).xyz;
    // 144: mul r10.xyz, r1.wwww, r8.xyzx
    r10.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 145: mad r7.xyz, r2.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 146: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 147: dp3 r3.w, r4.xyzx, r1.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 148: deriv_rtx_coarse r6.x, r3.w
    r6.x = (ddx_coarse(r3.wwww)).x;
    // 149: deriv_rty_coarse r6.y, r3.w
    r6.y = (ddy_coarse(r3.wwww)).y;
    // 150: dp2 r4.w, r6.xyxx, r6.xyxx
    r4.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 151: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 152: mad_sat r6.y, r4.w, l(0.300000), r6.z
    r6.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 153: add r4.w, -r6.y, l(1.000000)
    r4.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mov_sat r2.w, cb0[17].w
    r2.w = (saturate(source[17].wwww)).w;
    // 155: mad r7.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r7.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 156: mul r5.w, r2.w, l(0.080000)
    r5.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 157: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 158: mad r7.xyz, r6.wwww, r7.xyzx, r5.wwww
    r7.xyz = ((r6.wwww)*(r7.xyzx)+(r5.wwww)).xyz;
    // 159: max r10.xyz, r4.wwww, r7.xyzx
    r10.xyz = (max(r4.wwww,r7.xyzx)).xyz;
    // 160: add r10.xyz, -r7.xyzx, r10.xyzx
    r10.xyz = ((-(r7.xyzx))+(r10.xyzx)).xyz;
    // 161: mul_sat r2.w, r7.y, l(50.000000)
    r2.w = (saturate((r7.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 162: mul r10.xyz, r2.wwww, r10.xyzx
    r10.xyz = ((r2.wwww)*(r10.xyzx)).xyz;
    // 163: add r2.w, r3.w, l(1.000000)
    r2.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: mul r11.xyz, r3.wwww, r4.xyzx
    r11.xyz = ((r3.wwww)*(r4.xyzx)).xyz;
    // 165: mad r1.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 166: add r3.w, r1.z, l(1.000000)
    r3.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: add_sat r6.x, r2.w, -r3.w
    r6.x = (saturate((r2.wwww)+(-(r3.wwww)))).x;
    // 169: sample_indexable(texture2d)(float,float,float,float) r11.xy, r6.xyxx, t4.xyzw, s6
    r11.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 170: add r2.w, r0.x, r6.x
    r2.w = ((r0.xxxx)+(r6.xxxx)).w;
    // 171: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 172: mul r12.xyz, r7.xyzx, r11.yyyy
    r12.xyz = ((r7.xyzx)*(r11.yyyy)).xyz;
    // 173: mad r10.xyz, r10.xyzx, r11.xxxx, r12.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xxxx)+(r12.xyzx)).xyz;
    // 174: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r11.y
    r3.w = r11.y != 0.f ? 1.f / r11.y : 0.f;
    // 175: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 176: mad r11.xyz, r7.xyzx, r3.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((r7.xyzx)*(r3.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 177: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: mad r7.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r7.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 179: mad r12.xyz, -r10.xyzx, r11.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r12.xyz = ((-(r10.xyzx))*(r11.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 180: mul r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)*(r11.xyzx)).xyz;
    // 181: dp3 r3.w, v1.xyzx, v1.xyzx
    r3.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 182: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 183: mul r11.xyz, r3.wwww, v1.xyzx
    r11.xyz = ((r3.wwww)*(v1.xyzx)).xyz;
    // 184: dp3 r13.y, r11.xyzx, r4.xyzx
    r13.y = (dot((r11.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 185: dp3 r3.w, v0.xyzx, v0.xyzx
    r3.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 186: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 187: mul r14.xyz, r3.wwww, v0.xyzx
    r14.xyz = ((r3.wwww)*(v0.xyzx)).xyz;
    // 188: mul r15.xyz, r11.zxyz, r14.yzxy
    r15.xyz = ((r11.zxyz)*(r14.yzxy)).xyz;
    // 189: mad r15.xyz, r11.yzxy, r14.zxyz, -r15.xyzx
    r15.xyz = ((r11.yzxy)*(r14.zxyz)+(-(r15.xyzx))).xyz;
    // 190: dp3 r11.y, r11.xyzx, r1.xyzx
    r11.y = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 191: mul r15.xyz, r15.xyzx, v1.wwww
    r15.xyz = ((r15.xyzx)*(v1.wwww)).xyz;
    // 192: dp3 r16.y, r15.xyzx, r4.xyzx
    r16.y = (dot((r15.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 193: dp3 r16.x, r14.xyzx, r4.xyzx
    r16.x = (dot((r14.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 194: dp3 r4.x, r14.xyzx, r1.xyzx
    r4.x = (dot((r14.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 195: dp3 r4.y, r15.xyzx, r1.xyzx
    r4.y = (dot((r15.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 196: dp2 r13.z, r16.xyxx, cb0[21].xyxx
    r13.z = (dot((r16.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 197: mul r4.zw, cb0[21].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r4.zw = ((source[21].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 198: dp2 r13.x, r16.xyxx, r4.zwzz
    r13.x = (dot((r16.xyxx).xy,(r4.zwzz).xy).xxxx).x;
    // 199: dp2 r11.x, r4.xyxx, r4.zwzz
    r11.x = (dot((r4.xyxx).xy,(r4.zwzz).xy).xxxx).x;
    // 200: dp2 r11.z, r4.xyxx, cb0[21].xyxx
    r11.z = (dot((r4.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 201: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 202: dp4 r4.x, cb0[22].xyzw, r13.xyzw
    r4.x = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 203: dp4 r4.y, cb0[23].xyzw, r13.xyzw
    r4.y = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 204: dp4 r4.z, cb0[24].xyzw, r13.xyzw
    r4.z = (dot((source[24].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 205: mul r14.xyzw, r13.yzzx, r13.xyzz
    r14.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 206: dp4 r15.x, cb0[25].xyzw, r14.xyzw
    r15.x = (dot((source[25].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 207: dp4 r15.y, cb0[26].xyzw, r14.xyzw
    r15.y = (dot((source[26].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 208: dp4 r15.z, cb0[27].xyzw, r14.xyzw
    r15.z = (dot((source[27].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 209: add r4.xyz, r4.xyzx, r15.xyzx
    r4.xyz = ((r4.xyzx)+(r15.xyzx)).xyz;
    // 210: mul r3.w, r13.y, r13.y
    r3.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 211: mov r16.z, r13.y
    r16.z = (r13.yyyy).z;
    // 212: mad r3.w, r13.x, r13.x, -r3.w
    r3.w = ((r13.xxxx)*(r13.xxxx)+(-(r3.wwww))).w;
    // 213: mad r4.xyz, cb0[28].xyzx, r3.wwww, r4.xyzx
    r4.xyz = ((source[28].xyzx)*(r3.wwww)+(r4.xyzx)).xyz;
    // 214: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 215: mul r4.xyz, r4.xyzx, cb0[20].xyzx
    r4.xyz = ((r4.xyzx)*(source[20].xyzx)).xyz;
    // 216: mad r4.xyz, r4.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r4.xyz = ((r4.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 217: mul r4.xyz, r12.xyzx, r4.xyzx
    r4.xyz = ((r12.xyzx)*(r4.xyzx)).xyz;
    // 218: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 219: mad r0.yzw, -r0.yyzw, r6.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r6.wwww)+(r0.yyzw)).yzw;
    // 220: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 221: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 222: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 223: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 224: mul r1.xyz, r4.xyzx, r4.xyzx
    r1.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 225: dp3 r1.x, r9.xyzx, r1.xyzx
    r1.x = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 226: add r1.y, -r1.x, r1.w
    r1.y = ((-(r1.xxxx))+(r1.wwww)).y;
    // 227: mad r1.x, r6.z, r1.y, r1.x
    r1.x = ((r6.zzzz)*(r1.yyyy)+(r1.xxxx)).x;
    // 228: mad r1.yzw, r8.xxyz, r1.xxxx, r5.xxyz
    r1.yzw = ((r8.xxyz)*(r1.xxxx)+(r5.xxyz)).yzw;
    // 229: mul r4.xyz, r1.xxxx, r8.xyzx
    r4.xyz = ((r1.xxxx)*(r8.xyzx)).xyz;
    // 230: mul r1.x, r6.y, r6.y
    r1.x = ((r6.yyyy)*(r6.yyyy)).x;
    // 231: mul r3.w, r6.y, l(5.000000)
    r3.w = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 232: sample_l_indexable(texturecube)(float,float,float,float) r5.xyzw, r11.xyzx, t5.xyzw, s5, r3.w
    r5.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r11.xyzx).xyz, (r3.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 233: mul r5.xyz, r5.xyzx, r5.wwww
    r5.xyz = ((r5.xyzx)*(r5.wwww)).xyz;
    // 234: mul r5.xyz, r5.xyzx, cb0[20].xyzx
    r5.xyz = ((r5.xyzx)*(source[20].xyzx)).xyz;
    // 235: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 236: mul r1.x, r2.w, r1.x
    r1.x = ((r2.wwww)*(r1.xxxx)).x;
    // 237: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 238: add r1.x, r0.x, r1.x
    r1.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 239: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 240: add_sat r0.x, r1.x, l(-1.000000)
    r0.x = (saturate((r1.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 241: mad r1.x, r0.x, r7.x, r7.y
    r1.x = ((r0.xxxx)*(r7.xxxx)+(r7.yyyy)).x;
    // 242: mad r1.x, r1.x, r0.x, r7.z
    r1.x = ((r1.xxxx)*(r0.xxxx)+(r7.zzzz)).x;
    // 243: mul r1.x, r0.x, r1.x
    r1.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 244: max r0.x, r0.x, r1.x
    r0.x = (max(r0.xxxx,r1.xxxx)).x;
    // 245: mul r6.xyz, r0.xxxx, r1.yzwy
    r6.xyz = ((r0.xxxx)*(r1.yzwy)).xyz;
    // 246: add r1.xyz, r1.yzwy, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.yzwy)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 247: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 248: dp3 r0.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 249: mul r1.xyz, r5.xyzx, r6.xyzx
    r1.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 250: mad r0.yzw, r1.xxyz, r10.xxyz, r0.yyzw
    r0.yzw = ((r1.xxyz)*(r10.xxyz)+(r0.yyzw)).yzw;
    // 251: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 252: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 253: add r1.xyz, r0.yzwy, r3.xyzx
    r1.xyz = ((r0.yzwy)+(r3.xyzx)).xyz;
    // 254: mad o0.xyz, r2.xyzx, cb0[32].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[32].xyzx)+(r1.xyzx)).xyz;
    // 255: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 256: dp3 r1.x, r16.xyzx, r16.xyzx
    r1.x = (dot((r16.xyzx).xyz,(r16.xyzx).xyz).xxxx).x;
    // 257: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 258: mul r1.xyz, r1.xxxx, r16.xyzx
    r1.xyz = ((r1.xxxx)*(r16.xyzx)).xyz;
    // 259: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 260: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 261: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 262: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 263: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 264: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 265: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 266: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 267: mul o4.z, r0.x, r0.y
    output.targets[4].z = ((r0.xxxx)*(r0.yyyy)).z;
    // 268: dp3 o4.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 269: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 270: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
    // 271: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 272: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 273: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 274: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 275: ret
    return output;
}

// source.character.static-map-native-1160.v1 / source program 9d66afd8f5502d4ba941934cb727d2ea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1160(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1160(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterBaseConstants[63];
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[11]=g_SourceCharacterBaseConstants[9];
    source[12]=g_SourceCharacterBaseConstants[10];
    source[12].z=(g_SourceCharacterTime.xxxx).x;
    source[13]=g_SourceCharacterBaseConstants[11];
    source[14]=g_SourceCharacterBaseConstants[12];
    source[14].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[15]=g_SourceCharacterBaseConstants[13];
    source[15].x=(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))).x;
    source[15].y=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[15].z=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[15].w=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[16]=g_SourceCharacterBaseConstants[14];
    source[17]=g_SourceCharacterBaseConstants[15];
    source[18]=g_SourceCharacterBaseConstants[16];
    source[19]=g_SourceCharacterBaseConstants[17];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[20]=g_SourceCharacterEnvironmentColor;source[21]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.wxyz, s0, l(0.000000)
    r0.x = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).x;
    // 2: add r0.y, r0.x, -cb0[11].x
    r0.y = ((r0.xxxx)+(-(source[11].xxxx))).y;
    // 3: mul r0.y, r0.y, cb0[11].y
    r0.y = ((r0.yyyy)*(source[11].yyyy)).y;
    // 4: dp3 r0.z, v5.xyzx, v5.xyzx
    r0.z = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).z;
    // 5: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 6: mul r1.xyz, r0.zzzz, v5.xyzx
    r1.xyz = ((r0.zzzz)*(v5.xyzx)).xyz;
    // 7: mad r0.yz, r1.xxyx, r0.yyyy, v4.xxyx
    r0.yz = ((r1.xxyx)*(r0.yyyy)+(v4.xxyx)).yz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.yzyy, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 9: add_sat r0.w, r0.x, r2.w
    r0.w = (saturate((r0.xxxx)+(r2.wwww))).w;
    // 10: add_sat r0.x, r0.x, cb0[16].z
    r0.x = (saturate((r0.xxxx)+(source[16].zzzz))).x;
    // 11: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 12: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 13: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 14: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 15: add r0.w, cb0[0].y, cb0[0].x
    r0.w = ((source[0].yyyy)+(source[0].xxxx)).w;
    // 16: add r0.w, r0.w, cb0[0].z
    r0.w = ((r0.wwww)+(source[0].zzzz)).w;
    // 17: mul r0.w, r0.w, l(0.010000)
    r0.w = ((r0.wwww)*(float4(0.010000,0.010000,0.010000,0.010000))).w;
    // 18: mad r0.w, cb0[12].y, cb0[12].z, r0.w
    r0.w = ((source[12].yyyy)*(source[12].zzzz)+(r0.wwww)).w;
    // 19: mul r1.w, r0.w, l(3.524534)
    r1.w = ((r0.wwww)*(float4(3.524534,3.524534,3.524534,3.524534))).w;
    // 20: sincos null, r1.w, r1.w
    r1.w = (cos(r1.wwww)).w;
    // 21: add r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)+(r1.wwww)).w;
    // 22: mul r0.w, r0.w, l(1.328987)
    r0.w = ((r0.wwww)*(float4(1.328987,1.328987,1.328987,1.328987))).w;
    // 23: sincos r0.w, null, r0.w
    r0.w = (sin(r0.wwww)).w;
    // 24: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: mad r0.w, r0.w, l(0.500000), cb0[12].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[12].xxxx)).w;
    // 26: mul r3.xy, v4.xyxx, cb0[4].xyxx
    r3.xy = ((v4.xyxx)*(source[4].xyxx)).xy;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 28: mul r4.xyz, cb0[5].xyzx, cb0[11].wwww
    r4.xyz = ((source[5].xyzx)*(source[11].wwww)).xyz;
    // 29: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 30: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 31: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 32: mad r3.xyz, -r0.wwww, r3.xyzx, r1.wwww
    r3.xyz = ((-(r0.wwww))*(r3.xyzx)+(r1.wwww)).xyz;
    // 33: mad r3.xyz, cb0[12].wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 34: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 36: mad r3.xyz, cb0[13].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[13].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, r0.yzyy, t1.xyzw, s1, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 39: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 41: mul r4.xy, r4.xyxx, cb0[11].zzzz
    r4.xy = ((r4.xyxx)*(source[11].zzzz)).xy;
    // 42: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 50: dp3 r1.w, r4.xyzx, r1.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 51: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: add r2.w, -|r1.z|, l(1.000000)
    r2.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mad_sat r2.w, r1.w, cb0[13].y, -cb0[13].z
    r2.w = (saturate((r1.wwww)*(source[13].yyyy)+(-(source[13].zzzz)))).w;
    // 55: log r3.w, r2.w
    r3.w = (log2(r2.wwww)).w;
    // 56: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 57: mul r3.w, r3.w, cb0[13].w
    r3.w = ((r3.wwww)*(source[13].wwww)).w;
    // 58: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 59: mul r5.xyz, r3.wwww, cb0[8].xyzx
    r5.xyz = ((r3.wwww)*(source[8].xyzx)).xyz;
    // 60: movc r5.xyz, r2.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 61: add r5.xyz, r5.xyzx, -cb0[8].xyzx
    r5.xyz = ((r5.xyzx)+(-(source[8].xyzx))).xyz;
    // 62: mad r5.xyz, cb0[8].wwww, r5.xyzx, cb0[8].xyzx
    r5.xyz = ((source[8].wwww)*(r5.xyzx)+(source[8].xyzx)).xyz;
    // 63: mul r6.xyz, cb0[9].xyzx, cb0[14].yyyy
    r6.xyz = ((source[9].xyzx)*(source[14].yyyy)).xyz;
    // 64: mul r6.xyz, r6.xyzx, cb0[15].wwww
    r6.xyz = ((r6.xyzx)*(source[15].wwww)).xyz;
    // 65: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 66: mul r6.xyz, r6.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r6.xyz = ((r6.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 67: max r6.xyz, |r6.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r6.xyz = (max(abs(r6.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 68: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 69: mul r6.xyz, r6.xyzx, cb0[16].xxxx
    r6.xyz = ((r6.xyzx)*(source[16].xxxx)).xyz;
    // 70: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 71: min r6.xyz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: add r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)+(r6.xyzx)).xyz;
    // 73: mul r5.xyz, r5.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r5.xyz = ((r5.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 74: mad r6.xyz, cb0[6].wwww, cb0[6].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((source[6].wwww)*(source[6].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mad r7.xyz, cb0[7].wwww, cb0[7].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((source[7].wwww)*(source[7].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 76: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 77: mad r3.xyz, r3.xyzx, r6.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 78: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 79: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 80: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 81: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 82: mul r5.xyz, r2.wwww, cb0[3].xyzx
    r5.xyz = ((r2.wwww)*(source[3].xyzx)).xyz;
    // 83: movc r5.xyz, r1.wwww, l(0,0,0,0), r5.xyzx
    r5.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r5.xyzx)).xyz;
    // 84: add r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)+(r5.xyzx)).xyz;
    // 85: add r3.xyz, r3.xyzx, cb0[2].xyzx
    r3.xyz = ((r3.xyzx)+(source[2].xyzx)).xyz;
    // 86: mul r5.xyz, cb0[10].xyzx, cb0[16].yyyy
    r5.xyz = ((source[10].xyzx)*(source[16].yyyy)).xyz;
    // 87: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 88: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 89: mul r5.xyz, r2.xyzx, cb0[16].wwww
    r5.xyz = ((r2.xyzx)*(source[16].wwww)).xyz;
    // 90: mad r2.xyz, cb0[17].xxxx, r2.xyzx, -r5.xyzx
    r2.xyz = ((source[17].xxxx)*(r2.xyzx)+(-(r5.xyzx))).xyz;
    // 91: mul r0.x, r0.w, cb0[17].y
    r0.x = ((r0.wwww)*(source[17].yyyy)).x;
    // 92: mul r0.yz, r0.zzyz, cb0[18].yywy
    r0.yz = ((r0.zzyz)*(source[18].yywy)).yz;
    // 93: log r0.w, |r0.x|
    r0.w = (log2(abs(r0.xxxx))).w;
    // 94: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 95: mul r0.w, r0.w, cb0[17].z
    r0.w = ((r0.wwww)*(source[17].zzzz)).w;
    // 96: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 97: movc r0.x, r0.x, l(0), r0.w
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).x;
    // 98: min r0.w, r0.x, l(1.000000)
    r0.w = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: mul_sat r6.w, r0.x, cb2[3].w
    r6.w = (saturate((r0.xxxx)*(passValues[3].wwww))).w;
    // 100: mad r2.xyz, r0.wwww, r2.xyzx, r5.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 101: add r5.xyz, -cb0[2].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[2].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 102: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 103: mad_sat r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = (saturate((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 104: mad r5.xyz, r2.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r5.xyz = ((r2.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 105: mad r7.xyz, r2.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r7.xyz = ((r2.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 106: mad r8.xyz, r2.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r8.xyz = ((r2.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 107: log r0.xw, |r0.yyyz|
    r0.xw = (log2(abs(r0.yyyz))).xw;
    // 108: lt r0.yz, |r0.yyzy|, l(0.000000, 0.000001, 0.000001, 0.000000)
    r0.yz = (asfloat((uint4)((abs(r0.yyzy))<(float4(0.000000,0.000001,0.000001,0.000000))) * 0xffffffffu)).yz;
    // 109: mul r0.w, r0.w, cb0[19].x
    r0.w = ((r0.wwww)*(source[19].xxxx)).w;
    // 110: mul r0.x, r0.x, cb0[18].z
    r0.x = ((r0.xxxx)*(source[18].zzzz)).x;
    // 111: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 112: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 113: max r0.x, r0.x, cb0[1].x
    r0.x = (max(r0.xxxx,source[1].xxxx)).x;
    // 114: min r6.z, r0.x, l(1.000000)
    r6.z = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 115: exp r0.x, r0.w
    r0.x = (exp2(r0.wwww)).x;
    // 116: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 117: movc r0.x, r0.z, l(0), r0.x
    r0.x = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 118: mad r0.yzw, r0.xxxx, r7.xxyz, r8.xxyz
    r0.yzw = ((r0.xxxx)*(r7.xxyz)+(r8.xxyz)).yzw;
    // 119: mad r0.yzw, r0.yyzw, r0.xxxx, r5.xxyz
    r0.yzw = ((r0.yyzw)*(r0.xxxx)+(r5.xxyz)).yzw;
    // 120: mul r0.yzw, r0.xxxx, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r0.yyzw)).yzw;
    // 121: max r0.yzw, r0.yyzw, r0.xxxx
    r0.yzw = (max(r0.yyzw,r0.xxxx)).yzw;
    // 122: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 123: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 124: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 125: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 126: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 127: mul r5.xyz, r1.wwww, v6.xyzx
    r5.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 128: dp3 r1.w, r5.xyzx, r4.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 129: mad r6.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 130: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 131: mul r7.xyz, r6.yyyy, cb0[31].xyzx
    r7.xyz = ((r6.yyyy)*(source[31].xyzx)).xyz;
    // 132: mad r7.xyz, r6.xxxx, cb0[30].xyzx, r7.xyzx
    r7.xyz = ((r6.xxxx)*(source[30].xyzx)+(r7.xyzx)).xyz;
    // 133: mul r7.xyz, r7.xyzx, cb0[32].wwww
    r7.xyz = ((r7.xyzx)*(source[32].wwww)).xyz;
    // 134: mul r7.xyz, r2.xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 135: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 136: dp3 r1.w, r4.xyzx, r1.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 137: mul r7.xyz, r1.wwww, r4.xyzx
    r7.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 138: mad r1.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 139: add r3.w, r1.z, l(1.000000)
    r3.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 141: add r4.w, r1.w, l(1.000000)
    r4.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: add_sat r6.x, -r3.w, r4.w
    r6.x = (saturate((-(r3.wwww))+(r4.wwww))).x;
    // 143: deriv_rtx_coarse r7.x, r1.w
    r7.x = (ddx_coarse(r1.wwww)).x;
    // 144: deriv_rty_coarse r7.y, r1.w
    r7.y = (ddy_coarse(r1.wwww)).y;
    // 145: dp2 r1.w, r7.xyxx, r7.xyxx
    r1.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 146: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 147: mad_sat r6.y, r1.w, l(0.300000), r6.z
    r6.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r6.zzzz))).y;
    // 148: mov o2.zw, r6.zzzw
    output.targets[2].zw = (r6.zzzw).zw;
    // 149: sample_indexable(texture2d)(float,float,float,float) r7.xy, r6.xyxx, t4.xyzw, s5
    r7.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 150: add r1.w, r0.x, r6.x
    r1.w = ((r0.xxxx)+(r6.xxxx)).w;
    // 151: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 152: add r3.w, -r6.y, l(1.000000)
    r3.w = ((-(r6.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mov_sat r2.w, cb0[17].w
    r2.w = (saturate(source[17].wwww)).w;
    // 154: mad r8.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r2.xyzx
    r8.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r2.xyzx)).xyz;
    // 155: mul r4.w, r2.w, l(0.080000)
    r4.w = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 156: mov o3.xyzw, r2.xyzw
    output.targets[3].xyzw = (r2.xyzw).xyzw;
    // 157: mad r8.xyz, r6.wwww, r8.xyzx, r4.wwww
    r8.xyz = ((r6.wwww)*(r8.xyzx)+(r4.wwww)).xyz;
    // 158: max r9.xyz, r3.wwww, r8.xyzx
    r9.xyz = (max(r3.wwww,r8.xyzx)).xyz;
    // 159: add r9.xyz, -r8.xyzx, r9.xyzx
    r9.xyz = ((-(r8.xyzx))+(r9.xyzx)).xyz;
    // 160: mul_sat r2.w, r8.y, l(50.000000)
    r2.w = (saturate((r8.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 161: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 162: mul r10.xyz, r7.yyyy, r8.xyzx
    r10.xyz = ((r7.yyyy)*(r8.xyzx)).xyz;
    // 163: mad r7.xzw, r9.xxyz, r7.xxxx, r10.xxyz
    r7.xzw = ((r9.xxyz)*(r7.xxxx)+(r10.xxyz)).xzw;
    // 164: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r7.y
    r2.w = r7.y != 0.f ? 1.f / r7.y : 0.f;
    // 165: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 166: mad r9.xyz, r8.xyzx, r2.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((r8.xyzx)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 167: dp3 r2.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 168: mad r8.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r8.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 169: mad r10.xyz, -r7.xzwx, r9.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r10.xyz = ((-(r7.xzwx))*(r9.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mul r7.xyz, r7.xzwx, r9.xyzx
    r7.xyz = ((r7.xzwx)*(r9.xyzx)).xyz;
    // 171: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 172: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 173: mul r9.xyz, r2.wwww, v1.xyzx
    r9.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 174: dp3 r11.y, r9.xyzx, r4.xyzx
    r11.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 175: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 176: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 177: mul r12.xyz, r2.wwww, v0.xyzx
    r12.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 178: mul r13.xyz, r9.zxyz, r12.yzxy
    r13.xyz = ((r9.zxyz)*(r12.yzxy)).xyz;
    // 179: mad r13.xyz, r9.yzxy, r12.zxyz, -r13.xyzx
    r13.xyz = ((r9.yzxy)*(r12.zxyz)+(-(r13.xyzx))).xyz;
    // 180: dp3 r9.y, r9.xyzx, r1.xyzx
    r9.y = (dot((r9.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 181: mul r13.xyz, r13.xyzx, v1.wwww
    r13.xyz = ((r13.xyzx)*(v1.wwww)).xyz;
    // 182: dp3 r14.y, r13.xyzx, r4.xyzx
    r14.y = (dot((r13.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 183: dp3 r14.x, r12.xyzx, r4.xyzx
    r14.x = (dot((r12.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 184: dp3 r4.x, r12.xyzx, r1.xyzx
    r4.x = (dot((r12.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 185: dp3 r4.y, r13.xyzx, r1.xyzx
    r4.y = (dot((r13.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 186: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 187: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 188: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 189: dp2 r11.z, r14.xyxx, cb0[21].xyxx
    r11.z = (dot((r14.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 190: mul r4.zw, cb0[21].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r4.zw = ((source[21].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 191: dp2 r11.x, r14.xyxx, r4.zwzz
    r11.x = (dot((r14.xyxx).xy,(r4.zwzz).xy).xxxx).x;
    // 192: dp2 r9.x, r4.xyxx, r4.zwzz
    r9.x = (dot((r4.xyxx).xy,(r4.zwzz).xy).xxxx).x;
    // 193: dp2 r9.z, r4.xyxx, cb0[21].xyxx
    r9.z = (dot((r4.xyxx).xy,(source[21].xyxx).xy).xxxx).z;
    // 194: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 195: dp4 r4.x, cb0[22].xyzw, r11.xyzw
    r4.x = (dot((source[22].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 196: dp4 r4.y, cb0[23].xyzw, r11.xyzw
    r4.y = (dot((source[23].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 197: dp4 r4.z, cb0[24].xyzw, r11.xyzw
    r4.z = (dot((source[24].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 198: mul r5.xyzw, r11.yzzx, r11.xyzz
    r5.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 199: dp4 r12.x, cb0[25].xyzw, r5.xyzw
    r12.x = (dot((source[25].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).x;
    // 200: dp4 r12.y, cb0[26].xyzw, r5.xyzw
    r12.y = (dot((source[26].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).y;
    // 201: dp4 r12.z, cb0[27].xyzw, r5.xyzw
    r12.z = (dot((source[27].xyzw).xyzw,(r5.xyzw).xyzw).xxxx).z;
    // 202: add r4.xyz, r4.xyzx, r12.xyzx
    r4.xyz = ((r4.xyzx)+(r12.xyzx)).xyz;
    // 203: mul r1.z, r11.y, r11.y
    r1.z = ((r11.yyyy)*(r11.yyyy)).z;
    // 204: mov r14.z, r11.y
    r14.z = (r11.yyyy).z;
    // 205: mad r1.z, r11.x, r11.x, -r1.z
    r1.z = ((r11.xxxx)*(r11.xxxx)+(-(r1.zzzz))).z;
    // 206: mad r4.xyz, cb0[28].xyzx, r1.zzzz, r4.xyzx
    r4.xyz = ((source[28].xyzx)*(r1.zzzz)+(r4.xyzx)).xyz;
    // 207: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 208: mul r4.xyz, r4.xyzx, cb0[20].xyzx
    r4.xyz = ((r4.xyzx)*(source[20].xyzx)).xyz;
    // 209: mad r4.xyz, r4.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[20].wwww
    r4.xyz = ((r4.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[20].wwww)).xyz;
    // 210: mul r4.xyz, r10.xyzx, r4.xyzx
    r4.xyz = ((r10.xyzx)*(r4.xyzx)).xyz;
    // 211: mul r0.yzw, r0.yyzw, r4.xxyz
    r0.yzw = ((r0.yyzw)*(r4.xxyz)).yzw;
    // 212: mad r0.yzw, -r0.yyzw, r6.wwww, r0.yyzw
    r0.yzw = ((-(r0.yyzw))*(r6.wwww)+(r0.yyzw)).yzw;
    // 213: mul r1.z, r6.y, l(5.000000)
    r1.z = ((r6.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 214: mul r2.w, r6.y, r6.y
    r2.w = ((r6.yyyy)*(r6.yyyy)).w;
    // 215: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 216: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 217: add r1.w, r0.x, r1.w
    r1.w = ((r0.xxxx)+(r1.wwww)).w;
    // 218: mov o5.y, r0.x
    output.targets[5].y = (r0.xxxx).y;
    // 219: add_sat r0.x, r1.w, l(-1.000000)
    r0.x = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 220: sample_l_indexable(texturecube)(float,float,float,float) r4.xyzw, r9.xyzx, t5.xyzw, s4, r1.z
    r4.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r1.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 221: mul r4.xyz, r4.xyzx, r4.wwww
    r4.xyz = ((r4.xyzx)*(r4.wwww)).xyz;
    // 222: mul r4.xyz, r4.xyzx, cb0[20].xyzx
    r4.xyz = ((r4.xyzx)*(source[20].xyzx)).xyz;
    // 223: mad r4.xyz, r4.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[20].wwww
    r4.xyz = ((r4.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[20].wwww)).xyz;
    // 224: mad r1.z, r0.x, r8.x, r8.y
    r1.z = ((r0.xxxx)*(r8.xxxx)+(r8.yyyy)).z;
    // 225: mad r1.z, r1.z, r0.x, r8.z
    r1.z = ((r1.zzzz)*(r0.xxxx)+(r8.zzzz)).z;
    // 226: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 227: max r0.x, r0.x, r1.z
    r0.x = (max(r0.xxxx,r1.zzzz)).x;
    // 228: mul r1.yzw, r1.yyyy, cb0[31].xxyz
    r1.yzw = ((r1.yyyy)*(source[31].xxyz)).yzw;
    // 229: mad r1.xyz, cb0[30].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[30].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 230: mul r1.xyz, r1.xyzx, cb0[32].wwww
    r1.xyz = ((r1.xyzx)*(source[32].wwww)).xyz;
    // 231: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 232: mul r1.xyz, r1.xyzx, r4.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 233: mad r0.xyz, r1.xyzx, r7.xyzx, r0.yzwy
    r0.xyz = ((r1.xyzx)*(r7.xyzx)+(r0.yzwy)).xyz;
    // 234: mul r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)*(r1.xyzx)).xyz;
    // 235: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 236: add r1.xyz, r0.xyzx, r3.xyzx
    r1.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 237: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 238: mad o0.xyz, r2.xyzx, cb0[32].xyzx, r1.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[32].xyzx)+(r1.xyzx)).xyz;
    // 239: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 240: dp3 r0.x, r14.xyzx, r14.xyzx
    r0.x = (dot((r14.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 241: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 242: mul r0.xyz, r0.xxxx, r14.xyzx
    r0.xyz = ((r0.xxxx)*(r14.xyzx)).xyz;
    // 243: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 244: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 245: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 246: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 247: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 248: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 249: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 250: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 251: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 252: ftou r0.x, cb0[29].z
    r0.x = (asfloat((uint4)(source[29].zzzz))).x;
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

// source.character.static-map-native-1161.v1 / source program a3db118718e12541b396ceff69c573dc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1161(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[12]=g_SourceCharacterBaseConstants[13];
    source[12].z=(g_SourceCharacterTime.xxxx).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[13].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    source[17]=g_SourceCharacterBaseConstants[19];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[18]=g_SourceCharacterEnvironmentColor;source[19]=g_SourceCharacterEnvironmentRotation;}
    source[31]=1.f;
    source[32]=1.f;
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
    // 8: mad r1.xyz, cb0[13].wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((source[13].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r2.xyz, cb0[6].xyzx, cb0[14].xxxx
    r2.xyz = ((source[6].xyzx)*(source[14].xxxx)).xyz;
    // 10: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 11: mul r2.xyz, cb0[7].xyzx, cb0[14].yyyy
    r2.xyz = ((source[7].xyzx)*(source[14].yyyy)).xyz;
    // 12: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 14: mul r1.w, r2.z, cb0[14].z
    r1.w = ((r2.zzzz)*(source[14].zzzz)).w;
    // 15: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 16: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r2.z, r2.z, cb0[14].w
    r2.z = ((r2.zzzz)*(source[14].wwww)).z;
    // 18: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 19: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 20: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 21: mul_sat r3.w, r1.w, cb2[3].w
    r3.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 22: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, cb0[8].xyzx, cb0[15].xxxx
    r1.xyz = ((source[8].xyzx)*(source[15].xxxx)).xyz;
    // 24: mul r3.xy, v4.xyxx, cb0[9].yyyy
    r3.xy = ((v4.xyxx)*(source[9].yyyy)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 27: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 28: mul r5.xyz, r1.xyzx, r4.xyzx
    r5.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 29: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r1.xyz, -r1.xyzx, r4.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 31: mul r1.w, r4.w, r4.w
    r1.w = ((r4.wwww)*(r4.wwww)).w;
    // 32: mad r1.xyz, cb0[15].zzzz, r1.xyzx, r5.xyzx
    r1.xyz = ((source[15].zzzz)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 33: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 35: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 36: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 37: mul r4.xy, r4.xyxx, cb0[9].xxxx
    r4.xy = ((r4.xyxx)*(source[9].xxxx)).xy;
    // 38: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 39: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 42: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 44: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 45: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 46: mul r2.w, r4.z, r4.z
    r2.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 47: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 48: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 50: max r1.w, cb0[9].w, l(0.000000)
    r1.w = (max(source[9].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 51: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 52: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 53: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 54: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 56: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 57: mul r6.xyz, r4.wwww, v0.xyzx
    r6.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 58: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 59: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 60: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 61: mul r8.xyz, r4.wwww, v1.xyzx
    r8.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 62: dp3 r7.z, r8.xyzx, r4.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 63: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 64: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 65: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 67: dp3 r4.w, r7.xyzx, r5.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 68: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 70: mad r2.w, r4.w, r2.w, r4.w
    r2.w = ((r4.wwww)*(r2.wwww)+(r4.wwww)).w;
    // 71: add r4.w, -r1.w, r2.w
    r4.w = ((-(r1.wwww))+(r2.wwww)).w;
    // 72: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 74: mad r2.w, -r1.w, r4.w, r2.w
    r2.w = ((-(r1.wwww))*(r4.wwww)+(r2.wwww)).w;
    // 75: mul r1.w, r4.w, r1.w
    r1.w = ((r4.wwww)*(r1.wwww)).w;
    // 76: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 77: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 78: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 79: mul r1.xyz, r0.xyzx, cb0[15].wwww
    r1.xyz = ((r0.xyzx)*(source[15].wwww)).xyz;
    // 80: mad r0.xyz, cb0[16].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[16].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 81: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 82: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 84: mad_sat r1.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 85: mad r0.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 86: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 87: mad r7.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r7.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 88: mul r2.x, r2.x, cb0[17].y
    r2.x = ((r2.xxxx)*(source[17].yyyy)).x;
    // 89: mul r2.y, r2.y, cb0[16].w
    r2.y = ((r2.yyyy)*(source[16].wwww)).y;
    // 90: log r2.z, |r2.x|
    r2.z = (log2(abs(r2.xxxx))).z;
    // 91: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 92: mul r2.z, r2.z, cb0[17].z
    r2.z = ((r2.zzzz)*(source[17].zzzz)).z;
    // 93: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 94: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 96: mad r5.xyz, r2.xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 97: mad r0.xyz, r5.xyzx, r2.xxxx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r2.xxxx)+(r0.xyzx)).xyz;
    // 98: mul r0.xyz, r2.xxxx, r0.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)).xyz;
    // 99: max r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = (max(r0.xyzx,r2.xxxx)).xyz;
    // 100: dp2 r2.z, r3.xyxx, r3.xyxx
    r2.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 101: mul r5.xy, r3.xyxx, cb0[9].zzzz
    r5.xy = ((r3.xyxx)*(source[9].zzzz)).xy;
    // 102: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 103: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 104: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 105: add r5.z, r2.z, l(0.000010)
    r5.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 106: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 107: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 108: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 110: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 111: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 112: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 113: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 114: dp3 r0.w, r7.xyzx, r5.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 115: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 116: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 117: mul r7.xyz, r2.wwww, cb0[29].xyzx
    r7.xyz = ((r2.wwww)*(source[29].xyzx)).xyz;
    // 118: mad r7.xyz, r2.zzzz, cb0[28].xyzx, r7.xyzx
    r7.xyz = ((r2.zzzz)*(source[28].xyzx)+(r7.xyzx)).xyz;
    // 119: mul r7.xyz, r7.xyzx, cb0[30].wwww
    r7.xyz = ((r7.xyzx)*(source[30].wwww)).xyz;
    // 120: mul r10.xyz, r1.xyzx, r7.xyzx
    r10.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 121: dp2_sat r11.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r11.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 122: dp3_sat r11.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r11.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 123: dp3_sat r11.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r11.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 124: mul r11.xyz, r11.xyzx, r11.xyzx
    r11.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 125: sample_indexable(texture2d)(float,float,float,float) r12.xyz, v3.zwzz, t8.xyzw, s5
    r12.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 126: mul r12.xyz, r12.xyzx, cb0[32].xyzx
    r12.xyz = ((r12.xyzx)*(source[32].xyzx)).xyz;
    // 127: dp3 r0.w, r12.xyzx, r11.xyzx
    r0.w = (dot((r12.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 128: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t7.xyzw, s5
    r11.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 129: mul r11.xyz, r11.xyzx, cb0[31].xyzx
    r11.xyz = ((r11.xyzx)*(source[31].xyzx)).xyz;
    // 130: mul r13.xyz, r0.wwww, r11.xyzx
    r13.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 131: mad r10.xyz, r1.xyzx, r13.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r13.xyzx)+(r10.xyzx)).xyz;
    // 132: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 133: dp3 r10.x, r6.xyzx, r5.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 134: dp3 r10.y, r9.xyzx, r5.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 135: dp2 r13.z, r10.xyxx, cb0[19].xyxx
    r13.z = (dot((r10.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 136: dp3 r13.y, r8.xyzx, r5.xyzx
    r13.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 137: mul r2.zw, cb0[19].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r2.zw = ((source[19].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 138: dp2 r13.x, r10.xyxx, r2.zwzz
    r13.x = (dot((r10.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 139: mov r13.w, l(1.000000)
    r13.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 140: dp4 r14.x, cb0[20].xyzw, r13.xyzw
    r14.x = (dot((source[20].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 141: dp4 r14.y, cb0[21].xyzw, r13.xyzw
    r14.y = (dot((source[21].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 142: dp4 r14.z, cb0[22].xyzw, r13.xyzw
    r14.z = (dot((source[22].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 143: mul r15.xyzw, r13.yzzx, r13.xyzz
    r15.xyzw = ((r13.yzzx)*(r13.xyzz)).xyzw;
    // 144: dp4 r16.x, cb0[23].xyzw, r15.xyzw
    r16.x = (dot((source[23].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 145: dp4 r16.y, cb0[24].xyzw, r15.xyzw
    r16.y = (dot((source[24].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 146: dp4 r16.z, cb0[25].xyzw, r15.xyzw
    r16.z = (dot((source[25].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 147: add r14.xyz, r14.xyzx, r16.xyzx
    r14.xyz = ((r14.xyzx)+(r16.xyzx)).xyz;
    // 148: mul r3.x, r13.y, r13.y
    r3.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 149: mov r10.z, r13.y
    r10.z = (r13.yyyy).z;
    // 150: mad r3.x, r13.x, r13.x, -r3.x
    r3.x = ((r13.xxxx)*(r13.xxxx)+(-(r3.xxxx))).x;
    // 151: mad r13.xyz, cb0[26].xyzx, r3.xxxx, r14.xyzx
    r13.xyz = ((source[26].xyzx)*(r3.xxxx)+(r14.xyzx)).xyz;
    // 152: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 153: mul r13.xyz, r13.xyzx, cb0[18].xyzx
    r13.xyz = ((r13.xyzx)*(source[18].xyzx)).xyz;
    // 154: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[18].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[18].wwww)).xyz;
    // 155: mov_sat r1.w, cb0[16].y
    r1.w = (saturate(source[16].yyyy)).w;
    // 156: mad r14.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r14.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 157: mul r3.x, r1.w, l(0.080000)
    r3.x = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 158: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 159: mad r14.xyz, r3.wwww, r14.xyzx, r3.xxxx
    r14.xyz = ((r3.wwww)*(r14.xyzx)+(r3.xxxx)).xyz;
    // 160: mul_sat r1.w, r14.y, l(50.000000)
    r1.w = (saturate((r14.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 161: log r3.x, |r2.y|
    r3.x = (log2(abs(r2.yyyy))).x;
    // 162: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 163: mul r3.x, r3.x, cb0[17].x
    r3.x = ((r3.xxxx)*(source[17].xxxx)).x;
    // 164: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 165: movc r2.y, r2.y, l(0), r3.x
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xxxx)).y;
    // 166: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 167: min r3.z, r2.y, l(1.000000)
    r3.z = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 168: dp3 r2.y, v5.xyzx, v5.xyzx
    r2.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 169: rsq r2.y, r2.y
    r2.y = (rsqrt(r2.yyyy)).y;
    // 170: mul r15.xyz, r2.yyyy, v5.xyzx
    r15.xyz = ((r2.yyyy)*(v5.xyzx)).xyz;
    // 171: dp3 r2.y, r5.xyzx, r15.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r15.xyzx).xyz).xxxx).y;
    // 172: mul r5.xyz, r2.yyyy, r5.xyzx
    r5.xyz = ((r2.yyyy)*(r5.xyzx)).xyz;
    // 173: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r15.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r15.xyzx))).xyz;
    // 174: deriv_rtx_coarse r3.x, r2.y
    r3.x = (ddx_coarse(r2.yyyy)).x;
    // 175: deriv_rty_coarse r3.y, r2.y
    r3.y = (ddy_coarse(r2.yyyy)).y;
    // 176: add r2.y, r2.y, l(1.000000)
    r2.y = ((r2.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: dp2 r3.x, r3.xyxx, r3.xyxx
    r3.x = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 178: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 179: mad_sat r3.y, r3.x, l(0.300000), r3.z
    r3.y = (saturate((r3.xxxx)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 180: add r4.w, -r3.y, l(1.000000)
    r4.w = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 181: max r16.xyz, r14.xyzx, r4.wwww
    r16.xyz = (max(r14.xyzx,r4.wwww)).xyz;
    // 182: add r16.xyz, -r14.xyzx, r16.xyzx
    r16.xyz = ((-(r14.xyzx))+(r16.xyzx)).xyz;
    // 183: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 184: add r1.w, r5.z, l(1.000000)
    r1.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 186: add_sat r3.x, -r1.w, r2.y
    r3.x = (saturate((-(r1.wwww))+(r2.yyyy))).x;
    // 187: sample_indexable(texture2d)(float,float,float,float) r17.xy, r3.xyxx, t5.xyzw, s7
    r17.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 188: add r1.w, r2.x, r3.x
    r1.w = ((r2.xxxx)+(r3.xxxx)).w;
    // 189: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 190: mul r18.xyz, r14.xyzx, r17.yyyy
    r18.xyz = ((r14.xyzx)*(r17.yyyy)).xyz;
    // 191: mad r16.xyz, r16.xyzx, r17.xxxx, r18.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xxxx)+(r18.xyzx)).xyz;
    // 192: div r2.y, l(1.000000, 1.000000, 1.000000, 1.000000), r17.y
    r2.y = r17.y != 0.f ? 1.f / r17.y : 0.f;
    // 193: add r2.y, r2.y, l(-1.000000)
    r2.y = ((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 194: mad r17.xyz, r14.xyzx, r2.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r14.xyzx)*(r2.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 195: dp3 r2.y, r14.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r14.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 196: mad r14.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r14.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 197: mad r18.xyz, -r16.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 198: mul r16.xyz, r16.xyzx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xyzx)).xyz;
    // 199: mul r13.xyz, r13.xyzx, r18.xyzx
    r13.xyz = ((r13.xyzx)*(r18.xyzx)).xyz;
    // 200: mul r0.xyz, r0.xyzx, r13.xyzx
    r0.xyz = ((r0.xyzx)*(r13.xyzx)).xyz;
    // 201: mad r0.xyz, -r0.xyzx, r3.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r3.wwww)+(r0.xyzx)).xyz;
    // 202: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 203: dp3 r6.x, r6.xyzx, r5.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 204: dp3 r6.y, r9.xyzx, r5.xyzx
    r6.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 205: dp2 r9.x, r6.xyxx, r2.zwzz
    r9.x = (dot((r6.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 206: dp2 r9.z, r6.xyxx, cb0[19].xyxx
    r9.z = (dot((r6.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 207: mul r2.y, r3.y, l(5.000000)
    r2.y = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).y;
    // 208: mul r2.z, r3.y, r3.y
    r2.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 209: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 210: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 211: add r1.w, r2.x, r1.w
    r1.w = ((r2.xxxx)+(r1.wwww)).w;
    // 212: mov o5.y, r2.x
    output.targets[5].y = (r2.xxxx).y;
    // 213: add_sat r1.w, r1.w, l(-1.000000)
    r1.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 214: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 215: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r9.xyzx, t6.xyzw, s6, r2.y
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r2.yyyy).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 216: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 217: mul r2.xyz, r2.xyzx, cb0[18].xyzx
    r2.xyz = ((r2.xyzx)*(source[18].xyzx)).xyz;
    // 218: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[18].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[18].wwww)).xyz;
    // 219: dp2_sat r6.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 220: dp3_sat r6.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 221: dp3_sat r6.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 222: mul r3.xyw, r6.xyxz, r6.xyxz
    r3.xyw = ((r6.xyxz)*(r6.xyxz)).xyw;
    // 223: dp3 r2.w, r12.xyzx, r3.xywx
    r2.w = (dot((r12.xyzx).xyz,(r3.xywx).xyz).xxxx).w;
    // 224: add r0.w, r0.w, -r2.w
    r0.w = ((r0.wwww)+(-(r2.wwww))).w;
    // 225: mad r0.w, r3.z, r0.w, r2.w
    r0.w = ((r3.zzzz)*(r0.wwww)+(r2.wwww)).w;
    // 226: mad r3.xyz, r11.xyzx, r0.wwww, r7.xyzx
    r3.xyz = ((r11.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 227: mul r5.xyz, r0.wwww, r11.xyzx
    r5.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 228: mad r0.w, r1.w, r14.x, r14.y
    r0.w = ((r1.wwww)*(r14.xxxx)+(r14.yyyy)).w;
    // 229: mad r0.w, r0.w, r1.w, r14.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r14.zzzz)).w;
    // 230: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 231: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 232: mul r6.xyz, r0.wwww, r3.xyzx
    r6.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 233: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 234: div r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)/(r3.xyzx)).xyz;
    // 235: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 236: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 237: mad r0.xyz, r2.xyzx, r16.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r16.xyzx)+(r0.xyzx)).xyz;
    // 238: mul r2.xyz, r16.xyzx, r2.xyzx
    r2.xyz = ((r16.xyzx)*(r2.xyzx)).xyz;
    // 239: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 240: dp3 r1.w, r4.xyzx, r15.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r15.xyzx).xyz).xxxx).w;
    // 241: add r2.x, -|r15.z|, l(1.000000)
    r2.x = ((-(abs(r15.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 242: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 243: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 244: log r2.x, |r1.w|
    r2.x = (log2(abs(r1.wwww))).x;
    // 245: mul r2.x, r2.x, l(1.500000)
    r2.x = ((r2.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 246: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 247: mul r2.xyz, r2.xxxx, cb0[3].xyzx
    r2.xyz = ((r2.xxxx)*(source[3].xyzx)).xyz;
    // 248: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 249: movc r2.xyz, r2.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 250: mad_sat r2.w, r1.w, cb0[11].y, -cb0[11].z
    r2.w = (saturate((r1.wwww)*(source[11].yyyy)+(-(source[11].zzzz)))).w;
    // 251: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 252: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 253: mul r3.x, r3.x, cb0[11].w
    r3.x = ((r3.xxxx)*(source[11].wwww)).x;
    // 254: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 255: mul r3.xyz, r3.xxxx, cb0[4].xyzx
    r3.xyz = ((r3.xxxx)*(source[4].xyzx)).xyz;
    // 256: movc r3.xyz, r2.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 257: add r3.xyz, r3.xyzx, -cb0[4].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[4].xyzx))).xyz;
    // 258: mad r3.xyz, cb0[4].wwww, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((source[4].wwww)*(r3.xyzx)+(source[4].xyzx)).xyz;
    // 259: mul r4.xyz, cb0[5].xyzx, cb0[12].yyyy
    r4.xyz = ((source[5].xyzx)*(source[12].yyyy)).xyz;
    // 260: mul r4.xyz, r4.xyzx, cb0[13].xxxx
    r4.xyz = ((r4.xyzx)*(source[13].xxxx)).xyz;
    // 261: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 262: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 263: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 264: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 265: mul r4.xyz, r4.xyzx, cb0[13].yyyy
    r4.xyz = ((r4.xyzx)*(source[13].yyyy)).xyz;
    // 266: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 267: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 268: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 269: mad r2.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xyzx)).xyz;
    // 270: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 271: add r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 272: mad o0.xyz, r1.xyzx, cb0[30].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[30].xyzx)+(r2.xyzx)).xyz;
    // 273: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 274: dp3 r1.x, r10.xyzx, r10.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 275: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 276: mul r1.xyz, r1.xxxx, r10.xyzx
    r1.xyz = ((r1.xxxx)*(r10.xyzx)).xyz;
    // 277: ge r1.w, l(0.000000), r1.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r1.zzzz)) * 0xffffffffu)).w;
    // 278: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r1.xyzx|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r1.xyzx)).xyz).xxxx).z;
    // 279: div r1.xy, r1.xyxx, r1.zzzz
    r1.xy = ((r1.xyxx)/(r1.zzzz)).xy;
    // 280: ge r2.xy, r1.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r1.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 281: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 282: mad r2.xy, -|r1.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r1.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 283: movc r1.xy, r1.wwww, r2.xyxx, r1.xyxx
    r1.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r1.xyxx)).xy;
    // 284: mad o2.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 285: mul o4.z, r0.w, r0.x
    output.targets[4].z = ((r0.wwww)*(r0.xxxx)).z;
    // 286: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 287: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 288: ftou r0.x, cb0[27].z
    r0.x = (asfloat((uint4)(source[27].zzzz))).x;
    // 289: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 290: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 291: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 292: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 293: ret
    return output;
}

// source.character.static-map-native-1161.v1 / source program cb745af6f7582240baee5a8a7f9abe7b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1161(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1161(input);
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
    source[12]=g_SourceCharacterBaseConstants[13];
    source[12].z=(g_SourceCharacterTime.xxxx).x;
    source[13]=g_SourceCharacterBaseConstants[15];
    source[13].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[14]=g_SourceCharacterBaseConstants[16];
    source[15]=g_SourceCharacterBaseConstants[17];
    source[16]=g_SourceCharacterBaseConstants[18];
    source[17]=g_SourceCharacterBaseConstants[19];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[18]=g_SourceCharacterEnvironmentColor;source[19]=g_SourceCharacterEnvironmentRotation;}
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
    // 8: mad r1.xyz, cb0[13].wwww, r1.xyzx, r0.xyzx
    r1.xyz = ((source[13].wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 9: mul r2.xyz, cb0[6].xyzx, cb0[14].xxxx
    r2.xyz = ((source[6].xyzx)*(source[14].xxxx)).xyz;
    // 10: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 11: mul r2.xyz, cb0[7].xyzx, cb0[14].yyyy
    r2.xyz = ((source[7].xyzx)*(source[14].yyyy)).xyz;
    // 12: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 14: mul r1.w, r2.z, cb0[14].z
    r1.w = ((r2.zzzz)*(source[14].zzzz)).w;
    // 15: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 16: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 17: mul r2.z, r2.z, cb0[14].w
    r2.z = ((r2.zzzz)*(source[14].wwww)).z;
    // 18: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 19: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 20: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 21: mul_sat r3.w, r1.w, cb2[3].w
    r3.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 22: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, cb0[8].xyzx, cb0[15].xxxx
    r1.xyz = ((source[8].xyzx)*(source[15].xxxx)).xyz;
    // 24: mul r3.xy, v4.xyxx, cb0[9].yyyy
    r3.xy = ((v4.xyxx)*(source[9].yyyy)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 27: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 28: mul r5.xyz, r1.xyzx, r4.xyzx
    r5.xyz = ((r1.xyzx)*(r4.xyzx)).xyz;
    // 29: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 30: mad r1.xyz, -r1.xyzx, r4.xyzx, r1.wwww
    r1.xyz = ((-(r1.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 31: mul r1.w, r4.w, r4.w
    r1.w = ((r4.wwww)*(r4.wwww)).w;
    // 32: mad r1.xyz, cb0[15].zzzz, r1.xyzx, r5.xyzx
    r1.xyz = ((source[15].zzzz)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 33: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 35: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 36: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 37: mul r4.xy, r4.xyxx, cb0[9].xxxx
    r4.xy = ((r4.xyxx)*(source[9].xxxx)).xy;
    // 38: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 39: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 42: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 43: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 44: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 45: div r4.xyz, r4.xyzx, r2.wwww
    r4.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 46: mul r2.w, r4.z, r4.z
    r2.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 47: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 48: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 50: max r1.w, cb0[9].w, l(0.000000)
    r1.w = (max(source[9].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 51: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 52: mul r2.w, r0.w, r1.w
    r2.w = ((r0.wwww)*(r1.wwww)).w;
    // 53: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 54: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 55: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 56: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 57: mul r6.xyz, r4.wwww, v0.xyzx
    r6.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 58: dp3 r7.x, r6.xyzx, r4.xyzx
    r7.x = (dot((r6.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 59: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 60: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 61: mul r8.xyz, r4.wwww, v1.xyzx
    r8.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 62: dp3 r7.z, r8.xyzx, r4.xyzx
    r7.z = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 63: mul r9.xyz, r6.yzxy, r8.zxyz
    r9.xyz = ((r6.yzxy)*(r8.zxyz)).xyz;
    // 64: mad r9.xyz, r8.yzxy, r6.zxyz, -r9.xyzx
    r9.xyz = ((r8.yzxy)*(r6.zxyz)+(-(r9.xyzx))).xyz;
    // 65: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 66: dp3 r7.y, r9.xyzx, r4.xyzx
    r7.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 67: dp3 r4.w, r7.xyzx, r5.xyzx
    r4.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 68: add r4.w, r4.w, l(1.000000)
    r4.w = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mad r4.w, r4.w, l(0.500000), cb0[10].z
    r4.w = ((r4.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].zzzz)).w;
    // 70: mad r2.w, r4.w, r2.w, r4.w
    r2.w = ((r4.wwww)*(r2.wwww)+(r4.wwww)).w;
    // 71: add r4.w, -r1.w, r2.w
    r4.w = ((-(r1.wwww))+(r2.wwww)).w;
    // 72: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 74: mad r2.w, -r1.w, r4.w, r2.w
    r2.w = ((-(r1.wwww))*(r4.wwww)+(r2.wwww)).w;
    // 75: mul r1.w, r4.w, r1.w
    r1.w = ((r4.wwww)*(r1.wwww)).w;
    // 76: mad_sat r0.w, r0.w, r2.w, r1.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r1.wwww))).w;
    // 77: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 78: mul r0.w, r0.w, l(0.650000)
    r0.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 79: mul r1.xyz, r0.xyzx, cb0[15].wwww
    r1.xyz = ((r0.xyzx)*(source[15].wwww)).xyz;
    // 80: mad r0.xyz, cb0[16].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[16].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 81: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 82: add r1.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 84: mad_sat r1.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 85: mad r0.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r0.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 86: mad r5.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r5.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 87: mad r7.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r7.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 88: mul r2.x, r2.x, cb0[17].y
    r2.x = ((r2.xxxx)*(source[17].yyyy)).x;
    // 89: mul r2.y, r2.y, cb0[16].w
    r2.y = ((r2.yyyy)*(source[16].wwww)).y;
    // 90: log r2.z, |r2.x|
    r2.z = (log2(abs(r2.xxxx))).z;
    // 91: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 92: mul r2.z, r2.z, cb0[17].z
    r2.z = ((r2.zzzz)*(source[17].zzzz)).z;
    // 93: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 94: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 95: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 96: mad r5.xyz, r2.xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((r2.xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 97: mad r0.xyz, r5.xyzx, r2.xxxx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r2.xxxx)+(r0.xyzx)).xyz;
    // 98: mul r0.xyz, r2.xxxx, r0.xyzx
    r0.xyz = ((r2.xxxx)*(r0.xyzx)).xyz;
    // 99: max r0.xyz, r0.xyzx, r2.xxxx
    r0.xyz = (max(r0.xyzx,r2.xxxx)).xyz;
    // 100: dp2 r2.z, r3.xyxx, r3.xyxx
    r2.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 101: mul r5.xy, r3.xyxx, cb0[9].zzzz
    r5.xy = ((r3.xyxx)*(source[9].zzzz)).xy;
    // 102: add r2.z, -r2.z, l(1.000000)
    r2.z = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 103: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 104: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 105: add r5.z, r2.z, l(0.000010)
    r5.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 106: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 107: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 108: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 109: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 110: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 111: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 112: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 113: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 114: dp3 r0.w, r7.xyzx, r5.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 115: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 116: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 117: mul r10.xyz, r2.wwww, cb0[29].xyzx
    r10.xyz = ((r2.wwww)*(source[29].xyzx)).xyz;
    // 118: mad r10.xyz, r2.zzzz, cb0[28].xyzx, r10.xyzx
    r10.xyz = ((r2.zzzz)*(source[28].xyzx)+(r10.xyzx)).xyz;
    // 119: mul r10.xyz, r10.xyzx, cb0[30].wwww
    r10.xyz = ((r10.xyzx)*(source[30].wwww)).xyz;
    // 120: mul r10.xyz, r1.xyzx, r10.xyzx
    r10.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 121: mul r0.xyz, r0.xyzx, r10.xyzx
    r0.xyz = ((r0.xyzx)*(r10.xyzx)).xyz;
    // 122: dp3 r10.x, r6.xyzx, r5.xyzx
    r10.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 123: dp3 r10.y, r9.xyzx, r5.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 124: dp2 r11.z, r10.xyxx, cb0[19].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 125: dp3 r11.y, r8.xyzx, r5.xyzx
    r11.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 126: mul r2.zw, cb0[19].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r2.zw = ((source[19].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 127: dp2 r11.x, r10.xyxx, r2.zwzz
    r11.x = (dot((r10.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 128: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 129: dp4 r12.x, cb0[20].xyzw, r11.xyzw
    r12.x = (dot((source[20].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 130: dp4 r12.y, cb0[21].xyzw, r11.xyzw
    r12.y = (dot((source[21].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 131: dp4 r12.z, cb0[22].xyzw, r11.xyzw
    r12.z = (dot((source[22].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 132: mul r13.xyzw, r11.yzzx, r11.xyzz
    r13.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 133: dp4 r14.x, cb0[23].xyzw, r13.xyzw
    r14.x = (dot((source[23].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 134: dp4 r14.y, cb0[24].xyzw, r13.xyzw
    r14.y = (dot((source[24].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 135: dp4 r14.z, cb0[25].xyzw, r13.xyzw
    r14.z = (dot((source[25].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 136: add r12.xyz, r12.xyzx, r14.xyzx
    r12.xyz = ((r12.xyzx)+(r14.xyzx)).xyz;
    // 137: mul r0.w, r11.y, r11.y
    r0.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 138: mov r10.z, r11.y
    r10.z = (r11.yyyy).z;
    // 139: mad r0.w, r11.x, r11.x, -r0.w
    r0.w = ((r11.xxxx)*(r11.xxxx)+(-(r0.wwww))).w;
    // 140: mad r11.xyz, cb0[26].xyzx, r0.wwww, r12.xyzx
    r11.xyz = ((source[26].xyzx)*(r0.wwww)+(r12.xyzx)).xyz;
    // 141: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 142: mul r11.xyz, r11.xyzx, cb0[18].xyzx
    r11.xyz = ((r11.xyzx)*(source[18].xyzx)).xyz;
    // 143: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[18].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[18].wwww)).xyz;
    // 144: mov_sat r1.w, cb0[16].y
    r1.w = (saturate(source[16].yyyy)).w;
    // 145: mad r12.xyz, -r1.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r12.xyz = ((-(r1.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 146: mul r0.w, r1.w, l(0.080000)
    r0.w = ((r1.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 147: mov o3.xyzw, r1.xyzw
    output.targets[3].xyzw = (r1.xyzw).xyzw;
    // 148: mad r12.xyz, r3.wwww, r12.xyzx, r0.wwww
    r12.xyz = ((r3.wwww)*(r12.xyzx)+(r0.wwww)).xyz;
    // 149: mul_sat r0.w, r12.y, l(50.000000)
    r0.w = (saturate((r12.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 150: log r1.w, |r2.y|
    r1.w = (log2(abs(r2.yyyy))).w;
    // 151: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 152: mul r1.w, r1.w, cb0[17].x
    r1.w = ((r1.wwww)*(source[17].xxxx)).w;
    // 153: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 154: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 155: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 156: min r3.z, r1.w, l(1.000000)
    r3.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 157: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 158: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 159: mul r13.xyz, r1.wwww, v5.xyzx
    r13.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 160: dp3 r1.w, r5.xyzx, r13.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 161: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 162: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r13.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r13.xyzx))).xyz;
    // 163: deriv_rtx_coarse r3.x, r1.w
    r3.x = (ddx_coarse(r1.wwww)).x;
    // 164: deriv_rty_coarse r3.y, r1.w
    r3.y = (ddy_coarse(r1.wwww)).y;
    // 165: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: dp2 r2.y, r3.xyxx, r3.xyxx
    r2.y = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 167: sqrt r2.y, r2.y
    r2.y = (sqrt(r2.yyyy)).y;
    // 168: mad_sat r3.y, r2.y, l(0.300000), r3.z
    r3.y = (saturate((r2.yyyy)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.zzzz))).y;
    // 169: mov o2.zw, r3.zzzw
    output.targets[2].zw = (r3.zzzw).zw;
    // 170: add r2.y, -r3.y, l(1.000000)
    r2.y = ((-(r3.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 171: max r14.xyz, r12.xyzx, r2.yyyy
    r14.xyz = (max(r12.xyzx,r2.yyyy)).xyz;
    // 172: add r14.xyz, -r12.xyzx, r14.xyzx
    r14.xyz = ((-(r12.xyzx))+(r14.xyzx)).xyz;
    // 173: mul r14.xyz, r0.wwww, r14.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)).xyz;
    // 174: add r0.w, r5.z, l(1.000000)
    r0.w = ((r5.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 175: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 176: add_sat r3.x, -r0.w, r1.w
    r3.x = (saturate((-(r0.wwww))+(r1.wwww))).x;
    // 177: sample_indexable(texture2d)(float,float,float,float) r15.xy, r3.xyxx, t5.xyzw, s6
    r15.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 178: add r0.w, r2.x, r3.x
    r0.w = ((r2.xxxx)+(r3.xxxx)).w;
    // 179: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 180: mul r16.xyz, r12.xyzx, r15.yyyy
    r16.xyz = ((r12.xyzx)*(r15.yyyy)).xyz;
    // 181: mad r14.xyz, r14.xyzx, r15.xxxx, r16.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xxxx)+(r16.xyzx)).xyz;
    // 182: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.y
    r1.w = r15.y != 0.f ? 1.f / r15.y : 0.f;
    // 183: add r1.w, r1.w, l(-1.000000)
    r1.w = ((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 184: mad r15.xyz, r12.xyzx, r1.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((r12.xyzx)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: dp3 r1.w, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 186: mad r12.xyz, r1.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r12.xyz = ((r1.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 187: mad r16.xyz, -r14.xyzx, r15.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xyzx))*(r15.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mul r14.xyz, r14.xyzx, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r15.xyzx)).xyz;
    // 189: mul r11.xyz, r11.xyzx, r16.xyzx
    r11.xyz = ((r11.xyzx)*(r16.xyzx)).xyz;
    // 190: mul r0.xyz, r0.xyzx, r11.xyzx
    r0.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 191: mad r0.xyz, -r0.xyzx, r3.wwww, r0.xyzx
    r0.xyz = ((-(r0.xyzx))*(r3.wwww)+(r0.xyzx)).xyz;
    // 192: dp3 r6.x, r6.xyzx, r5.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 193: dp3 r6.y, r9.xyzx, r5.xyzx
    r6.y = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 194: dp2 r9.x, r6.xyxx, r2.zwzz
    r9.x = (dot((r6.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 195: dp2 r9.z, r6.xyxx, cb0[19].xyxx
    r9.z = (dot((r6.xyxx).xy,(source[19].xyxx).xy).xxxx).z;
    // 196: mul r1.w, r3.y, l(5.000000)
    r1.w = ((r3.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 197: mul r2.y, r3.y, r3.y
    r2.y = ((r3.yyyy)*(r3.yyyy)).y;
    // 198: mul r0.w, r0.w, r2.y
    r0.w = ((r0.wwww)*(r2.yyyy)).w;
    // 199: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 200: add r0.w, r2.x, r0.w
    r0.w = ((r2.xxxx)+(r0.wwww)).w;
    // 201: mov o5.y, r2.x
    output.targets[5].y = (r2.xxxx).y;
    // 202: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 203: dp3 r9.y, r8.xyzx, r5.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 204: dp3 r2.x, r7.xyzx, r5.xyzx
    r2.x = (dot((r7.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 205: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 206: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 207: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r9.xyzx, t6.xyzw, s5, r1.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 208: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 209: mul r3.xyz, r3.xyzx, cb0[18].xyzx
    r3.xyz = ((r3.xyzx)*(source[18].xyzx)).xyz;
    // 210: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[18].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[18].wwww)).xyz;
    // 211: mad r1.w, r0.w, r12.x, r12.y
    r1.w = ((r0.wwww)*(r12.xxxx)+(r12.yyyy)).w;
    // 212: mad r1.w, r1.w, r0.w, r12.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r12.zzzz)).w;
    // 213: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 214: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 215: mul r2.yzw, r2.yyyy, cb0[29].xxyz
    r2.yzw = ((r2.yyyy)*(source[29].xxyz)).yzw;
    // 216: mad r2.xyz, cb0[28].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[28].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 217: mul r2.xyz, r2.xyzx, cb0[30].wwww
    r2.xyz = ((r2.xyzx)*(source[30].wwww)).xyz;
    // 218: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 219: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 220: mad r0.xyz, r2.xyzx, r14.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r14.xyzx)+(r0.xyzx)).xyz;
    // 221: mul r2.xyz, r14.xyzx, r2.xyzx
    r2.xyz = ((r14.xyzx)*(r2.xyzx)).xyz;
    // 222: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 223: dp3 r0.w, r4.xyzx, r13.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 224: add r1.w, -|r13.z|, l(1.000000)
    r1.w = ((-(abs(r13.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 225: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 226: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 227: log r1.w, |r0.w|
    r1.w = (log2(abs(r0.wwww))).w;
    // 228: mul r1.w, r1.w, l(1.500000)
    r1.w = ((r1.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 229: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 230: mul r2.xyz, r1.wwww, cb0[3].xyzx
    r2.xyz = ((r1.wwww)*(source[3].xyzx)).xyz;
    // 231: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 232: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 233: mad_sat r1.w, r0.w, cb0[11].y, -cb0[11].z
    r1.w = (saturate((r0.wwww)*(source[11].yyyy)+(-(source[11].zzzz)))).w;
    // 234: log r2.w, r1.w
    r2.w = (log2(r1.wwww)).w;
    // 235: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 236: mul r2.w, r2.w, cb0[11].w
    r2.w = ((r2.wwww)*(source[11].wwww)).w;
    // 237: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 238: mul r3.xyz, r2.wwww, cb0[4].xyzx
    r3.xyz = ((r2.wwww)*(source[4].xyzx)).xyz;
    // 239: movc r3.xyz, r1.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 240: add r3.xyz, r3.xyzx, -cb0[4].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[4].xyzx))).xyz;
    // 241: mad r3.xyz, cb0[4].wwww, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((source[4].wwww)*(r3.xyzx)+(source[4].xyzx)).xyz;
    // 242: mul r4.xyz, cb0[5].xyzx, cb0[12].yyyy
    r4.xyz = ((source[5].xyzx)*(source[12].yyyy)).xyz;
    // 243: mul r4.xyz, r4.xyzx, cb0[13].xxxx
    r4.xyz = ((r4.xyzx)*(source[13].xxxx)).xyz;
    // 244: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 245: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 246: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 247: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 248: mul r4.xyz, r4.xyzx, cb0[13].yyyy
    r4.xyz = ((r4.xyzx)*(source[13].yyyy)).xyz;
    // 249: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 250: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 251: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 252: mad r2.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r2.xyzx
    r2.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r2.xyzx)).xyz;
    // 253: add r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 254: add r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 255: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 256: mad o0.xyz, r1.xyzx, cb0[30].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[30].xyzx)+(r2.xyzx)).xyz;
    // 257: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 258: dp3 r0.x, r10.xyzx, r10.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 259: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 260: mul r0.xyz, r0.xxxx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r10.xyzx)).xyz;
    // 261: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 262: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 263: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 264: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 265: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 266: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 267: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 268: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 269: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 270: ftou r0.x, cb0[27].z
    r0.x = (asfloat((uint4)(source[27].zzzz))).x;
    // 271: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 272: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 273: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 274: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 275: ret
    return output;
}

// source.character.static-map-native-1162.v1 / source program ad7db1f9b4d51d4786938fa20d4ee027
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1162(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 50: mul r7.xyz, cb0[5].xyzx, cb0[8].xxxx
    r7.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 51: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 52: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 54: mad r6.xyz, cb0[8].zzzz, r6.xyzx, r8.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[4].xyzx, cb0[7].wwww
    r7.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 56: mad r6.xyz, -r3.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 57: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 58: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 59: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 60: mul r6.xyz, r3.xyzx, cb0[8].wwww
    r6.xyz = ((r3.xyzx)*(source[8].wwww)).xyz;
    // 61: mad r3.xyz, cb0[9].xxxx, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 63: mul r0.y, r7.z, cb0[9].y
    r0.y = ((r7.zzzz)*(source[9].yyyy)).y;
    // 64: mul r7.xy, r7.yxyy, cb0[10].ywyy
    r7.xy = ((r7.yxyy)*(source[10].ywyy)).xy;
    // 65: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 66: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 67: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 68: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 69: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 70: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 72: mad r3.xyz, r1.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 73: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 75: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 76: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 77: mul r6.xy, r0.zwzz, cb0[6].zzzz
    r6.xy = ((r0.zwzz)*(source[6].zzzz)).xy;
    // 78: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 79: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 80: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 81: add r6.z, r0.y, l(0.000010)
    r6.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 82: add r0.yzw, -r1.xxyz, r6.xxyz
    r0.yzw = ((-(r1.xxyz))+(r6.xxyz)).yzw;
    // 83: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 84: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 85: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 86: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 88: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 89: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 90: dp3 r0.w, r6.xyzx, r1.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 91: mad r6.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 93: mul r6.yzw, r6.yyyy, cb0[23].xxyz
    r6.yzw = ((r6.yyyy)*(source[23].xxyz)).yzw;
    // 94: mad r6.xyz, r6.xxxx, cb0[22].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[22].xyzx)+(r6.yzwy)).xyz;
    // 95: mul r6.xyz, r6.xyzx, cb0[24].wwww
    r6.xyz = ((r6.xyzx)*(source[24].wwww)).xyz;
    // 96: mul r8.xyz, r3.xyzx, r6.xyzx
    r8.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 97: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 98: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 99: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 100: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 101: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t8.xyzw, s5
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 102: mul r10.xyz, r10.xyzx, cb0[26].xyzx
    r10.xyz = ((r10.xyzx)*(source[26].xyzx)).xyz;
    // 103: dp3 r0.w, r10.xyzx, r9.xyzx
    r0.w = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 104: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t7.xyzw, s5
    r9.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[25].xyzx
    r9.xyz = ((r9.xyzx)*(source[25].xyzx)).xyz;
    // 106: mul r11.xyz, r0.wwww, r9.xyzx
    r11.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 107: mad r8.xyz, r3.xyzx, r11.xyzx, r8.xyzx
    r8.xyz = ((r3.xyzx)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 108: mad r11.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r11.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 109: mad r12.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r12.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 110: mad r13.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r13.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 111: log r14.xy, |r7.xyxx|
    r14.xy = (log2(abs(r7.xyxx))).xy;
    // 112: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 113: mul r1.w, r14.y, cb0[11].x
    r1.w = ((r14.yyyy)*(source[11].xxxx)).w;
    // 114: mul r2.w, r14.x, cb0[10].z
    r2.w = ((r14.xxxx)*(source[10].zzzz)).w;
    // 115: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 116: movc r2.w, r7.x, l(0), r2.w
    r2.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 117: max r2.w, r2.w, cb0[0].x
    r2.w = (max(r2.wwww,source[0].xxxx)).w;
    // 118: min r7.z, r2.w, l(1.000000)
    r7.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 119: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 120: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: movc r1.w, r7.y, l(0), r1.w
    r1.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 122: mad r12.xyz, r1.wwww, r12.xyzx, r13.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)+(r13.xyzx)).xyz;
    // 123: mad r11.xyz, r12.xyzx, r1.wwww, r11.xyzx
    r11.xyz = ((r12.xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 124: mul r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = ((r1.wwww)*(r11.xyzx)).xyz;
    // 125: max r11.xyz, r1.wwww, r11.xyzx
    r11.xyz = (max(r1.wwww,r11.xyzx)).xyz;
    // 126: mul r8.xyz, r8.xyzx, r11.xyzx
    r8.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 127: dp3 r11.x, r2.xyzx, r1.xyzx
    r11.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 128: dp3 r11.y, r5.xyzx, r1.xyzx
    r11.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 129: dp2 r12.z, r11.xyxx, cb0[13].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 130: dp3 r12.y, r4.xyzx, r1.xyzx
    r12.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 131: mul r7.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 132: dp2 r12.x, r11.xyxx, r7.xyxx
    r12.x = (dot((r11.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 133: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 134: dp4 r13.x, cb0[14].xyzw, r12.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 135: dp4 r13.y, cb0[15].xyzw, r12.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 136: dp4 r13.z, cb0[16].xyzw, r12.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 137: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 138: dp4 r15.x, cb0[17].xyzw, r14.xyzw
    r15.x = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 139: dp4 r15.y, cb0[18].xyzw, r14.xyzw
    r15.y = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 140: dp4 r15.z, cb0[19].xyzw, r14.xyzw
    r15.z = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 141: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 142: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 143: mov r11.z, r12.y
    r11.z = (r12.yyyy).z;
    // 144: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 145: mad r12.xyz, cb0[20].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[20].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 146: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 147: mul r12.xyz, r12.xyzx, cb0[12].xyzx
    r12.xyz = ((r12.xyzx)*(source[12].xyzx)).xyz;
    // 148: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 149: mov_sat r3.w, cb0[9].w
    r3.w = (saturate(source[9].wwww)).w;
    // 150: mad r13.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r13.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 151: mul r2.w, r3.w, l(0.080000)
    r2.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 152: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 153: mad r13.xyz, r7.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r7.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 154: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 155: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 156: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 157: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 158: dp3 r3.w, r1.xyzx, r14.xyzx
    r3.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 159: mul r1.xyz, r1.xyzx, r3.wwww
    r1.xyz = ((r1.xyzx)*(r3.wwww)).xyz;
    // 160: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 161: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 162: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 163: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 164: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 165: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 166: mad_sat r15.y, r4.w, l(0.300000), r7.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 167: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 168: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 169: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 170: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 171: add r2.w, r1.z, l(1.000000)
    r2.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 172: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 174: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 175: add r2.w, r1.w, r15.x
    r2.w = ((r1.wwww)+(r15.xxxx)).w;
    // 176: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 177: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 178: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 179: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 180: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 181: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 182: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 183: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 184: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 185: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 186: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 187: mul r8.xyz, r8.xyzx, r12.xyzx
    r8.xyz = ((r8.xyzx)*(r12.xyzx)).xyz;
    // 188: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 189: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 190: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 191: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 192: dp2 r5.x, r2.xyxx, r7.xyxx
    r5.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 193: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 194: mul r2.x, r15.y, l(5.000000)
    r2.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 195: mul r2.y, r15.y, r15.y
    r2.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 196: mul r2.y, r2.w, r2.y
    r2.y = ((r2.wwww)*(r2.yyyy)).y;
    // 197: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 198: add r2.y, r1.w, r2.y
    r2.y = ((r1.wwww)+(r2.yyyy)).y;
    // 199: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 200: add_sat r1.w, r2.y, l(-1.000000)
    r1.w = (saturate((r2.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 201: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 202: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s6, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 203: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 204: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 205: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 206: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 207: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 208: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 209: mul r1.xyz, r4.xyzx, r4.xyzx
    r1.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 210: dp3 r1.x, r10.xyzx, r1.xyzx
    r1.x = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 211: add r0.w, r0.w, -r1.x
    r0.w = ((r0.wwww)+(-(r1.xxxx))).w;
    // 212: mad r0.w, r7.z, r0.w, r1.x
    r0.w = ((r7.zzzz)*(r0.wwww)+(r1.xxxx)).w;
    // 213: mad r1.xyz, r9.xyzx, r0.wwww, r6.xyzx
    r1.xyz = ((r9.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 214: mul r4.xyz, r0.wwww, r9.xyzx
    r4.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 215: mad r0.w, r1.w, r13.x, r13.y
    r0.w = ((r1.wwww)*(r13.xxxx)+(r13.yyyy)).w;
    // 216: mad r0.w, r0.w, r1.w, r13.z
    r0.w = ((r0.wwww)*(r1.wwww)+(r13.zzzz)).w;
    // 217: mul r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)*(r0.wwww)).w;
    // 218: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 219: mul r5.xyz, r0.wwww, r1.xyzx
    r5.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 220: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 221: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 222: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 223: mul r1.xyz, r2.xyzx, r5.xyzx
    r1.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 224: mad r2.xyz, r1.xyzx, r15.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r15.xzwx)+(r8.xyzx)).xyz;
    // 225: mul r1.xyz, r15.xzwx, r1.xyzx
    r1.xyz = ((r15.xzwx)*(r1.xyzx)).xyz;
    // 226: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 227: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 228: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 229: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 230: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 231: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 232: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 233: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 234: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 235: mul r1.xyz, r0.xxxx, cb0[3].xyzx
    r1.xyz = ((r0.xxxx)*(source[3].xyzx)).xyz;
    // 236: movc r0.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 237: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 238: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 239: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 240: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 241: dp3 r0.x, r11.xyzx, r11.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 242: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 243: mul r0.xyz, r0.xxxx, r11.xyzx
    r0.xyz = ((r0.xxxx)*(r11.xyzx)).xyz;
    // 244: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 245: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 246: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 247: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 248: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 249: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 250: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 251: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 252: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 253: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 254: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 255: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 256: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 257: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 258: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 259: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 260: ret
    return output;
}

// source.character.static-map-native-1162.v1 / source program 315e238eb72829489b1a46e3d07fb419
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1162(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1162(input);
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
    // 50: mul r7.xyz, cb0[5].xyzx, cb0[8].xxxx
    r7.xyz = ((source[5].xyzx)*(source[8].xxxx)).xyz;
    // 51: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 52: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 53: mad r6.xyz, -r7.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 54: mad r6.xyz, cb0[8].zzzz, r6.xyzx, r8.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 55: mul r7.xyz, cb0[4].xyzx, cb0[7].wwww
    r7.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 56: mad r6.xyz, -r3.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r3.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 57: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 58: mad r3.xyz, r0.xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 59: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 60: mul r6.xyz, r3.xyzx, cb0[8].wwww
    r6.xyz = ((r3.xyzx)*(source[8].wwww)).xyz;
    // 61: mad r3.xyz, cb0[9].xxxx, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 63: mul r0.y, r7.z, cb0[9].y
    r0.y = ((r7.zzzz)*(source[9].yyyy)).y;
    // 64: mul r7.xy, r7.yxyy, cb0[10].ywyy
    r7.xy = ((r7.yxyy)*(source[10].ywyy)).xy;
    // 65: log r1.w, |r0.y|
    r1.w = (log2(abs(r0.yyyy))).w;
    // 66: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 67: mul r1.w, r1.w, cb0[9].z
    r1.w = ((r1.wwww)*(source[9].zzzz)).w;
    // 68: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 69: movc r0.y, r0.y, l(0), r1.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).y;
    // 70: min r1.w, r0.y, l(1.000000)
    r1.w = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 71: mul_sat r7.w, r0.y, cb2[3].w
    r7.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 72: mad r3.xyz, r1.wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 73: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 75: mad_sat r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = (saturate((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 76: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 77: mul r6.xy, r0.zwzz, cb0[6].zzzz
    r6.xy = ((r0.zwzz)*(source[6].zzzz)).xy;
    // 78: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 79: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 80: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 81: add r6.z, r0.y, l(0.000010)
    r6.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 82: add r0.yzw, -r1.xxyz, r6.xxyz
    r0.yzw = ((-(r1.xxyz))+(r6.xxyz)).yzw;
    // 83: mad r0.xyz, r0.xxxx, r0.yzwy, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r1.xyzx)).xyz;
    // 84: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 85: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 86: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 88: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 89: mul r6.xyz, r0.wwww, v6.xyzx
    r6.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 90: dp3 r0.w, r6.xyzx, r1.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 91: mad r8.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r8.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 92: mul r8.xy, r8.xyxx, r8.xyxx
    r8.xy = ((r8.xyxx)*(r8.xyxx)).xy;
    // 93: mul r8.yzw, r8.yyyy, cb0[23].xxyz
    r8.yzw = ((r8.yyyy)*(source[23].xxyz)).yzw;
    // 94: mad r8.xyz, r8.xxxx, cb0[22].xyzx, r8.yzwy
    r8.xyz = ((r8.xxxx)*(source[22].xyzx)+(r8.yzwy)).xyz;
    // 95: mul r8.xyz, r8.xyzx, cb0[24].wwww
    r8.xyz = ((r8.xyzx)*(source[24].wwww)).xyz;
    // 96: mul r8.xyz, r3.xyzx, r8.xyzx
    r8.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 97: mad r9.xyz, r3.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r9.xyz = ((r3.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 98: mad r10.xyz, r3.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r10.xyz = ((r3.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 99: mad r11.xyz, r3.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r11.xyz = ((r3.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 100: log r12.xy, |r7.xyxx|
    r12.xy = (log2(abs(r7.xyxx))).xy;
    // 101: lt r7.xy, |r7.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((abs(r7.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 102: mul r0.w, r12.y, cb0[11].x
    r0.w = ((r12.yyyy)*(source[11].xxxx)).w;
    // 103: mul r1.w, r12.x, cb0[10].z
    r1.w = ((r12.xxxx)*(source[10].zzzz)).w;
    // 104: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 105: movc r1.w, r7.x, l(0), r1.w
    r1.w = ((asuint(r7.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 106: max r1.w, r1.w, cb0[0].x
    r1.w = (max(r1.wwww,source[0].xxxx)).w;
    // 107: min r7.z, r1.w, l(1.000000)
    r7.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 108: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 109: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: movc r0.w, r7.y, l(0), r0.w
    r0.w = ((asuint(r7.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 111: mad r10.xyz, r0.wwww, r10.xyzx, r11.xyzx
    r10.xyz = ((r0.wwww)*(r10.xyzx)+(r11.xyzx)).xyz;
    // 112: mad r9.xyz, r10.xyzx, r0.wwww, r9.xyzx
    r9.xyz = ((r10.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 113: mul r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = ((r0.wwww)*(r9.xyzx)).xyz;
    // 114: max r9.xyz, r0.wwww, r9.xyzx
    r9.xyz = (max(r0.wwww,r9.xyzx)).xyz;
    // 115: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 116: dp3 r9.x, r2.xyzx, r1.xyzx
    r9.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 117: dp3 r9.y, r5.xyzx, r1.xyzx
    r9.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 118: dp2 r10.z, r9.xyxx, cb0[13].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 119: dp3 r10.y, r4.xyzx, r1.xyzx
    r10.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 120: mul r7.xy, cb0[13].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((source[13].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 121: dp2 r10.x, r9.xyxx, r7.xyxx
    r10.x = (dot((r9.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 122: mov r10.w, l(1.000000)
    r10.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 123: dp4 r11.x, cb0[14].xyzw, r10.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 124: dp4 r11.y, cb0[15].xyzw, r10.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 125: dp4 r11.z, cb0[16].xyzw, r10.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 126: mul r12.xyzw, r10.yzzx, r10.xyzz
    r12.xyzw = ((r10.yzzx)*(r10.xyzz)).xyzw;
    // 127: dp4 r13.x, cb0[17].xyzw, r12.xyzw
    r13.x = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 128: dp4 r13.y, cb0[18].xyzw, r12.xyzw
    r13.y = (dot((source[18].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 129: dp4 r13.z, cb0[19].xyzw, r12.xyzw
    r13.z = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 130: add r11.xyz, r11.xyzx, r13.xyzx
    r11.xyz = ((r11.xyzx)+(r13.xyzx)).xyz;
    // 131: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 132: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 133: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 134: mad r10.xyz, cb0[20].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[20].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 135: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 136: mul r10.xyz, r10.xyzx, cb0[12].xyzx
    r10.xyz = ((r10.xyzx)*(source[12].xyzx)).xyz;
    // 137: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 138: mov_sat r3.w, cb0[9].w
    r3.w = (saturate(source[9].wwww)).w;
    // 139: mad r11.xyz, -r3.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r3.xyzx
    r11.xyz = ((-(r3.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r3.xyzx)).xyz;
    // 140: mul r1.w, r3.w, l(0.080000)
    r1.w = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 141: mov o3.xyzw, r3.xyzw
    output.targets[3].xyzw = (r3.xyzw).xyzw;
    // 142: mad r11.xyz, r7.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r7.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 143: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 144: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 145: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 146: mul r12.xyz, r2.wwww, v5.xyzx
    r12.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 147: dp3 r2.w, r1.xyzx, r12.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 148: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 149: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 150: deriv_rtx_coarse r13.x, r2.w
    r13.x = (ddx_coarse(r2.wwww)).x;
    // 151: deriv_rty_coarse r13.y, r2.w
    r13.y = (ddy_coarse(r2.wwww)).y;
    // 152: add r2.w, r2.w, l(1.000000)
    r2.w = ((r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 154: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 155: mad_sat r13.y, r3.w, l(0.300000), r7.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r7.zzzz))).y;
    // 156: mov o2.zw, r7.zzzw
    output.targets[2].zw = (r7.zzzw).zw;
    // 157: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 159: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 160: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 161: add r1.w, r1.z, l(1.000000)
    r1.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 163: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 164: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
    r13.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 165: add r1.w, r0.w, r13.x
    r1.w = ((r0.wwww)+(r13.xxxx)).w;
    // 166: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 167: mul r15.xyz, r11.xyzx, r13.wwww
    r15.xyz = ((r11.xyzx)*(r13.wwww)).xyz;
    // 168: mad r14.xyz, r14.xyzx, r13.zzzz, r15.xyzx
    r14.xyz = ((r14.xyzx)*(r13.zzzz)+(r15.xyzx)).xyz;
    // 169: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r13.w
    r2.w = r13.w != 0.f ? 1.f / r13.w : 0.f;
    // 170: add r2.w, r2.w, l(-1.000000)
    r2.w = ((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 171: mad r13.xzw, r11.xxyz, r2.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r13.xzw = ((r11.xxyz)*(r2.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 172: dp3 r2.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 173: mad r11.xyz, r2.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r11.xyz = ((r2.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 174: mad r15.xyz, -r14.xyzx, r13.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r15.xyz = ((-(r14.xyzx))*(r13.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 175: mul r13.xzw, r13.xxzw, r14.xxyz
    r13.xzw = ((r13.xxzw)*(r14.xxyz)).xzw;
    // 176: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 177: mul r8.xyz, r8.xyzx, r10.xyzx
    r8.xyz = ((r8.xyzx)*(r10.xyzx)).xyz;
    // 178: mad r8.xyz, -r8.xyzx, r7.wwww, r8.xyzx
    r8.xyz = ((-(r8.xyzx))*(r7.wwww)+(r8.xyzx)).xyz;
    // 179: dp3 r2.x, r2.xyzx, r1.xyzx
    r2.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 180: dp3 r2.y, r5.xyzx, r1.xyzx
    r2.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 181: dp2 r5.x, r2.xyxx, r7.xyxx
    r5.x = (dot((r2.xyxx).xy,(r7.xyxx).xy).xxxx).x;
    // 182: dp2 r5.z, r2.xyxx, cb0[13].xyxx
    r5.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 183: mul r2.x, r13.y, l(5.000000)
    r2.x = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 184: mul r2.y, r13.y, r13.y
    r2.y = ((r13.yyyy)*(r13.yyyy)).y;
    // 185: mul r1.w, r1.w, r2.y
    r1.w = ((r1.wwww)*(r2.yyyy)).w;
    // 186: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 187: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 188: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 189: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 190: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 191: dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 192: mad r1.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 193: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 194: sample_l_indexable(texturecube)(float,float,float,float) r2.xyzw, r5.xyzx, t6.xyzw, s5, r2.x
    r2.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 195: mul r2.xyz, r2.xyzx, r2.wwww
    r2.xyz = ((r2.xyzx)*(r2.wwww)).xyz;
    // 196: mul r2.xyz, r2.xyzx, cb0[12].xyzx
    r2.xyz = ((r2.xyzx)*(source[12].xyzx)).xyz;
    // 197: mad r2.xyz, r2.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r2.xyz = ((r2.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 198: mad r1.z, r0.w, r11.x, r11.y
    r1.z = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).z;
    // 199: mad r1.z, r1.z, r0.w, r11.z
    r1.z = ((r1.zzzz)*(r0.wwww)+(r11.zzzz)).z;
    // 200: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 201: max r0.w, r0.w, r1.z
    r0.w = (max(r0.wwww,r1.zzzz)).w;
    // 202: mul r1.yzw, r1.yyyy, cb0[23].xxyz
    r1.yzw = ((r1.yyyy)*(source[23].xxyz)).yzw;
    // 203: mad r1.xyz, cb0[22].xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((source[22].xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 204: mul r1.xyz, r1.xyzx, cb0[24].wwww
    r1.xyz = ((r1.xyzx)*(source[24].wwww)).xyz;
    // 205: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 206: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 207: mad r2.xyz, r1.xyzx, r13.xzwx, r8.xyzx
    r2.xyz = ((r1.xyzx)*(r13.xzwx)+(r8.xyzx)).xyz;
    // 208: mul r1.xyz, r13.xzwx, r1.xyzx
    r1.xyz = ((r13.xzwx)*(r1.xyzx)).xyz;
    // 209: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 210: dp3 r0.x, r0.xyzx, r12.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 211: add r0.y, -|r12.z|, l(1.000000)
    r0.y = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 212: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 213: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 214: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 215: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 216: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 217: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 218: mul r0.xzw, r0.xxxx, cb0[3].xxyz
    r0.xzw = ((r0.xxxx)*(source[3].xxyz)).xzw;
    // 219: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 220: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 221: add r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)+(r0.xyzx)).xyz;
    // 222: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 223: mad o0.xyz, r3.xyzx, cb0[24].xyzx, r0.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(source[24].xyzx)+(r0.xyzx)).xyz;
    // 224: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 225: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 226: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 227: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 228: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 229: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 230: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 231: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 232: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 233: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 234: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 235: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 236: mov o4.zw, l(0,0,0,0)
    output.targets[4].zw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).zw;
    // 237: ftou r0.x, cb0[21].z
    r0.x = (asfloat((uint4)(source[21].zzzz))).x;
    // 238: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 239: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 240: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 241: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 242: ret
    return output;
}

// source.character.static-map-native-1163.v1 / source program 5b68751ea0e618459f8dc031d62a193d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1163(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 5: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 6: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 7: mul r1.zw, r1.zzzw, cb0[7].wwww
    r1.zw = ((r1.zzzw)*(source[7].wwww)).zw;
    // 8: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 16: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 19: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 23: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 24: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 25: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 27: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 28: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 29: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 31: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.zw, v4.xxxy, cb0[8].xxxx
    r0.zw = ((v4.xxxy)*(source[8].xxxx)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 39: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 41: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 42: max r1.w, cb0[8].z, l(0.000000)
    r1.w = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 44: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 45: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 46: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 49: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 50: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 51: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 52: mul r8.xyz, cb0[6].xyzx, cb0[9].wwww
    r8.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 53: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 54: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 56: mad r7.xyz, cb0[10].yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((source[10].yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 57: mul r8.xyz, cb0[5].xyzx, cb0[9].zzzz
    r8.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 58: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 59: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 60: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 61: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 62: mul r7.xyz, r4.xyzx, cb0[10].zzzz
    r7.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 63: mad r4.xyz, cb0[10].wwww, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 64: mul r0.y, r1.z, cb0[11].x
    r0.y = ((r1.zzzz)*(source[11].xxxx)).y;
    // 65: mul r1.xy, r1.yxyy, cb0[12].xzxx
    r1.xy = ((r1.yxyy)*(source[12].xzxx)).xy;
    // 66: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 67: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 68: mul r1.z, r1.z, cb0[11].y
    r1.z = ((r1.zzzz)*(source[11].yyyy)).z;
    // 69: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 70: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 71: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 72: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 73: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 74: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 76: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 77: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 78: mul r7.xy, r0.zwzz, cb0[8].yyyy
    r7.xy = ((r0.zwzz)*(source[8].yyyy)).xy;
    // 79: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 80: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 83: add r0.yzw, -r2.xxyz, r7.xxyz
    r0.yzw = ((-(r2.xxyz))+(r7.xxyz)).yzw;
    // 84: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 85: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 88: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 91: dp3 r0.w, r7.xyzx, r2.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 92: mad r1.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 93: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 94: mul r7.xyz, r1.wwww, cb0[24].xyzx
    r7.xyz = ((r1.wwww)*(source[24].xyzx)).xyz;
    // 95: mad r7.xyz, r1.zzzz, cb0[23].xyzx, r7.xyzx
    r7.xyz = ((r1.zzzz)*(source[23].xyzx)+(r7.xyzx)).xyz;
    // 96: mul r7.xyz, r7.xyzx, cb0[25].wwww
    r7.xyz = ((r7.xyzx)*(source[25].wwww)).xyz;
    // 97: mul r9.xyz, r4.xyzx, r7.xyzx
    r9.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 98: dp2_sat r10.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r10.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 99: dp3_sat r10.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r10.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 100: dp3_sat r10.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r10.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 101: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 102: sample_indexable(texture2d)(float,float,float,float) r11.xyz, v3.zwzz, t8.xyzw, s5
    r11.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 103: mul r11.xyz, r11.xyzx, cb0[27].xyzx
    r11.xyz = ((r11.xyzx)*(source[27].xyzx)).xyz;
    // 104: dp3 r0.w, r11.xyzx, r10.xyzx
    r0.w = (dot((r11.xyzx).xyz,(r10.xyzx).xyz).xxxx).w;
    // 105: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t7.xyzw, s5
    r10.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 106: mul r10.xyz, r10.xyzx, cb0[26].xyzx
    r10.xyz = ((r10.xyzx)*(source[26].xyzx)).xyz;
    // 107: mul r12.xyz, r0.wwww, r10.xyzx
    r12.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 108: mad r9.xyz, r4.xyzx, r12.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r12.xyzx)+(r9.xyzx)).xyz;
    // 109: mad r12.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r12.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 110: mad r13.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r13.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 111: mad r14.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r14.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 112: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 113: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 114: mul r1.zw, r1.zzzw, cb0[12].yyyw
    r1.zw = ((r1.zzzw)*(source[12].yyyw)).zw;
    // 115: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 116: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 117: movc r1.xy, r1.xyxx, l(0,0,0,0), r1.zwzz
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zwzz)).xy;
    // 118: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 119: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 120: mad r1.xzw, r1.yyyy, r13.xxyz, r14.xxyz
    r1.xzw = ((r1.yyyy)*(r13.xxyz)+(r14.xxyz)).xzw;
    // 121: mad r1.xzw, r1.xxzw, r1.yyyy, r12.xxyz
    r1.xzw = ((r1.xxzw)*(r1.yyyy)+(r12.xxyz)).xzw;
    // 122: mul r1.xzw, r1.yyyy, r1.xxzw
    r1.xzw = ((r1.yyyy)*(r1.xxzw)).xzw;
    // 123: max r1.xzw, r1.xxzw, r1.yyyy
    r1.xzw = (max(r1.xxzw,r1.yyyy)).xzw;
    // 124: mul r1.xzw, r1.xxzw, r9.xxyz
    r1.xzw = ((r1.xxzw)*(r9.xxyz)).xzw;
    // 125: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 126: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 127: dp2 r12.z, r9.xyxx, cb0[14].xyxx
    r12.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 128: dp3 r12.y, r5.xyzx, r2.xyzx
    r12.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 129: mul r8.xy, cb0[14].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[14].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 130: dp2 r12.x, r9.xyxx, r8.xyxx
    r12.x = (dot((r9.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 131: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 132: dp4 r13.x, cb0[15].xyzw, r12.xyzw
    r13.x = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 133: dp4 r13.y, cb0[16].xyzw, r12.xyzw
    r13.y = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 134: dp4 r13.z, cb0[17].xyzw, r12.xyzw
    r13.z = (dot((source[17].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 135: mul r14.xyzw, r12.yzzx, r12.xyzz
    r14.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 136: dp4 r15.x, cb0[18].xyzw, r14.xyzw
    r15.x = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 137: dp4 r15.y, cb0[19].xyzw, r14.xyzw
    r15.y = (dot((source[19].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 138: dp4 r15.z, cb0[20].xyzw, r14.xyzw
    r15.z = (dot((source[20].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 139: add r13.xyz, r13.xyzx, r15.xyzx
    r13.xyz = ((r13.xyzx)+(r15.xyzx)).xyz;
    // 140: mul r2.w, r12.y, r12.y
    r2.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 141: mov r9.z, r12.y
    r9.z = (r12.yyyy).z;
    // 142: mad r2.w, r12.x, r12.x, -r2.w
    r2.w = ((r12.xxxx)*(r12.xxxx)+(-(r2.wwww))).w;
    // 143: mad r12.xyz, cb0[21].xyzx, r2.wwww, r13.xyzx
    r12.xyz = ((source[21].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 144: max r12.xyz, r12.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r12.xyz = (max(r12.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 145: mul r12.xyz, r12.xyzx, cb0[13].xyzx
    r12.xyz = ((r12.xyzx)*(source[13].xyzx)).xyz;
    // 146: mad r12.xyz, r12.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r12.xyz = ((r12.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 147: mov_sat r4.w, cb0[11].z
    r4.w = (saturate(source[11].zzzz)).w;
    // 148: mad r13.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r13.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 149: mul r2.w, r4.w, l(0.080000)
    r2.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 150: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 151: mad r13.xyz, r8.wwww, r13.xyzx, r2.wwww
    r13.xyz = ((r8.wwww)*(r13.xyzx)+(r2.wwww)).xyz;
    // 152: mul_sat r2.w, r13.y, l(50.000000)
    r2.w = (saturate((r13.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 153: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 154: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 155: mul r14.xyz, r3.wwww, v5.xyzx
    r14.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 156: dp3 r3.w, r2.xyzx, r14.xyzx
    r3.w = (dot((r2.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 157: mul r2.xyz, r2.xyzx, r3.wwww
    r2.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // 158: mad r2.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r2.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 159: deriv_rtx_coarse r15.x, r3.w
    r15.x = (ddx_coarse(r3.wwww)).x;
    // 160: deriv_rty_coarse r15.y, r3.w
    r15.y = (ddy_coarse(r3.wwww)).y;
    // 161: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: dp2 r4.w, r15.xyxx, r15.xyxx
    r4.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 163: sqrt r4.w, r4.w
    r4.w = (sqrt(r4.wwww)).w;
    // 164: mad_sat r15.y, r4.w, l(0.300000), r8.z
    r15.y = (saturate((r4.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 165: add r4.w, -r15.y, l(1.000000)
    r4.w = ((-(r15.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 166: max r16.xyz, r13.xyzx, r4.wwww
    r16.xyz = (max(r13.xyzx,r4.wwww)).xyz;
    // 167: add r16.xyz, -r13.xyzx, r16.xyzx
    r16.xyz = ((-(r13.xyzx))+(r16.xyzx)).xyz;
    // 168: mul r16.xyz, r2.wwww, r16.xyzx
    r16.xyz = ((r2.wwww)*(r16.xyzx)).xyz;
    // 169: add r2.w, r2.z, l(1.000000)
    r2.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 170: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 171: add_sat r15.x, -r2.w, r3.w
    r15.x = (saturate((-(r2.wwww))+(r3.wwww))).x;
    // 172: sample_indexable(texture2d)(float,float,float,float) r15.zw, r15.xyxx, t5.zwxy, s7
    r15.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 173: add r2.w, r1.y, r15.x
    r2.w = ((r1.yyyy)+(r15.xxxx)).w;
    // 174: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 175: mul r17.xyz, r13.xyzx, r15.wwww
    r17.xyz = ((r13.xyzx)*(r15.wwww)).xyz;
    // 176: mad r16.xyz, r16.xyzx, r15.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r15.zzzz)+(r17.xyzx)).xyz;
    // 177: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r15.w
    r3.w = r15.w != 0.f ? 1.f / r15.w : 0.f;
    // 178: add r3.w, r3.w, l(-1.000000)
    r3.w = ((r3.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 179: mad r15.xzw, r13.xxyz, r3.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r15.xzw = ((r13.xxyz)*(r3.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 180: dp3 r3.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 181: mad r13.xyz, r3.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r13.xyz = ((r3.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 182: mad r17.xyz, -r16.xyzx, r15.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r16.xyzx))*(r15.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 183: mul r15.xzw, r15.xxzw, r16.xxyz
    r15.xzw = ((r15.xxzw)*(r16.xxyz)).xzw;
    // 184: mul r12.xyz, r12.xyzx, r17.xyzx
    r12.xyz = ((r12.xyzx)*(r17.xyzx)).xyz;
    // 185: mul r1.xzw, r1.xxzw, r12.xxyz
    r1.xzw = ((r1.xxzw)*(r12.xxyz)).xzw;
    // 186: mad r1.xzw, -r1.xxzw, r8.wwww, r1.xxzw
    r1.xzw = ((-(r1.xxzw))*(r8.wwww)+(r1.xxzw)).xzw;
    // 187: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 188: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 189: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 190: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 191: dp2 r6.z, r3.xyxx, cb0[14].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 192: mul r3.x, r15.y, l(5.000000)
    r3.x = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).x;
    // 193: mul r3.y, r15.y, r15.y
    r3.y = ((r15.yyyy)*(r15.yyyy)).y;
    // 194: mul r2.w, r2.w, r3.y
    r2.w = ((r2.wwww)*(r3.yyyy)).w;
    // 195: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 196: add r2.w, r1.y, r2.w
    r2.w = ((r1.yyyy)+(r2.wwww)).w;
    // 197: mov o5.y, r1.y
    output.targets[5].y = (r1.yyyy).y;
    // 198: add_sat r1.y, r2.w, l(-1.000000)
    r1.y = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 199: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s6, r3.x
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r3.xxxx).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 202: mul r3.xyz, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((r3.xyzx)*(source[13].xyzx)).xyz;
    // 203: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 204: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 205: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 206: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 207: mul r2.xyz, r5.xyzx, r5.xyzx
    r2.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 208: dp3 r2.x, r11.xyzx, r2.xyzx
    r2.x = (dot((r11.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 209: add r0.w, r0.w, -r2.x
    r0.w = ((r0.wwww)+(-(r2.xxxx))).w;
    // 210: mad r0.w, r8.z, r0.w, r2.x
    r0.w = ((r8.zzzz)*(r0.wwww)+(r2.xxxx)).w;
    // 211: mad r2.xyz, r10.xyzx, r0.wwww, r7.xyzx
    r2.xyz = ((r10.xyzx)*(r0.wwww)+(r7.xyzx)).xyz;
    // 212: mul r5.xyz, r0.wwww, r10.xyzx
    r5.xyz = ((r0.wwww)*(r10.xyzx)).xyz;
    // 213: mad r0.w, r1.y, r13.x, r13.y
    r0.w = ((r1.yyyy)*(r13.xxxx)+(r13.yyyy)).w;
    // 214: mad r0.w, r0.w, r1.y, r13.z
    r0.w = ((r0.wwww)*(r1.yyyy)+(r13.zzzz)).w;
    // 215: mul r0.w, r1.y, r0.w
    r0.w = ((r1.yyyy)*(r0.wwww)).w;
    // 216: max r0.w, r0.w, r1.y
    r0.w = (max(r0.wwww,r1.yyyy)).w;
    // 217: mul r6.xyz, r0.wwww, r2.xyzx
    r6.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 218: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 219: div r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)/(r2.xyzx)).xyz;
    // 220: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: mul r2.xyz, r3.xyzx, r6.xyzx
    r2.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 222: mad r1.xyz, r2.xyzx, r15.xzwx, r1.xzwx
    r1.xyz = ((r2.xyzx)*(r15.xzwx)+(r1.xzwx)).xyz;
    // 223: mul r2.xyz, r15.xzwx, r2.xyzx
    r2.xyz = ((r15.xzwx)*(r2.xyzx)).xyz;
    // 224: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 225: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 226: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 227: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 228: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 229: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 230: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 231: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 232: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 233: mul r2.xyz, r0.xxxx, cb0[4].xyzx
    r2.xyz = ((r0.xxxx)*(source[4].xyzx)).xyz;
    // 234: movc r0.xyz, r0.yyyy, l(0,0,0,0), r2.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 235: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 236: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 237: mad o0.xyz, r4.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
    // 238: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 239: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 240: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 241: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 242: ge r1.w, l(0.000000), r0.z
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 243: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 244: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 245: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 246: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 247: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 248: movc r0.xy, r1.wwww, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.wwww) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 249: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 250: mul o4.z, r0.w, r1.x
    output.targets[4].z = ((r0.wwww)*(r1.xxxx)).z;
    // 251: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 252: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 253: ftou r0.x, cb0[22].z
    r0.x = (asfloat((uint4)(source[22].zzzz))).x;
    // 254: and r0.x, r0.x, l(31)
    r0.x = (asfloat(asuint(r0.xxxx) & uint4(31u,31u,31u,31u))).x;
    // 255: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 256: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 257: mov o5.xz, l(0,0,1.000000,0)
    output.targets[5].xz = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).xz;
    // 258: ret
    return output;
}

// source.character.static-map-native-1163.v1 / source program e441f0cc85c04d40b1ea69df4731f392
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1163(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1163(input);
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
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 5: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 6: dp2 r0.w, r1.zwzz, r1.zwzz
    r0.w = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).w;
    // 7: mul r1.zw, r1.zzzw, cb0[7].wwww
    r1.zw = ((r1.zzzw)*(source[7].wwww)).zw;
    // 8: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r2.z, r0.w, l(0.000010)
    r2.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 16: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 19: dp3 r4.x, r3.xyzx, r2.xyzx
    r4.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r5.xyz, r0.wwww, v1.xyzx
    r5.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 23: mul r6.xyz, r3.yzxy, r5.zxyz
    r6.xyz = ((r3.yzxy)*(r5.zxyz)).xyz;
    // 24: mad r6.xyz, r5.yzxy, r3.zxyz, -r6.xyzx
    r6.xyz = ((r5.yzxy)*(r3.zxyz)+(-(r6.xyzx))).xyz;
    // 25: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 27: dp3 r4.z, r5.xyzx, r2.xyzx
    r4.z = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 28: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 29: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 31: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.zw, v4.xxxy, cb0[8].xxxx
    r0.zw = ((v4.xxxy)*(source[8].xxxx)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r0.zwzz, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t1.zwxy, s1, l(0.000000)
    r0.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 39: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mul r1.w, r7.w, r7.w
    r1.w = ((r7.wwww)*(r7.wwww)).w;
    // 41: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 42: max r1.w, cb0[8].z, l(0.000000)
    r1.w = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 44: mul r2.w, r0.y, r1.w
    r2.w = ((r0.yyyy)*(r1.wwww)).w;
    // 45: mad r0.x, r0.x, r2.w, r0.x
    r0.x = ((r0.xxxx)*(r2.wwww)+(r0.xxxx)).x;
    // 46: add r2.w, -r1.w, r0.x
    r2.w = ((-(r1.wwww))+(r0.xxxx)).w;
    // 47: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: div r1.w, l(1.000000, 1.000000, 1.000000, 1.000000), r1.w
    r1.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.wwww)).w;
    // 49: mad r0.x, -r1.w, r2.w, r0.x
    r0.x = ((-(r1.wwww))*(r2.wwww)+(r0.xxxx)).x;
    // 50: mul r1.w, r2.w, r1.w
    r1.w = ((r2.wwww)*(r1.wwww)).w;
    // 51: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 52: mul r8.xyz, cb0[6].xyzx, cb0[9].wwww
    r8.xyz = ((source[6].xyzx)*(source[9].wwww)).xyz;
    // 53: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 54: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 55: mad r7.xyz, -r8.xyzx, r7.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 56: mad r7.xyz, cb0[10].yyyy, r7.xyzx, r9.xyzx
    r7.xyz = ((source[10].yyyy)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 57: mul r8.xyz, cb0[5].xyzx, cb0[9].zzzz
    r8.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 58: mad r7.xyz, -r4.xyzx, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r4.xyzx))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 59: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 60: mad r4.xyz, r0.xxxx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 61: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 62: mul r7.xyz, r4.xyzx, cb0[10].zzzz
    r7.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 63: mad r4.xyz, cb0[10].wwww, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[10].wwww)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 64: mul r0.y, r1.z, cb0[11].x
    r0.y = ((r1.zzzz)*(source[11].xxxx)).y;
    // 65: mul r1.xy, r1.yxyy, cb0[12].xzxx
    r1.xy = ((r1.yxyy)*(source[12].xzxx)).xy;
    // 66: log r1.z, |r0.y|
    r1.z = (log2(abs(r0.yyyy))).z;
    // 67: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 68: mul r1.z, r1.z, cb0[11].y
    r1.z = ((r1.zzzz)*(source[11].yyyy)).z;
    // 69: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 70: movc r0.y, r0.y, l(0), r1.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).y;
    // 71: min r1.z, r0.y, l(1.000000)
    r1.z = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 72: mul_sat r8.w, r0.y, cb2[3].w
    r8.w = (saturate((r0.yyyy)*(passValues[3].wwww))).w;
    // 73: mad r4.xyz, r1.zzzz, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.zzzz)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 74: add r7.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r7.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 76: mad_sat r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = (saturate((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 77: dp2 r0.y, r0.zwzz, r0.zwzz
    r0.y = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).y;
    // 78: mul r7.xy, r0.zwzz, cb0[8].yyyy
    r7.xy = ((r0.zwzz)*(source[8].yyyy)).xy;
    // 79: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 80: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 81: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 82: add r7.z, r0.y, l(0.000010)
    r7.z = ((r0.yyyy)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 83: add r0.yzw, -r2.xxyz, r7.xxyz
    r0.yzw = ((-(r2.xxyz))+(r7.xxyz)).yzw;
    // 84: mad r0.xyz, r0.xxxx, r0.yzwy, r2.xyzx
    r0.xyz = ((r0.xxxx)*(r0.yzwy)+(r2.xyzx)).xyz;
    // 85: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 88: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r7.xyz, r0.wwww, v6.xyzx
    r7.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 91: dp3 r0.w, r7.xyzx, r2.xyzx
    r0.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 92: mad r1.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r1.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 93: mul r1.zw, r1.zzzw, r1.zzzw
    r1.zw = ((r1.zzzw)*(r1.zzzw)).zw;
    // 94: mul r9.xyz, r1.wwww, cb0[24].xyzx
    r9.xyz = ((r1.wwww)*(source[24].xyzx)).xyz;
    // 95: mad r9.xyz, r1.zzzz, cb0[23].xyzx, r9.xyzx
    r9.xyz = ((r1.zzzz)*(source[23].xyzx)+(r9.xyzx)).xyz;
    // 96: mul r9.xyz, r9.xyzx, cb0[25].wwww
    r9.xyz = ((r9.xyzx)*(source[25].wwww)).xyz;
    // 97: mul r9.xyz, r4.xyzx, r9.xyzx
    r9.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 98: mad r10.xyz, r4.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r4.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 99: mad r11.xyz, r4.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r11.xyz = ((r4.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 100: mad r12.xyz, r4.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r12.xyz = ((r4.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 101: log r1.zw, |r1.xxxy|
    r1.zw = (log2(abs(r1.xxxy))).zw;
    // 102: lt r1.xy, |r1.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((abs(r1.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 103: mul r1.zw, r1.zzzw, cb0[12].yyyw
    r1.zw = ((r1.zzzw)*(source[12].yyyw)).zw;
    // 104: exp r1.zw, r1.zzzw
    r1.zw = (exp2(r1.zzzw)).zw;
    // 105: min r0.w, r1.w, l(1.000000)
    r0.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 106: movc r1.x, r1.x, l(0), r1.z
    r1.x = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).x;
    // 107: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 108: max r1.x, r1.x, cb0[0].x
    r1.x = (max(r1.xxxx,source[0].xxxx)).x;
    // 109: min r8.z, r1.x, l(1.000000)
    r8.z = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 110: mad r1.xyz, r0.wwww, r11.xyzx, r12.xyzx
    r1.xyz = ((r0.wwww)*(r11.xyzx)+(r12.xyzx)).xyz;
    // 111: mad r1.xyz, r1.xyzx, r0.wwww, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r0.wwww)+(r10.xyzx)).xyz;
    // 112: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 113: max r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = (max(r0.wwww,r1.xyzx)).xyz;
    // 114: mul r1.xyz, r1.xyzx, r9.xyzx
    r1.xyz = ((r1.xyzx)*(r9.xyzx)).xyz;
    // 115: dp3 r9.x, r3.xyzx, r2.xyzx
    r9.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 116: dp3 r9.y, r6.xyzx, r2.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 117: dp2 r10.z, r9.xyxx, cb0[14].xyxx
    r10.z = (dot((r9.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 118: dp3 r10.y, r5.xyzx, r2.xyzx
    r10.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
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
    // 130: mul r1.w, r10.y, r10.y
    r1.w = ((r10.yyyy)*(r10.yyyy)).w;
    // 131: mov r9.z, r10.y
    r9.z = (r10.yyyy).z;
    // 132: mad r1.w, r10.x, r10.x, -r1.w
    r1.w = ((r10.xxxx)*(r10.xxxx)+(-(r1.wwww))).w;
    // 133: mad r10.xyz, cb0[21].xyzx, r1.wwww, r11.xyzx
    r10.xyz = ((source[21].xyzx)*(r1.wwww)+(r11.xyzx)).xyz;
    // 134: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 135: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 136: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[13].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[13].wwww)).xyz;
    // 137: mov_sat r4.w, cb0[11].z
    r4.w = (saturate(source[11].zzzz)).w;
    // 138: mad r11.xyz, -r4.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r4.xyzx
    r11.xyz = ((-(r4.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r4.xyzx)).xyz;
    // 139: mul r1.w, r4.w, l(0.080000)
    r1.w = ((r4.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 140: mov o3.xyzw, r4.xyzw
    output.targets[3].xyzw = (r4.xyzw).xyzw;
    // 141: mad r11.xyz, r8.wwww, r11.xyzx, r1.wwww
    r11.xyz = ((r8.wwww)*(r11.xyzx)+(r1.wwww)).xyz;
    // 142: mul_sat r1.w, r11.y, l(50.000000)
    r1.w = (saturate((r11.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
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
    // 154: mad_sat r13.y, r3.w, l(0.300000), r8.z
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 155: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 156: add r3.w, -r13.y, l(1.000000)
    r3.w = ((-(r13.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 157: max r14.xyz, r11.xyzx, r3.wwww
    r14.xyz = (max(r11.xyzx,r3.wwww)).xyz;
    // 158: add r14.xyz, -r11.xyzx, r14.xyzx
    r14.xyz = ((-(r11.xyzx))+(r14.xyzx)).xyz;
    // 159: mul r14.xyz, r1.wwww, r14.xyzx
    r14.xyz = ((r1.wwww)*(r14.xyzx)).xyz;
    // 160: add r1.w, r2.z, l(1.000000)
    r1.w = ((r2.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 161: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: add_sat r13.x, -r1.w, r2.w
    r13.x = (saturate((-(r1.wwww))+(r2.wwww))).x;
    // 163: sample_indexable(texture2d)(float,float,float,float) r13.zw, r13.xyxx, t5.zwxy, s6
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
    // 175: mul r10.xyz, r10.xyzx, r15.xyzx
    r10.xyz = ((r10.xyzx)*(r15.xyzx)).xyz;
    // 176: mul r1.xyz, r1.xyzx, r10.xyzx
    r1.xyz = ((r1.xyzx)*(r10.xyzx)).xyz;
    // 177: mad r1.xyz, -r1.xyzx, r8.wwww, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(r8.wwww)+(r1.xyzx)).xyz;
    // 178: dp3 r3.x, r3.xyzx, r2.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 179: dp3 r3.y, r6.xyzx, r2.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 180: dp2 r6.x, r3.xyxx, r8.xyxx
    r6.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 181: dp2 r6.z, r3.xyxx, cb0[14].xyxx
    r6.z = (dot((r3.xyxx).xy,(source[14].xyxx).xy).xxxx).z;
    // 182: mul r2.w, r13.y, l(5.000000)
    r2.w = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 183: mul r3.x, r13.y, r13.y
    r3.x = ((r13.yyyy)*(r13.yyyy)).x;
    // 184: mul r1.w, r1.w, r3.x
    r1.w = ((r1.wwww)*(r3.xxxx)).w;
    // 185: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 186: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 187: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 188: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 189: dp3 r6.y, r5.xyzx, r2.xyzx
    r6.y = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 190: dp3 r1.w, r7.xyzx, r2.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 191: mad r2.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 192: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 193: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r6.xyzx, t6.xyzw, s5, r2.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r6.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 194: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 195: mul r3.xyz, r3.xyzx, cb0[13].xyzx
    r3.xyz = ((r3.xyzx)*(source[13].xyzx)).xyz;
    // 196: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[13].wwww)).xyz;
    // 197: mad r1.w, r0.w, r11.x, r11.y
    r1.w = ((r0.wwww)*(r11.xxxx)+(r11.yyyy)).w;
    // 198: mad r1.w, r1.w, r0.w, r11.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r11.zzzz)).w;
    // 199: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 200: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 201: mul r2.yzw, r2.yyyy, cb0[24].xxyz
    r2.yzw = ((r2.yyyy)*(source[24].xxyz)).yzw;
    // 202: mad r2.xyz, cb0[23].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[23].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 203: mul r2.xyz, r2.xyzx, cb0[25].wwww
    r2.xyz = ((r2.xyzx)*(source[25].wwww)).xyz;
    // 204: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 205: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 206: mad r1.xyz, r2.xyzx, r13.xzwx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r13.xzwx)+(r1.xyzx)).xyz;
    // 207: mul r2.xyz, r13.xzwx, r2.xyzx
    r2.xyz = ((r13.xzwx)*(r2.xyzx)).xyz;
    // 208: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
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
    // 217: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 218: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 219: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 220: add r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)+(r0.xyzx)).xyz;
    // 221: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 222: mad o0.xyz, r4.xyzx, cb0[25].xyzx, r0.xyzx
    output.targets[0].xyz = ((r4.xyzx)*(source[25].xyzx)+(r0.xyzx)).xyz;
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

// source.character.static-map-native-1164.v1 / source program af9e580628c8fe4aa040273cc15a3690
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1164(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[12]=1.f;
    source[13]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[5].zzzz
    r0.xy = ((source[0].xyxx)*(source[5].zzzz)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 8: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 16: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r2.xyz, r0.wwww, v1.xyzx
    r2.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 22: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 23: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 24: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 25: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 26: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 27: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 29: dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 30: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mad r0.x, r0.x, l(0.500000), cb0[6].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].yyyy)).x;
    // 32: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.zw, v4.xxxy, cb0[6].zzzz
    r0.zw = ((v4.xxxy)*(source[6].zzzz)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 39: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 40: max r0.z, cb0[5].y, l(0.000000)
    r0.z = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 41: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 42: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 43: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 44: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 45: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 47: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 48: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 49: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 50: dp3 r0.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r0.yzw, -r5.xxyz, r0.yyyy
    r0.yzw = ((-(r5.xxyz))+(r0.yyyy)).yzw;
    // 52: mad r0.yzw, cb0[7].xxxx, r0.yyzw, r5.xxyz
    r0.yzw = ((source[7].xxxx)*(r0.yyzw)+(r5.xxyz)).yzw;
    // 53: mul r7.xyz, cb0[3].xyzx, cb0[7].zzzz
    r7.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 54: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 55: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: mad r7.xyz, -r7.xyzx, r6.xyzx, r1.wwww
    r7.xyz = ((-(r7.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 57: mad r7.xyz, cb0[8].xxxx, r7.xyzx, r8.xyzx
    r7.xyz = ((source[8].xxxx)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 58: mul r8.xyz, cb0[2].xyzx, cb0[7].yyyy
    r8.xyz = ((source[2].xyzx)*(source[7].yyyy)).xyz;
    // 59: mad r7.xyz, -r0.yzwy, r8.xyzx, r7.xyzx
    r7.xyz = ((-(r0.yzwy))*(r8.xyzx)+(r7.xyzx)).xyz;
    // 60: mul r0.yzw, r0.yyzw, r8.xxyz
    r0.yzw = ((r0.yyzw)*(r8.xxyz)).yzw;
    // 61: mad r0.yzw, r0.xxxx, r7.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r7.xxyz)+(r0.yyzw)).yzw;
    // 62: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 63: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 64: mul r1.w, r0.x, l(0.650000)
    r1.w = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 65: mad r1.xyz, r1.wwww, r7.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 66: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r1.xyz, r1.wwww, r1.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 69: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 70: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 71: mul r7.xyz, r1.wwww, v6.xyzx
    r7.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 72: dp3 r1.w, r7.xyzx, r1.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 73: mad r7.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r7.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 74: mul r7.xy, r7.xyxx, r7.xyxx
    r7.xy = ((r7.xyxx)*(r7.xyxx)).xy;
    // 75: mul r7.yzw, r7.yyyy, cb0[10].xxyz
    r7.yzw = ((r7.yyyy)*(source[10].xxyz)).yzw;
    // 76: mad r7.xyz, r7.xxxx, cb0[9].xyzx, r7.yzwy
    r7.xyz = ((r7.xxxx)*(source[9].xyzx)+(r7.yzwy)).xyz;
    // 77: mul r7.xyz, r7.xyzx, cb0[11].wwww
    r7.xyz = ((r7.xyzx)*(source[11].wwww)).xyz;
    // 78: mul r8.xyz, r0.yzwy, r7.xyzx
    r8.xyz = ((r0.yzwy)*(r7.xyzx)).xyz;
    // 79: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 80: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 81: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 82: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 83: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t4.xyzw, s3
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 84: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 85: dp3 r1.w, r10.xyzx, r9.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 86: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t3.xyzw, s3
    r9.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 87: mul r9.xyz, r9.xyzx, cb0[12].xyzx
    r9.xyz = ((r9.xyzx)*(source[12].xyzx)).xyz;
    // 88: mul r11.xyz, r1.wwww, r9.xyzx
    r11.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 89: mad r7.xyz, r9.xyzx, r1.wwww, r7.xyzx
    r7.xyz = ((r9.xyzx)*(r1.wwww)+(r7.xyzx)).xyz;
    // 90: add r7.xyz, r7.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r7.xyz = ((r7.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 91: div r7.xyz, r11.xyzx, r7.xyzx
    r7.xyz = ((r11.xyzx)/(r7.xyzx)).xyz;
    // 92: mad r8.xyz, r0.yzwy, r11.xyzx, r8.xyzx
    r8.xyz = ((r0.yzwy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 93: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 95: add r7.xyz, -r6.xyzx, r2.wwww
    r7.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 96: mad r6.xyz, cb0[8].xxxx, r7.xyzx, r6.xyzx
    r6.xyz = ((source[8].xxxx)*(r7.xyzx)+(r6.xyzx)).xyz;
    // 97: mul r7.xyz, cb0[4].xyzx, cb0[8].yyyy
    r7.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 98: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 99: mad r6.xyz, cb0[8].zzzz, r6.xyzx, -r5.xyzx
    r6.xyz = ((source[8].zzzz)*(r6.xyzx)+(-(r5.xyzx))).xyz;
    // 100: mad r5.xyz, r0.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 101: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 102: mul r5.xyz, r9.xyzx, r5.xyzx
    r5.xyz = ((r9.xyzx)*(r5.xyzx)).xyz;
    // 103: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 104: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 105: mul r6.xyz, r0.xxxx, v5.xyzx
    r6.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 106: dp3 r0.x, r1.xyzx, r6.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 107: mul r7.xyz, r0.xxxx, r1.xyzx
    r7.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 108: mad r6.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 109: dp2_sat r7.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 110: dp3_sat r7.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 111: dp3_sat r7.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 112: log r6.xyz, r7.xyzx
    r6.xyz = (log2(r7.xyzx)).xyz;
    // 113: add r0.x, cb0[8].w, l(1.000000)
    r0.x = ((source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 114: mul r6.xyz, r6.xyzx, r0.xxxx
    r6.xyz = ((r6.xyzx)*(r0.xxxx)).xyz;
    // 115: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 116: dp3 r0.x, r10.xyzx, r6.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 117: mad r6.xyz, r5.xyzx, r0.xxxx, r8.xyzx
    r6.xyz = ((r5.xyzx)*(r0.xxxx)+(r8.xyzx)).xyz;
    // 118: mul r5.xyz, r0.xxxx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 119: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 120: add r5.xyz, r6.xyzx, cb0[1].xyzx
    r5.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // 121: mad o0.xyz, r0.yzwy, cb0[11].xyzx, r5.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 122: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 123: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 124: dp3 r0.x, r3.xyzx, r1.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 125: dp3 r0.y, r4.xyzx, r1.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 126: dp3 r0.z, r2.xyzx, r1.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 127: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 128: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 129: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 130: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 131: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 132: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 133: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 134: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 135: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 136: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 137: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 138: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 139: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 140: mul o4.z, r1.w, r6.x
    output.targets[4].z = ((r1.wwww)*(r6.xxxx)).z;
    // 141: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 142: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 144: ret
    return output;
}

// source.character.static-map-native-1164.v1 / source program 284418bf4cb3c847b5d5dd31d3670634
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1164(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1164(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: mul r0.xy, cb0[0].xyxx, cb0[4].zzzz
    r0.xy = ((source[0].xyxx)*(source[4].zzzz)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: mul r1.xy, r1.xyxx, cb0[4].xxxx
    r1.xy = ((r1.xyxx)*(source[4].xxxx)).xy;
    // 8: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 9: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 11: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 12: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 14: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 15: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 16: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r2.xyz, r0.wwww, v1.xyzx
    r2.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 19: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r3.xyz, r0.wwww, v0.xyzx
    r3.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 22: mul r4.xyz, r2.zxyz, r3.yzxy
    r4.xyz = ((r2.zxyz)*(r3.yzxy)).xyz;
    // 23: mad r4.xyz, r2.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r2.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 24: mul r4.xyz, r4.xyzx, v1.wwww
    r4.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 25: dp3 r5.y, r4.xyzx, r1.xyzx
    r5.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 26: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 27: dp3 r5.x, r3.xyzx, r1.xyzx
    r5.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 29: dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 30: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mad r0.x, r0.x, l(0.500000), cb0[5].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].yyyy)).x;
    // 32: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 35: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 36: mul r0.zw, v4.xxxy, cb0[5].zzzz
    r0.zw = ((v4.xxxy)*(source[5].zzzz)).zw;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 39: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 40: max r0.z, cb0[4].y, l(0.000000)
    r0.z = (max(source[4].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 41: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 42: mul r0.w, r0.y, r0.z
    r0.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 43: mad r0.x, r0.x, r0.w, r0.x
    r0.x = ((r0.xxxx)*(r0.wwww)+(r0.xxxx)).x;
    // 44: add r0.w, -r0.z, r0.x
    r0.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 45: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 46: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 47: mad r0.x, -r0.z, r0.w, r0.x
    r0.x = ((-(r0.zzzz))*(r0.wwww)+(r0.xxxx)).x;
    // 48: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 49: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 50: dp3 r0.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r0.yzw, -r5.xxyz, r0.yyyy
    r0.yzw = ((-(r5.xxyz))+(r0.yyyy)).yzw;
    // 52: mad r0.yzw, cb0[6].xxxx, r0.yyzw, r5.xxyz
    r0.yzw = ((source[6].xxxx)*(r0.yyzw)+(r5.xxyz)).yzw;
    // 53: mul r5.xyz, cb0[3].xyzx, cb0[6].zzzz
    r5.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 54: mul r7.xyz, r6.xyzx, r5.xyzx
    r7.xyz = ((r6.xyzx)*(r5.xyzx)).xyz;
    // 55: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: mad r5.xyz, -r5.xyzx, r6.xyzx, r1.wwww
    r5.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 57: mad r5.xyz, cb0[7].xxxx, r5.xyzx, r7.xyzx
    r5.xyz = ((source[7].xxxx)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 58: mul r6.xyz, cb0[2].xyzx, cb0[6].yyyy
    r6.xyz = ((source[2].xyzx)*(source[6].yyyy)).xyz;
    // 59: mad r5.xyz, -r0.yzwy, r6.xyzx, r5.xyzx
    r5.xyz = ((-(r0.yzwy))*(r6.xyzx)+(r5.xyzx)).xyz;
    // 60: mul r0.yzw, r0.yyzw, r6.xxyz
    r0.yzw = ((r0.yyzw)*(r6.xxyz)).yzw;
    // 61: mad r0.yzw, r0.xxxx, r5.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r5.xxyz)+(r0.yyzw)).yzw;
    // 62: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 63: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 64: add r5.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 65: mad r1.xyz, r0.xxxx, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 66: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 68: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 69: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 70: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 71: mul r5.xyz, r0.xxxx, v6.xyzx
    r5.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 72: dp3 r0.x, r5.xyzx, r1.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 73: mad r5.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 74: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 75: mul r5.yzw, r5.yyyy, cb0[9].xxyz
    r5.yzw = ((r5.yyyy)*(source[9].xxyz)).yzw;
    // 76: mad r5.xyz, r5.xxxx, cb0[8].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[8].xyzx)+(r5.yzwy)).xyz;
    // 77: mul r5.xyz, r5.xyzx, cb0[10].wwww
    r5.xyz = ((r5.xyzx)*(source[10].wwww)).xyz;
    // 78: mad r6.xyz, r5.xyzx, r0.yzwy, cb0[1].xyzx
    r6.xyz = ((r5.xyzx)*(r0.yzwy)+(source[1].xyzx)).xyz;
    // 79: mul r5.xyz, r0.yzwy, r5.xyzx
    r5.xyz = ((r0.yzwy)*(r5.xyzx)).xyz;
    // 80: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 81: mad o0.xyz, r0.yzwy, cb0[10].xyzx, r6.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[10].xyzx)+(r6.xyzx)).xyz;
    // 82: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 83: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 84: dp3 r0.x, r3.xyzx, r1.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 85: dp3 r0.y, r4.xyzx, r1.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 86: dp3 r0.z, r2.xyzx, r1.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 87: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 88: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 89: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 90: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 91: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 92: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 93: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 94: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 95: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 96: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 97: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 98: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 99: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 100: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 101: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 102: ret
    return output;
}

// source.character.static-map-native-1165.v1 / source program 19257930632a36468f4f1a80b05e5c42
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1165(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[6]=1.f;
    source[7]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 4: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 5: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 8: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 9: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 10: mad r1.xyz, r1.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r1.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 11: dp2_sat r2.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 12: dp3_sat r2.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 13: dp3_sat r2.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 14: mul r1.xyz, r2.xyzx, r2.xyzx
    r1.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 15: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 16: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 17: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 18: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t2.xyzw, s1
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 19: mul r2.xyz, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((r2.xyzx)*(source[7].xyzx)).xyz;
    // 20: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 21: dp3 r1.x, r2.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r1.x = (dot((r2.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).x;
    // 22: dp3 r1.y, v6.xyzx, v6.xyzx
    r1.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 23: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 24: mul r1.y, r1.y, v6.z
    r1.y = ((r1.yyyy)*(v6.zzzz)).y;
    // 25: mad r1.yz, r1.yyyy, l(0.000000, 0.500000, -0.500000, 0.000000), l(0.000000, 0.500000, 0.500000, 0.000000)
    r1.yz = ((r1.yyyy)*(float4(0.000000,0.500000,-0.500000,0.000000))+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 26: mul r1.yz, r1.yyzy, r1.yyzy
    r1.yz = ((r1.yyzy)*(r1.yyzy)).yz;
    // 27: mul r2.xyz, r1.zzzz, cb0[4].xyzx
    r2.xyz = ((r1.zzzz)*(source[4].xyzx)).xyz;
    // 28: mad r1.yzw, r1.yyyy, cb0[3].xxyz, r2.xxyz
    r1.yzw = ((r1.yyyy)*(source[3].xxyz)+(r2.xxyz)).yzw;
    // 29: mul r1.yzw, r1.yyzw, cb0[5].wwww
    r1.yzw = ((r1.yyzw)*(source[5].wwww)).yzw;
    // 30: add r2.xyz, -cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[0].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 31: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 32: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 33: mul r3.xyz, r1.yzwy, r2.xyzx
    r3.xyz = ((r1.yzwy)*(r2.xyzx)).xyz;
    // 34: sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t1.xyzw, s1
    r4.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 35: mul r4.xyz, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((r4.xyzx)*(source[6].xyzx)).xyz;
    // 36: mul r5.xyz, r1.xxxx, r4.xyzx
    r5.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 37: mad r1.xyz, r4.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((r4.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 38: mul r4.xyz, r4.xyzx, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].xyzx)).xyz;
    // 39: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 40: div r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)/(r1.xyzx)).xyz;
    // 41: mad r3.xyz, r2.xyzx, r5.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 42: mad r3.xyz, r4.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r4.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 43: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 44: dp3 o4.x, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 45: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 46: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 47: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 48: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 49: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r0.xyzx)).xyz;
    // 50: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 51: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 52: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 53: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 54: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 55: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 56: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 57: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 58: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 59: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 60: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 61: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 62: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 63: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 64: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 65: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 66: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 67: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 68: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 69: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 70: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 71: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 72: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 73: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 74: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 75: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 76: ftou r0.x, cb0[2].z
    r0.x = (asfloat((uint4)(source[2].zzzz))).x;
    // 77: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 78: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 79: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 80: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 81: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 82: ret
    return output;
}

// source.character.static-map-native-1165.v1 / source program b0951d28638edf4cb5fab09e5a93ab40
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1165(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1165(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 2: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 3: add r0.xyz, r0.xyzx, cb0[0].xyzx
    r0.xyz = ((r0.xyzx)+(source[0].xyzx)).xyz;
    // 4: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 5: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 8: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 9: mul r0.w, r0.w, v6.z
    r0.w = ((r0.wwww)*(v6.zzzz)).w;
    // 10: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 11: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 12: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 13: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 14: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 15: add r2.xyz, -cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[0].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 16: mul r2.xyz, r2.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 17: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 18: mad r0.xyz, r1.xyzx, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 19: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 20: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 21: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r0.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r0.xyzx)).xyz;
    // 22: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 23: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 24: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 25: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 26: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 27: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 30: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 31: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 32: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 33: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 34: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 35: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 36: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 37: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 38: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 39: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 40: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 41: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 42: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 43: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 44: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 45: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 46: mov o3.w, l(0.030000)
    output.targets[3].w = (float4(0.030000,0.030000,0.030000,0.030000)).w;
    // 47: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 48: ftou r0.x, cb0[2].z
    r0.x = (asfloat((uint4)(source[2].zzzz))).x;
    // 49: bfi r0.x, l(5), l(0), r0.x, l(32)
    r0.x = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.xxxx),uint4(32u,32u,32u,32u))).x;
    // 50: utof r0.x, r0.x
    r0.x = ((float4)(asuint(r0.xxxx))).x;
    // 51: mul o5.w, r0.x, l(0.003922)
    output.targets[5].w = ((r0.xxxx)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 52: mul_sat r0.xyz, cb2[4].xyzx, l(0.100000, 0.100000, 0.100000, 0.000000)
    r0.xyz = (saturate((passValues[4].xyzx)*(float4(0.100000,0.100000,0.100000,0.000000)))).xyz;
    // 53: sqrt o5.xyz, r0.xyzx
    output.targets[5].xyz = (sqrt(r0.xyzx)).xyz;
    // 54: ret
    return output;
}

// source.character.static-map-native-1166.v1 / source program e3014d7053be164ab345b80d4e351cea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1166(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: mul r1.xy, v4.xyxx, cb0[1].xyxx
    r1.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 7: mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: mul r1.xy, r1.xyxx, cb0[4].wwww
    r1.xy = ((r1.xyxx)*(source[4].wwww)).xy;
    // 10: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 21: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 22: mul r3.xyz, r0.wwww, r1.xyzx
    r3.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 23: mad r0.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 24: dp2_sat r3.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 25: dp3_sat r3.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 26: dp3_sat r3.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 27: log r0.xyz, r3.xyzx
    r0.xyz = (log2(r3.xyzx)).xyz;
    // 28: add r0.w, cb0[5].z, l(1.000000)
    r0.w = ((source[5].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 30: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 31: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s2
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 32: mul r3.xyz, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((r3.xyzx)*(source[10].xyzx)).xyz;
    // 33: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: dp2_sat r4.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 35: dp3_sat r4.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 36: dp3_sat r4.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 37: mul r0.yzw, r4.xxyz, r4.xxyz
    r0.yzw = ((r4.xxyz)*(r4.xxyz)).yzw;
    // 38: dp3 r0.y, r3.xyzx, r0.yzwy
    r0.y = (dot((r3.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 39: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s2
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 40: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // 41: mul r4.xyz, r0.yyyy, r3.xyzx
    r4.xyz = ((r0.yyyy)*(r3.xyzx)).xyz;
    // 42: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 43: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 44: mul r5.xyz, r0.zzzz, v6.xyzx
    r5.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 45: dp3 r0.z, r5.xyzx, r1.xyzx
    r0.z = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 46: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 47: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 48: mul r5.xyz, r0.wwww, cb0[7].xyzx
    r5.xyz = ((r0.wwww)*(source[7].xyzx)).xyz;
    // 49: mad r5.xyz, r0.zzzz, cb0[6].xyzx, r5.xyzx
    r5.xyz = ((r0.zzzz)*(source[6].xyzx)+(r5.xyzx)).xyz;
    // 50: mul r5.xyz, r5.xyzx, cb0[8].wwww
    r5.xyz = ((r5.xyzx)*(source[8].wwww)).xyz;
    // 51: mul r6.xyz, cb0[2].xyzx, cb0[5].xxxx
    r6.xyz = ((source[2].xyzx)*(source[5].xxxx)).xyz;
    // 52: mul r6.xyz, r2.xyzx, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 53: mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 54: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 55: mad r0.yzw, r3.xxyz, r0.yyyy, r5.xxyz
    r0.yzw = ((r3.xxyz)*(r0.yyyy)+(r5.xxyz)).yzw;
    // 56: add r0.yzw, r0.yyzw, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 57: div r0.yzw, r4.xxyz, r0.yyzw
    r0.yzw = ((r4.xxyz)/(r0.yyzw)).yzw;
    // 58: mad r4.xyz, r6.xyzx, r4.xyzx, r7.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 59: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 60: mul r5.xyz, cb0[3].xyzx, cb0[5].yyyy
    r5.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 61: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 62: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 63: mul r2.xyz, r3.xyzx, r2.xyzx
    r2.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 64: mad r3.xyz, r2.xyzx, r0.xxxx, r4.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 65: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 66: dp3 o4.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 67: add r0.xzw, r3.xxyz, cb0[0].xxyz
    r0.xzw = ((r3.xxyz)+(source[0].xxyz)).xzw;
    // 68: mad o0.xyz, r6.xyzx, cb0[8].xyzx, r0.xzwx
    output.targets[0].xyz = ((r6.xyzx)*(source[8].xyzx)+(r0.xzwx)).xyz;
    // 69: mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // 70: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 71: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 72: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 73: mul r0.xzw, r0.xxxx, v1.xxyz
    r0.xzw = ((r0.xxxx)*(v1.xxyz)).xzw;
    // 74: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 75: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 76: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 77: mul r4.xyz, r0.wxzw, r2.yzxy
    r4.xyz = ((r0.wxzw)*(r2.yzxy)).xyz;
    // 78: mad r4.xyz, r0.zwxz, r2.zxyz, -r4.xyzx
    r4.xyz = ((r0.zwxz)*(r2.zxyz)+(-(r4.xyzx))).xyz;
    // 79: dp3 r5.z, r0.xzwx, r1.xyzx
    r5.z = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 80: dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 81: mul r0.xzw, r4.xxyz, v1.wwww
    r0.xzw = ((r4.xxyz)*(v1.wwww)).xzw;
    // 82: dp3 r5.y, r0.xzwx, r1.xyzx
    r5.y = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 83: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 84: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 85: mul r0.xzw, r0.xxxx, r5.xxyz
    r0.xzw = ((r0.xxxx)*(r5.xxyz)).xzw;
    // 86: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 87: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 88: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 89: ge r1.yz, r0.xxzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxzx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 90: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 91: mad r1.yz, -|r0.zzxz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzxz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 92: movc r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.xxzx)).xz;
    // 93: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 94: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 95: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 96: mul o4.z, r0.y, r3.x
    output.targets[4].z = ((r0.yyyy)*(r3.xxxx)).z;
    // 97: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 98: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 99: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 100: ret
    return output;
}

// source.character.static-map-native-1166.v1 / source program e92a9cb5a1fb8c43adba03497419eaf3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1166(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1166(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[4];
    source[4]=g_SourceCharacterBaseConstants[5];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v6.xyzx
    r0.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 4: mul r1.xy, v4.xyxx, cb0[1].xyxx
    r1.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.xyxx, t0.zwxy, s0, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 7: mad r1.xy, r1.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 8: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 9: mul r1.xy, r1.xyxx, cb0[3].wwww
    r1.xy = ((r1.xyxx)*(source[3].wwww)).xy;
    // 10: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 11: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 12: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 13: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 14: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 15: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 16: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 17: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 18: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 19: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 20: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 21: dp3 r0.x, r0.xyzx, r1.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 22: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 23: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 24: mul r0.yzw, r0.yyyy, cb0[6].xxyz
    r0.yzw = ((r0.yyyy)*(source[6].xxyz)).yzw;
    // 25: mad r0.xyz, r0.xxxx, cb0[5].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[5].xyzx)+(r0.yzwy)).xyz;
    // 26: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 27: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 28: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 29: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 30: mad r3.xyz, r0.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r0.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 32: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 33: mad o0.xyz, r2.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 34: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 35: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 36: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 37: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 38: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 39: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 40: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 41: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 42: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 43: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 44: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 45: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 46: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 47: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 48: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 51: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 52: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 53: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 54: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 55: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 56: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 57: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 58: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 59: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 60: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 61: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 62: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 63: ret
    return output;
}

// source.character.static-map-native-1400.v1 / source program 34d101656e12f1449b346b9e5ae789b3
