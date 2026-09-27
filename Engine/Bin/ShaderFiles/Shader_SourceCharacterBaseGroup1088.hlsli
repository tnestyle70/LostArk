#ifndef SOURCE_CHARACTER_BASE_DISPATCH_CASES
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1100(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    // 26: mul r3.xyz, cb0[3].xyzx, cb0[5].xxxx
    r3.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
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
    // 37: mad r5.xyz, cb0[4].yyyy, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = ((source[4].yyyy)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 38: mad r4.yzw, r4.zzzz, r5.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r4.yzw = ((r4.zzzz)*(r5.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 39: mul r5.xyz, r4.xxxx, cb0[2].xyzx
    r5.xyz = ((r4.xxxx)*(source[2].xyzx)).xyz;
    // 40: mul r5.xyz, r5.xyzx, cb0[4].zzzz
    r5.xyz = ((r5.xyzx)*(source[4].zzzz)).xyz;
    // 41: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 42: mul r0.xyz, r0.xyzx, r4.yzwy
    r0.xyz = ((r0.xyzx)*(r4.yzwy)).xyz;
    // 43: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 44: mul r4.xyz, r0.xyzx, r1.xyzx
    r4.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 45: dp2_sat r7.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 46: dp3_sat r7.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 47: dp3_sat r7.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 48: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 49: mad r3.xyz, r7.xyzx, r3.xyzx, r6.xyzx
    r3.xyz = ((r7.xyzx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 50: sample_indexable(texture2d)(float,float,float,float) r6.xyz, v3.zwzz, t4.xyzw, s3
    r6.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 51: mul r6.xyz, r6.xyzx, cb0[10].xyzx
    r6.xyz = ((r6.xyzx)*(source[10].xyzx)).xyz;
    // 52: dp3 r0.w, r6.xyzx, r3.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 53: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 54: mul r3.xyz, r3.xyzx, cb0[9].xyzx
    r3.xyz = ((r3.xyzx)*(source[9].xyzx)).xyz;
    // 55: mul r7.xyz, r0.wwww, r3.xyzx
    r7.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 56: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 57: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 58: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 59: div r1.xyz, r7.xyzx, r1.xyzx
    r1.xyz = ((r7.xyzx)/(r1.xyzx)).xyz;
    // 60: mad r4.xyz, r0.xyzx, r7.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 61: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 62: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 63: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 64: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 65: dp3 r1.w, r2.xyzx, r1.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 66: mul r5.xyz, r1.wwww, r2.xyzx
    r5.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 67: mad r1.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 68: dp2_sat r5.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 69: dp3_sat r5.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 70: dp3_sat r5.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 71: log r1.xyz, r5.xyzx
    r1.xyz = (log2(r5.xyzx)).xyz;
    // 72: add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 73: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 74: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 75: dp3 r1.x, r6.xyzx, r1.xyzx
    r1.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 76: mad r1.yzw, r3.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r3.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 77: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 78: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 79: add r3.xyz, r1.yzwy, cb0[0].xyzx
    r3.xyz = ((r1.yzwy)+(source[0].xyzx)).xyz;
    // 80: mad o0.xyz, r0.xyzx, cb0[8].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)+(r3.xyzx)).xyz;
    // 81: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 82: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 83: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 84: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 85: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 86: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 87: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 88: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 89: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 90: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 91: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 92: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 93: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 94: dp3 r0.y, r3.xyzx, r2.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 95: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 96: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 97: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 98: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 99: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 100: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 101: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 102: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 103: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 104: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 105: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 106: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 107: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 108: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 109: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 110: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 111: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 112: ret
    return output;
}

// source.character.static-map-native-1100.v1 / source program 644042db50951247b8e9d1b5e34d50e1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1100(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1100(input);
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
    // 26: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
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
    // 43: mad o0.xyz, r0.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
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

// source.character.static-map-native-1101.v1 / source program ab8f9810e121b74cb9ba26d7f0c62bf0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1101(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
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
    // 4: mul r0.xy, r0.xyxx, cb0[7].xxxx
    r0.xy = ((r0.xyxx)*(source[7].xxxx)).xy;
    // 5: mul r1.xy, r0.xyxx, v2.wwww
    r1.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 6: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 7: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 8: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 9: add r1.z, r0.x, l(0.000010)
    r1.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 10: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 11: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 12: div r0.xyz, r1.xyzx, r0.xxxx
    r0.xyz = ((r1.xyzx)/(r0.xxxx)).xyz;
    // 13: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r1.xyz, r0.wwww, r0.xyzx
    r1.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 16: dp2_sat r2.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 17: dp3_sat r2.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 18: dp3_sat r2.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 19: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 20: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t8.xyzw, s5
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 21: mul r3.xyz, r3.xyzx, cb0[25].xyzx
    r3.xyz = ((r3.xyzx)*(source[25].xyzx)).xyz;
    // 22: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 23: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t7.xyzw, s5
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 24: mul r2.xyz, r2.xyzx, cb0[24].xyzx
    r2.xyz = ((r2.xyzx)*(source[24].xyzx)).xyz;
    // 25: mul r4.xyz, r0.wwww, r2.xyzx
    r4.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 26: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 29: dp3 r1.w, r5.xyzx, r1.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 30: dp3 r2.w, -r5.xyzx, r1.xyzx
    r2.w = (dot((-(r5.xyzx)).xyz,(r1.xyzx).xyz).xxxx).w;
    // 31: mad r5.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: mad r5.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r5.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 33: mul r5.xyzw, r5.xyzw, r5.xyzw
    r5.xyzw = ((r5.xyzw)*(r5.xyzw)).xyzw;
    // 34: mul r6.xyz, r5.wwww, cb0[22].xyzx
    r6.xyz = ((r5.wwww)*(source[22].xyzx)).xyz;
    // 35: mad r6.xyz, r5.zzzz, cb0[21].xyzx, r6.xyzx
    r6.xyz = ((r5.zzzz)*(source[21].xyzx)+(r6.xyzx)).xyz;
    // 36: mul r6.xyz, r6.xyzx, cb0[23].wwww
    r6.xyz = ((r6.xyzx)*(source[23].wwww)).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul r1.w, r7.z, cb0[8].x
    r1.w = ((r7.zzzz)*(source[8].xxxx)).w;
    // 39: mul r5.zw, r7.yyyx, cb0[10].xxxz
    r5.zw = ((r7.yyyx)*(source[10].xxxz)).zw;
    // 40: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 41: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 42: mul r2.w, r2.w, cb0[8].y
    r2.w = ((r2.wwww)*(source[8].yyyy)).w;
    // 43: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 44: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 45: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: mul r7.xyz, cb0[5].xyzx, cb0[7].yyyy
    r7.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 49: add r8.xy, r8.wwww, cb0[9].zxzz
    r8.xy = ((r8.wwww)+(source[9].zxzz)).xy;
    // 50: mul r9.xyz, r7.xyzx, cb0[7].zzzz
    r9.xyz = ((r7.xyzx)*(source[7].zzzz)).xyz;
    // 51: mad r7.xyz, cb0[7].wwww, r7.xyzx, -r9.xyzx
    r7.xyz = ((source[7].wwww)*(r7.xyzx)+(-(r9.xyzx))).xyz;
    // 52: mad r7.xyz, r2.wwww, r7.xyzx, r9.xyzx
    r7.xyz = ((r2.wwww)*(r7.xyzx)+(r9.xyzx)).xyz;
    // 53: add r9.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 54: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 55: mad_sat r7.xyz, r7.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = (saturate((r7.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 56: mul r9.xyz, r6.xyzx, r7.xyzx
    r9.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 57: mad r4.xyz, r7.xyzx, r4.xyzx, r9.xyzx
    r4.xyz = ((r7.xyzx)*(r4.xyzx)+(r9.xyzx)).xyz;
    // 58: mad r9.xyz, r7.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r9.xyz = ((r7.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 59: mad r10.xyz, r7.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r10.xyz = ((r7.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 60: log r8.zw, |r5.zzzw|
    r8.zw = (log2(abs(r5.zzzw))).zw;
    // 61: lt r5.zw, |r5.zzzw|, l(0.000000, 0.000000, 0.000001, 0.000001)
    r5.zw = (asfloat((uint4)((abs(r5.zzzw))<(float4(0.000000,0.000000,0.000001,0.000001))) * 0xffffffffu)).zw;
    // 62: mul r8.zw, r8.zzzw, cb0[10].yyyw
    r8.zw = ((r8.zzzw)*(source[10].yyyw)).zw;
    // 63: exp r8.zw, r8.zzzw
    r8.zw = (exp2(r8.zzzw)).zw;
    // 64: min r2.w, r8.w, l(1.000000)
    r2.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: movc r3.w, r5.z, l(0), r8.z
    r3.w = ((asuint(r5.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r8.zzzz)).w;
    // 66: movc r2.w, r5.w, l(0), r2.w
    r2.w = ((asuint(r5.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 67: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 68: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 69: mad r9.xyz, r2.wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 70: mad r10.xyz, r7.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r10.xyz = ((r7.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 71: mad r9.xyz, r9.xyzx, r2.wwww, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 72: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 73: max r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = (max(r2.wwww,r9.xyzx)).xyz;
    // 74: mul r4.xyz, r4.xyzx, r9.xyzx
    r4.xyz = ((r4.xyzx)*(r9.xyzx)).xyz;
    // 75: dp3 r4.w, v1.xyzx, v1.xyzx
    r4.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 76: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 77: mul r10.xyz, r4.wwww, v1.xyzx
    r10.xyz = ((r4.wwww)*(v1.xyzx)).xyz;
    // 78: dp3 r4.w, v0.xyzx, v0.xyzx
    r4.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 79: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 80: mul r11.xyz, r4.wwww, v0.xyzx
    r11.xyz = ((r4.wwww)*(v0.xyzx)).xyz;
    // 81: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 82: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 83: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 84: dp3 r13.y, r12.xyzx, r1.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 85: dp3 r13.x, r11.xyzx, r1.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 86: dp2 r14.z, r13.xyxx, cb0[13].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 87: mul r5.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r5.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 88: dp2 r14.x, r13.xyxx, r5.zwzz
    r14.x = (dot((r13.xyxx).xy,(r5.zwzz).xy).xxxx).x;
    // 89: dp3 r14.y, r10.xyzx, r1.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 90: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 91: dp4 r13.x, cb0[14].xyzw, r14.xyzw
    r13.x = (dot((source[14].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 92: dp4 r13.y, cb0[15].xyzw, r14.xyzw
    r13.y = (dot((source[15].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 93: dp4 r13.z, cb0[16].xyzw, r14.xyzw
    r13.z = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 94: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 95: mul r4.w, r14.y, r14.y
    r4.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 96: mad r4.w, r14.x, r14.x, -r4.w
    r4.w = ((r14.xxxx)*(r14.xxxx)+(-(r4.wwww))).w;
    // 97: dp4 r14.x, cb0[17].xyzw, r15.xyzw
    r14.x = (dot((source[17].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 98: dp4 r14.y, cb0[18].xyzw, r15.xyzw
    r14.y = (dot((source[18].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 99: dp4 r14.z, cb0[19].xyzw, r15.xyzw
    r14.z = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 100: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 101: mad r13.xyz, cb0[20].xyzx, r4.wwww, r13.xyzx
    r13.xyz = ((source[20].xyzx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 102: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 103: mul r13.xyz, r13.xyzx, cb0[12].xyzx
    r13.xyz = ((r13.xyzx)*(source[12].xyzx)).xyz;
    // 104: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 105: dp3 r4.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r4.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 106: dp3 r6.w, v6.xyzx, v6.xyzx
    r6.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 107: rsq r6.w, r6.w
    r6.w = (rsqrt(r6.wwww)).w;
    // 108: mul r14.xyz, r6.wwww, v6.xyzx
    r14.xyz = ((r6.wwww)*(v6.xyzx)).xyz;
    // 109: dp3 r6.w, r1.xyzx, r14.xyzx
    r6.w = (dot((r1.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 110: mul r1.xyz, r1.xyzx, r6.wwww
    r1.xyz = ((r1.xyzx)*(r6.wwww)).xyz;
    // 111: mad r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 112: deriv_rtx_coarse r15.x, r6.w
    r15.x = (ddx_coarse(r6.wwww)).x;
    // 113: deriv_rty_coarse r15.y, r6.w
    r15.y = (ddy_coarse(r6.wwww)).y;
    // 114: dp2 r7.w, r15.xyxx, r15.xyxx
    r7.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 115: sqrt r7.w, r7.w
    r7.w = (sqrt(r7.wwww)).w;
    // 116: mad_sat r15.y, r7.w, l(0.300000), r3.w
    r15.y = (saturate((r7.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r3.wwww))).y;
    // 117: mad r7.w, r15.y, l(0.200000), l(0.200000)
    r7.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 118: div r4.w, r4.w, r7.w
    r4.w = ((r4.wwww)/(r7.wwww)).w;
    // 119: dp3 r8.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 120: mad r4.w, r8.z, l(5.000000), r4.w
    r4.w = ((r8.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r4.wwww)).w;
    // 121: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r8.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 122: mad r8.x, r8.w, -r8.x, r8.x
    r8.x = ((r8.wwww)*(-(r8.xxxx))+(r8.xxxx)).x;
    // 123: add_sat r1.w, r1.w, r8.x
    r1.w = (saturate((r1.wwww)+(r8.xxxx))).w;
    // 124: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 125: add_sat r4.w, r1.w, r4.w
    r4.w = (saturate((r1.wwww)+(r4.wwww))).w;
    // 126: mad r8.x, r4.w, l(-2.000000), l(3.000000)
    r8.x = ((r4.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 127: mul r4.w, r4.w, r4.w
    r4.w = ((r4.wwww)*(r4.wwww)).w;
    // 128: mul r4.w, r4.w, r8.x
    r4.w = ((r4.wwww)*(r8.xxxx)).w;
    // 129: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 130: mul r4.w, r4.w, l(1.500000)
    r4.w = ((r4.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 131: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 132: mul r13.xyz, r4.wwww, r13.xyzx
    r13.xyz = ((r4.wwww)*(r13.xyzx)).xyz;
    // 133: mul r4.xyz, r4.xyzx, r13.xyzx
    r4.xyz = ((r4.xyzx)*(r13.xyzx)).xyz;
    // 134: add r4.w, -r8.y, l(1.000000)
    r4.w = ((-(r8.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: mad_sat r4.w, r8.w, r4.w, r8.y
    r4.w = (saturate((r8.wwww)*(r4.wwww)+(r8.yyyy))).w;
    // 136: mad r4.w, -r4.w, cb0[2].x, l(1.000000)
    r4.w = ((-(r4.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 137: mul r8.x, r15.y, r15.y
    r8.x = ((r15.yyyy)*(r15.yyyy)).x;
    // 138: mad r8.y, r8.x, l(0.350000), l(1.000000)
    r8.y = ((r8.xxxx)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 139: div_sat r4.w, r4.w, r8.y
    r4.w = (saturate((r4.wwww)/(r8.yyyy))).w;
    // 140: add r8.y, r6.w, l(1.000000)
    r8.y = ((r6.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 141: mov_sat r6.w, r6.w
    r6.w = (saturate(r6.wwww)).w;
    // 142: log r6.w, r6.w
    r6.w = (log2(r6.wwww)).w;
    // 143: mul r6.w, r6.w, cb0[1].y
    r6.w = ((r6.wwww)*(source[1].yyyy)).w;
    // 144: exp r6.w, r6.w
    r6.w = (exp2(r6.wwww)).w;
    // 145: mad_sat r6.w, r6.w, cb0[1].w, cb0[1].z
    r6.w = (saturate((r6.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 146: add r8.w, r1.z, l(1.000000)
    r8.w = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: min r8.w, r8.w, l(1.000000)
    r8.w = (min(r8.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 148: add_sat r15.x, -r8.w, r8.y
    r15.x = (saturate((-(r8.wwww))+(r8.yyyy))).x;
    // 149: sample_indexable(texture2d)(float,float,float,float) r8.yw, r15.xyxx, t5.zxwy, s7
    r8.yw = ((float4(0.0,0.0,0.0,0.0)).zxwy).yw;
    // 150: add r15.zw, -r15.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r15.zw = ((-(r15.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 151: mul r9.w, r15.y, l(5.000000)
    r9.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 152: add r10.w, r2.w, r15.x
    r10.w = ((r2.wwww)+(r15.xxxx)).w;
    // 153: log r10.w, r10.w
    r10.w = (log2(r10.wwww)).w;
    // 154: mul r8.x, r8.x, r10.w
    r8.x = ((r8.xxxx)*(r10.wwww)).x;
    // 155: exp r8.x, r8.x
    r8.x = (exp2(r8.xxxx)).x;
    // 156: add r2.w, r2.w, r8.x
    r2.w = ((r2.wwww)+(r8.xxxx)).w;
    // 157: add_sat r2.w, r2.w, l(-1.000000)
    r2.w = (saturate((r2.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 158: mov_sat r8.x, cb0[8].z
    r8.x = (saturate(source[8].zzzz)).x;
    // 159: mad r16.xyz, -r8.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r7.xyzx
    r16.xyz = ((-(r8.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r7.xyzx)).xyz;
    // 160: mul r8.x, r8.x, l(0.080000)
    r8.x = ((r8.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 161: mad r16.xyz, r1.wwww, r16.xyzx, r8.xxxx
    r16.xyz = ((r1.wwww)*(r16.xyzx)+(r8.xxxx)).xyz;
    // 162: max r15.xyz, r15.zzzz, r16.xyzx
    r15.xyz = (max(r15.zzzz,r16.xyzx)).xyz;
    // 163: add r15.xyz, -r16.xyzx, r15.xyzx
    r15.xyz = ((-(r16.xyzx))+(r15.xyzx)).xyz;
    // 164: mul_sat r8.x, r16.y, l(50.000000)
    r8.x = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 165: mul r15.xyz, r8.xxxx, r15.xyzx
    r15.xyz = ((r8.xxxx)*(r15.xyzx)).xyz;
    // 166: mul r17.xyz, r8.wwww, r16.xyzx
    r17.xyz = ((r8.wwww)*(r16.xyzx)).xyz;
    // 167: mad r15.xyz, r15.xyzx, r8.yyyy, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r8.yyyy)+(r17.xyzx)).xyz;
    // 168: div r8.y, l(1.000000, 1.000000, 1.000000, 1.000000), r8.w
    r8.y = r8.w != 0.f ? 1.f / r8.w : 0.f;
    // 169: add r8.y, r8.y, l(-1.000000)
    r8.y = ((r8.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 170: mad r17.xyz, r16.xyzx, r8.yyyy, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r8.yyyy)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 171: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 173: mul r8.y, r15.w, r15.w
    r8.y = ((r15.wwww)*(r15.wwww)).y;
    // 174: mul r8.y, r8.y, r8.y
    r8.y = ((r8.yyyy)*(r8.yyyy)).y;
    // 175: mul r8.w, r15.w, r8.y
    r8.w = ((r15.wwww)*(r8.yyyy)).w;
    // 176: mad r8.y, -r8.y, r15.w, l(1.000000)
    r8.y = ((-(r8.yyyy))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 177: mul r17.xyz, r16.xyzx, r8.yyyy
    r17.xyz = ((r16.xyzx)*(r8.yyyy)).xyz;
    // 178: dp3 r8.y, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.y = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 179: mad r16.xyz, r8.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r8.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 180: mad r8.xyw, r8.xxxx, r8.wwww, r17.xyxz
    r8.xyw = ((r8.xxxx)*(r8.wwww)+(r17.xyxz)).xyw;
    // 181: add r8.xyw, -r8.xyxw, l(1.000000, 1.000000, 0.000000, 1.000000)
    r8.xyw = ((-(r8.xyxw))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 182: mul r8.xyw, r8.xyxw, r8.xyxw
    r8.xyw = ((r8.xyxw)*(r8.xyxw)).xyw;
    // 183: mad r17.xyz, -r4.wwww, r8.xywx, r18.xyzx
    r17.xyz = ((-(r4.wwww))*(r8.xywx)+(r18.xyzx)).xyz;
    // 184: mul r18.xyz, r7.xyzx, r18.xyzx
    r18.xyz = ((r7.xyzx)*(r18.xyzx)).xyz;
    // 185: mul r8.xyw, r4.wwww, r8.xyxw
    r8.xyw = ((r4.wwww)*(r8.xyxw)).xyw;
    // 186: mul r8.xyw, r7.xyxz, r8.xyxw
    r8.xyw = ((r7.xyxz)*(r8.xyxw)).xyw;
    // 187: mad r8.xyw, -r8.xyxw, r1.wwww, r8.xyxw
    r8.xyw = ((-(r8.xyxw))*(r1.wwww)+(r8.xyxw)).xyw;
    // 188: mul r4.xyz, r4.xyzx, r17.xyzx
    r4.xyz = ((r4.xyzx)*(r17.xyzx)).xyz;
    // 189: mad r4.xyz, -r4.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(r1.wwww)+(r4.xyzx)).xyz;
    // 190: dp2_sat r17.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r17.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 191: dp3_sat r17.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r17.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 192: dp3_sat r17.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r17.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 193: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 194: dp3 r10.w, r3.xyzx, r17.xyzx
    r10.w = (dot((r3.xyzx).xyz,(r17.xyzx).xyz).xxxx).w;
    // 195: add r0.w, r0.w, -r10.w
    r0.w = ((r0.wwww)+(-(r10.wwww))).w;
    // 196: mad r0.w, r3.w, r0.w, r10.w
    r0.w = ((r3.wwww)*(r0.wwww)+(r10.wwww)).w;
    // 197: mad r6.xyz, r2.xyzx, r0.wwww, r6.xyzx
    r6.xyz = ((r2.xyzx)*(r0.wwww)+(r6.xyzx)).xyz;
    // 198: mad r0.w, r2.w, r16.x, r16.y
    r0.w = ((r2.wwww)*(r16.xxxx)+(r16.yyyy)).w;
    // 199: mad r0.w, r0.w, r2.w, r16.z
    r0.w = ((r0.wwww)*(r2.wwww)+(r16.zzzz)).w;
    // 200: mul r0.w, r2.w, r0.w
    r0.w = ((r2.wwww)*(r0.wwww)).w;
    // 201: max r0.w, r0.w, r2.w
    r0.w = (max(r0.wwww,r2.wwww)).w;
    // 202: mul r6.xyz, r0.wwww, r6.xyzx
    r6.xyz = ((r0.wwww)*(r6.xyzx)).xyz;
    // 203: dp3 r11.x, r11.xyzx, r1.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 204: dp3 r11.y, r12.xyzx, r1.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 205: dp3 r1.y, r10.xyzx, r1.xyzx
    r1.y = (dot((r10.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 206: dp2 r1.x, r11.xyxx, r5.zwzz
    r1.x = (dot((r11.xyxx).xy,(r5.zwzz).xy).xxxx).x;
    // 207: dp2 r1.z, r11.xyxx, cb0[13].xyxx
    r1.z = (dot((r11.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 208: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r1.xyzx, t6.xyzw, s6, r9.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r1.xyzx).xyz, (r9.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 209: mul r1.xyz, r10.xyzx, r10.wwww
    r1.xyz = ((r10.xyzx)*(r10.wwww)).xyz;
    // 210: mul r1.xyz, r1.xyzx, cb0[12].xyzx
    r1.xyz = ((r1.xyzx)*(source[12].xyzx)).xyz;
    // 211: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 212: dp3 r2.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 213: div r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)/(r7.wwww)).w;
    // 214: mad r2.w, r8.z, l(5.000000), r2.w
    r2.w = ((r8.zzzz)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 215: add_sat r2.w, r1.w, r2.w
    r2.w = (saturate((r1.wwww)+(r2.wwww))).w;
    // 216: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: mad r3.w, r2.w, l(-2.000000), l(3.000000)
    r3.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 218: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 219: mul r2.w, r2.w, r3.w
    r2.w = ((r2.wwww)*(r3.wwww)).w;
    // 220: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 221: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 222: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 223: mul r1.xyz, r1.xyzx, r2.wwww
    r1.xyz = ((r1.xyzx)*(r2.wwww)).xyz;
    // 224: mul r6.xyz, r6.xyzx, r1.xyzx
    r6.xyz = ((r6.xyzx)*(r1.xyzx)).xyz;
    // 225: mul r1.xyz, r1.xyzx, r15.xyzx
    r1.xyz = ((r1.xyzx)*(r15.xyzx)).xyz;
    // 226: mad r4.xyz, r6.xyzx, r15.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r15.xyzx)+(r4.xyzx)).xyz;
    // 227: mul r5.yzw, r5.yyyy, cb0[22].xxyz
    r5.yzw = ((r5.yyyy)*(source[22].xxyz)).yzw;
    // 228: mad r5.xyz, r5.xxxx, cb0[21].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[21].xyzx)+(r5.yzwy)).xyz;
    // 229: mul r5.xyz, r5.xyzx, cb0[23].wwww
    r5.xyz = ((r5.xyzx)*(source[23].wwww)).xyz;
    // 230: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 231: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 232: add_sat r6.xyz, r6.xyzx, cb0[11].yyyy
    r6.xyz = (saturate((r6.xyzx)+(source[11].yyyy))).xyz;
    // 233: mul r2.w, r6.x, cb0[11].z
    r2.w = ((r6.xxxx)*(source[11].zzzz)).w;
    // 234: mul_sat r6.xyz, r6.xyzx, cb0[6].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[6].xyzx))).xyz;
    // 235: mul r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)*(r6.wwww)).w;
    // 236: mul r6.xyz, r6.xyzx, r2.wwww
    r6.xyz = ((r6.xyzx)*(r2.wwww)).xyz;
    // 237: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 238: mul r10.xyz, r1.wwww, r18.xyzx
    r10.xyz = ((r1.wwww)*(r18.xyzx)).xyz;
    // 239: mul r10.xyz, r13.xyzx, r10.xyzx
    r10.xyz = ((r13.xyzx)*(r10.xyzx)).xyz;
    // 240: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 241: mad r1.xyz, r1.xyzx, r0.wwww, r9.xyzx
    r1.xyz = ((r1.xyzx)*(r0.wwww)+(r9.xyzx)).xyz;
    // 242: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 243: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 244: dp3 r0.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 245: dp3 r0.w, r3.xyzx, r0.wwww
    r0.w = (dot((r3.xyzx).xyz,(r0.wwww).xyz).xxxx).w;
    // 246: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 247: mul r3.xyz, r7.xyzx, r5.xyzx
    r3.xyz = ((r7.xyzx)*(r5.xyzx)).xyz;
    // 248: mad r2.xyz, r7.xyzx, r2.xyzx, r3.xyzx
    r2.xyz = ((r7.xyzx)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 249: dp3 r0.x, r0.xyzx, r14.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 250: add r0.y, -|r14.z|, l(1.000000)
    r0.y = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 251: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 252: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 253: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 254: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 255: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 256: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 257: mul r0.xzw, r0.xxxx, cb0[4].xxyz
    r0.xzw = ((r0.xxxx)*(source[4].xxyz)).xzw;
    // 258: movc r0.xyz, r0.yyyy, l(0,0,0,0), r0.xzwx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xzwx)).xyz;
    // 259: add r0.xyz, r0.xyzx, cb0[3].xyzx
    r0.xyz = ((r0.xyzx)+(source[3].xyzx)).xyz;
    // 260: add r0.w, -r4.w, l(1.000000)
    r0.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 261: mad r0.xyz, r1.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 262: mul r1.xyz, r4.wwww, r1.xyzx
    r1.xyz = ((r4.wwww)*(r1.xyzx)).xyz;
    // 263: mul r3.xyz, r4.wwww, r2.xyzx
    r3.xyz = ((r4.wwww)*(r2.xyzx)).xyz;
    // 264: mad r0.xyz, r2.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 265: add r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)+(r4.xyzx)).xyz;
    // 266: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 267: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r8.xywx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r8.xywx)).xyz;
    // 268: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 269: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 270: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 271: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 272: ret
    return output;
}

// source.character.static-map-native-1101.v1 / source program 17893cb66b08a3479db258929a9ee340
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1101(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1101(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[12]=g_SourceCharacterEnvironmentColor;source[13]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 2: mul r0.z, r0.z, cb0[8].x
    r0.z = ((r0.zzzz)*(source[8].xxxx)).z;
    // 3: mul r0.xy, r0.yxyy, cb0[10].xzxx
    r0.xy = ((r0.yxyy)*(source[10].xzxx)).xy;
    // 4: log r0.w, |r0.z|
    r0.w = (log2(abs(r0.zzzz))).w;
    // 5: lt r0.z, |r0.z|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 6: mul r0.w, r0.w, cb0[8].y
    r0.w = ((r0.wwww)*(source[8].yyyy)).w;
    // 7: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 8: movc r0.z, r0.z, l(0), r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).z;
    // 9: min r0.w, r0.z, l(1.000000)
    r0.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 10: mul r1.xyz, cb0[5].xyzx, cb0[7].yyyy
    r1.xyz = ((source[5].xyzx)*(source[7].yyyy)).xyz;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 12: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 13: add r2.xy, r2.wwww, cb0[9].zxzz
    r2.xy = ((r2.wwww)+(source[9].zxzz)).xy;
    // 14: mul r3.xyz, r1.xyzx, cb0[7].zzzz
    r3.xyz = ((r1.xyzx)*(source[7].zzzz)).xyz;
    // 15: mad r1.xyz, cb0[7].wwww, r1.xyzx, -r3.xyzx
    r1.xyz = ((source[7].wwww)*(r1.xyzx)+(-(r3.xyzx))).xyz;
    // 16: mad r1.xyz, r0.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 17: add r3.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 18: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 19: mad_sat r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = (saturate((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 20: mad r3.xyz, r1.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r3.xyz = ((r1.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 21: mad r4.xyz, r1.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 22: log r2.zw, |r0.xxxy|
    r2.zw = (log2(abs(r0.xxxy))).zw;
    // 23: lt r0.xy, |r0.xyxx|, l(0.000001, 0.000001, 0.000000, 0.000000)
    r0.xy = (asfloat((uint4)((abs(r0.xyxx))<(float4(0.000001,0.000001,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 24: mul r2.zw, r2.zzzw, cb0[10].yyyw
    r2.zw = ((r2.zzzw)*(source[10].yyyw)).zw;
    // 25: exp r2.zw, r2.zzzw
    r2.zw = (exp2(r2.zzzw)).zw;
    // 26: min r0.w, r2.w, l(1.000000)
    r0.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: movc r0.x, r0.x, l(0), r2.z
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 28: movc r0.y, r0.y, l(0), r0.w
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).y;
    // 29: max r0.x, r0.x, cb0[0].x
    r0.x = (max(r0.xxxx,source[0].xxxx)).x;
    // 30: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mad r3.xyz, r0.yyyy, r3.xyzx, r4.xyzx
    r3.xyz = ((r0.yyyy)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 32: mad r4.xyz, r1.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 33: mad r3.xyz, r3.xyzx, r0.yyyy, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r0.yyyy)+(r4.xyzx)).xyz;
    // 34: mul r3.xyz, r0.yyyy, r3.xyzx
    r3.xyz = ((r0.yyyy)*(r3.xyzx)).xyz;
    // 35: max r3.xyz, r0.yyyy, r3.xyzx
    r3.xyz = (max(r0.yyyy,r3.xyzx)).xyz;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 37: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 38: dp2 r0.w, r2.zwzz, r2.zwzz
    r0.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 39: mul r2.zw, r2.zzzw, cb0[7].xxxx
    r2.zw = ((r2.zzzw)*(source[7].xxxx)).zw;
    // 40: mul r4.xy, r2.zwzz, v2.wwww
    r4.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 41: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 44: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: div r4.xyz, r4.xyzx, r0.wwww
    r4.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 48: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r5.xyz, r0.wwww, r4.xyzx
    r5.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 51: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 52: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 53: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 54: dp3 r0.w, r6.xyzx, r5.xyzx
    r0.w = (dot((r6.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 55: mad r2.zw, r0.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r0.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 56: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 57: mul r7.xyz, r2.wwww, cb0[22].xyzx
    r7.xyz = ((r2.wwww)*(source[22].xyzx)).xyz;
    // 58: mad r7.xyz, r2.zzzz, cb0[21].xyzx, r7.xyzx
    r7.xyz = ((r2.zzzz)*(source[21].xyzx)+(r7.xyzx)).xyz;
    // 59: mul r7.xyz, r7.xyzx, cb0[23].wwww
    r7.xyz = ((r7.xyzx)*(source[23].wwww)).xyz;
    // 60: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 61: mul r7.xyz, r3.xyzx, r7.xyzx
    r7.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 62: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 63: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 64: mul r8.xyz, r0.wwww, v1.xyzx
    r8.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 65: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 66: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 67: mul r9.xyz, r0.wwww, v0.xyzx
    r9.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 68: mul r10.xyz, r8.zxyz, r9.yzxy
    r10.xyz = ((r8.zxyz)*(r9.yzxy)).xyz;
    // 69: mad r10.xyz, r8.yzxy, r9.zxyz, -r10.xyzx
    r10.xyz = ((r8.yzxy)*(r9.zxyz)+(-(r10.xyzx))).xyz;
    // 70: mul r10.xyz, r10.xyzx, v1.wwww
    r10.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 71: dp3 r11.y, r10.xyzx, r5.xyzx
    r11.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 72: dp3 r11.x, r9.xyzx, r5.xyzx
    r11.x = (dot((r9.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 73: dp2 r12.z, r11.xyxx, cb0[13].xyxx
    r12.z = (dot((r11.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 74: mul r2.zw, cb0[13].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r2.zw = ((source[13].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 75: dp2 r12.x, r11.xyxx, r2.zwzz
    r12.x = (dot((r11.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 76: dp3 r12.y, r8.xyzx, r5.xyzx
    r12.y = (dot((r8.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 77: mov r12.w, l(1.000000)
    r12.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 78: dp4 r11.x, cb0[14].xyzw, r12.xyzw
    r11.x = (dot((source[14].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 79: dp4 r11.y, cb0[15].xyzw, r12.xyzw
    r11.y = (dot((source[15].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 80: dp4 r11.z, cb0[16].xyzw, r12.xyzw
    r11.z = (dot((source[16].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 81: mul r13.xyzw, r12.yzzx, r12.xyzz
    r13.xyzw = ((r12.yzzx)*(r12.xyzz)).xyzw;
    // 82: mul r0.w, r12.y, r12.y
    r0.w = ((r12.yyyy)*(r12.yyyy)).w;
    // 83: mad r0.w, r12.x, r12.x, -r0.w
    r0.w = ((r12.xxxx)*(r12.xxxx)+(-(r0.wwww))).w;
    // 84: dp4 r12.x, cb0[17].xyzw, r13.xyzw
    r12.x = (dot((source[17].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).x;
    // 85: dp4 r12.y, cb0[18].xyzw, r13.xyzw
    r12.y = (dot((source[18].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).y;
    // 86: dp4 r12.z, cb0[19].xyzw, r13.xyzw
    r12.z = (dot((source[19].xyzw).xyzw,(r13.xyzw).xyzw).xxxx).z;
    // 87: add r11.xyz, r11.xyzx, r12.xyzx
    r11.xyz = ((r11.xyzx)+(r12.xyzx)).xyz;
    // 88: mad r11.xyz, cb0[20].xyzx, r0.wwww, r11.xyzx
    r11.xyz = ((source[20].xyzx)*(r0.wwww)+(r11.xyzx)).xyz;
    // 89: max r11.xyz, r11.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r11.xyz = (max(r11.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: mul r11.xyz, r11.xyzx, cb0[12].xyzx
    r11.xyz = ((r11.xyzx)*(source[12].xyzx)).xyz;
    // 91: mad r11.xyz, r11.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[12].wwww
    r11.xyz = ((r11.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[12].wwww)).xyz;
    // 92: dp3 r0.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 93: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 94: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 95: mul r12.xyz, r1.wwww, v6.xyzx
    r12.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 96: dp3 r1.w, r5.xyzx, r12.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 97: deriv_rtx_coarse r13.x, r1.w
    r13.x = (ddx_coarse(r1.wwww)).x;
    // 98: deriv_rty_coarse r13.y, r1.w
    r13.y = (ddy_coarse(r1.wwww)).y;
    // 99: dp2 r3.w, r13.xyxx, r13.xyxx
    r3.w = (dot((r13.xyxx).xy,(r13.xyxx).xy).xxxx).w;
    // 100: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 101: mad_sat r13.y, r3.w, l(0.300000), r0.x
    r13.y = (saturate((r3.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r0.xxxx))).y;
    // 102: mad r0.x, r13.y, l(0.200000), l(0.200000)
    r0.x = ((r13.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 103: div r0.w, r0.w, r0.x
    r0.w = ((r0.wwww)/(r0.xxxx)).w;
    // 104: dp3 r3.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 105: mad r0.w, r3.w, l(5.000000), r0.w
    r0.w = ((r3.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.wwww)).w;
    // 106: sample_b_indexable(texture2d)(float,float,float,float) r4.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r4.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 107: mad r2.x, r4.w, -r2.x, r2.x
    r2.x = ((r4.wwww)*(-(r2.xxxx))+(r2.xxxx)).x;
    // 108: add_sat r0.z, r0.z, r2.x
    r0.z = (saturate((r0.zzzz)+(r2.xxxx))).z;
    // 109: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 110: add_sat r0.w, r0.z, r0.w
    r0.w = (saturate((r0.zzzz)+(r0.wwww))).w;
    // 111: mad r2.x, r0.w, l(-2.000000), l(3.000000)
    r2.x = ((r0.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).x;
    // 112: mul r0.w, r0.w, r0.w
    r0.w = ((r0.wwww)*(r0.wwww)).w;
    // 113: mul r0.w, r0.w, r2.x
    r0.w = ((r0.wwww)*(r2.xxxx)).w;
    // 114: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 115: mul r0.w, r0.w, l(1.500000)
    r0.w = ((r0.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 116: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 117: mul r11.xyz, r0.wwww, r11.xyzx
    r11.xyz = ((r0.wwww)*(r11.xyzx)).xyz;
    // 118: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 119: mul r14.xyz, r1.wwww, r5.xyzx
    r14.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 120: dp3 r0.w, -r6.xyzx, r5.xyzx
    r0.w = (dot((-(r6.xyzx)).xyz,(r5.xyzx).xyz).xxxx).w;
    // 121: mad r5.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 122: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 123: mad r14.xyz, r14.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r12.xyzx
    r14.xyz = ((r14.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r12.xyzx))).xyz;
    // 124: add r0.w, r14.z, l(1.000000)
    r0.w = ((r14.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: add r2.x, r1.w, l(1.000000)
    r2.x = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 127: mov_sat r1.w, r1.w
    r1.w = (saturate(r1.wwww)).w;
    // 128: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 129: mul r1.w, r1.w, cb0[1].y
    r1.w = ((r1.wwww)*(source[1].yyyy)).w;
    // 130: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 131: mad_sat r1.w, r1.w, cb0[1].w, cb0[1].z
    r1.w = (saturate((r1.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 132: add_sat r13.x, -r0.w, r2.x
    r13.x = (saturate((-(r0.wwww))+(r2.xxxx))).x;
    // 133: sample_indexable(texture2d)(float,float,float,float) r5.zw, r13.xyxx, t5.zwxy, s6
    r5.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 134: add r13.zw, -r13.yyyx, l(0.000000, 0.000000, 1.000000, 1.000000)
    r13.zw = ((-(r13.yyyx))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 135: add r0.w, r0.y, r13.x
    r0.w = ((r0.yyyy)+(r13.xxxx)).w;
    // 136: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 137: mov_sat r2.x, cb0[8].z
    r2.x = (saturate(source[8].zzzz)).x;
    // 138: mad r15.xyz, -r2.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r1.xyzx
    r15.xyz = ((-(r2.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r1.xyzx)).xyz;
    // 139: mul r2.x, r2.x, l(0.080000)
    r2.x = ((r2.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).x;
    // 140: mad r15.xyz, r0.zzzz, r15.xyzx, r2.xxxx
    r15.xyz = ((r0.zzzz)*(r15.xyzx)+(r2.xxxx)).xyz;
    // 141: max r16.xyz, r13.zzzz, r15.xyzx
    r16.xyz = (max(r13.zzzz,r15.xyzx)).xyz;
    // 142: add r16.xyz, -r15.xyzx, r16.xyzx
    r16.xyz = ((-(r15.xyzx))+(r16.xyzx)).xyz;
    // 143: mul_sat r2.x, r15.y, l(50.000000)
    r2.x = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 144: mul r16.xyz, r2.xxxx, r16.xyzx
    r16.xyz = ((r2.xxxx)*(r16.xyzx)).xyz;
    // 145: mul r17.xyz, r5.wwww, r15.xyzx
    r17.xyz = ((r5.wwww)*(r15.xyzx)).xyz;
    // 146: mad r16.xyz, r16.xyzx, r5.zzzz, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r5.zzzz)+(r17.xyzx)).xyz;
    // 147: div r5.z, l(1.000000, 1.000000, 1.000000, 1.000000), r5.w
    r5.z = r5.w != 0.f ? 1.f / r5.w : 0.f;
    // 148: add r5.z, r5.z, l(-1.000000)
    r5.z = ((r5.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 149: mad r17.xyz, r15.xyzx, r5.zzzz, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r15.xyzx)*(r5.zzzz)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 150: mad r18.xyz, -r16.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r16.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 151: mul r16.xyz, r16.xyzx, r17.xyzx
    r16.xyz = ((r16.xyzx)*(r17.xyzx)).xyz;
    // 152: add r5.z, -r2.y, l(1.000000)
    r5.z = ((-(r2.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 153: mad_sat r2.y, r4.w, r5.z, r2.y
    r2.y = (saturate((r4.wwww)*(r5.zzzz)+(r2.yyyy))).y;
    // 154: mad r2.y, -r2.y, cb0[2].x, l(1.000000)
    r2.y = ((-(r2.yyyy))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 155: mul r4.w, r13.y, r13.y
    r4.w = ((r13.yyyy)*(r13.yyyy)).w;
    // 156: mul r5.z, r13.y, l(5.000000)
    r5.z = ((r13.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 157: mad r5.w, r4.w, l(0.350000), l(1.000000)
    r5.w = ((r4.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 158: mul r0.w, r0.w, r4.w
    r0.w = ((r0.wwww)*(r4.wwww)).w;
    // 159: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 160: add r0.y, r0.y, r0.w
    r0.y = ((r0.yyyy)+(r0.wwww)).y;
    // 161: add_sat r0.y, r0.y, l(-1.000000)
    r0.y = (saturate((r0.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).y;
    // 162: div_sat r0.w, r2.y, r5.w
    r0.w = (saturate((r2.yyyy)/(r5.wwww))).w;
    // 163: mul r2.y, r13.w, r13.w
    r2.y = ((r13.wwww)*(r13.wwww)).y;
    // 164: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 165: mul r4.w, r13.w, r2.y
    r4.w = ((r13.wwww)*(r2.yyyy)).w;
    // 166: mad r2.y, -r2.y, r13.w, l(1.000000)
    r2.y = ((-(r2.yyyy))*(r13.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 167: mul r13.xyz, r15.xyzx, r2.yyyy
    r13.xyz = ((r15.xyzx)*(r2.yyyy)).xyz;
    // 168: dp3 r2.y, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.y = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 169: mad r15.xyz, r2.yyyy, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r2.yyyy)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 170: mad r13.xyz, r2.xxxx, r4.wwww, r13.xyzx
    r13.xyz = ((r2.xxxx)*(r4.wwww)+(r13.xyzx)).xyz;
    // 171: add r13.xyz, -r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((-(r13.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 172: mul r13.xyz, r13.xyzx, r13.xyzx
    r13.xyz = ((r13.xyzx)*(r13.xyzx)).xyz;
    // 173: mad r17.xyz, -r0.wwww, r13.xyzx, r18.xyzx
    r17.xyz = ((-(r0.wwww))*(r13.xyzx)+(r18.xyzx)).xyz;
    // 174: mul r18.xyz, r1.xyzx, r18.xyzx
    r18.xyz = ((r1.xyzx)*(r18.xyzx)).xyz;
    // 175: mul r13.xyz, r0.wwww, r13.xyzx
    r13.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 176: mul r13.xyz, r1.xyzx, r13.xyzx
    r13.xyz = ((r1.xyzx)*(r13.xyzx)).xyz;
    // 177: mad r13.xyz, -r13.xyzx, r0.zzzz, r13.xyzx
    r13.xyz = ((-(r13.xyzx))*(r0.zzzz)+(r13.xyzx)).xyz;
    // 178: mul r7.xyz, r7.xyzx, r17.xyzx
    r7.xyz = ((r7.xyzx)*(r17.xyzx)).xyz;
    // 179: mad r7.xyz, -r7.xyzx, r0.zzzz, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r0.zzzz)+(r7.xyzx)).xyz;
    // 180: dp3 r2.x, r9.xyzx, r14.xyzx
    r2.x = (dot((r9.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 181: dp3 r2.y, r10.xyzx, r14.xyzx
    r2.y = (dot((r10.xyzx).xyz,(r14.xyzx).xyz).xxxx).y;
    // 182: dp2 r9.x, r2.xyxx, r2.zwzz
    r9.x = (dot((r2.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 183: dp2 r9.z, r2.xyxx, cb0[13].xyxx
    r9.z = (dot((r2.xyxx).xy,(source[13].xyxx).xy).xxxx).z;
    // 184: dp3 r9.y, r8.xyzx, r14.xyzx
    r9.y = (dot((r8.xyzx).xyz,(r14.xyzx).xyz).xxxx).y;
    // 185: dp3 r2.x, r6.xyzx, r14.xyzx
    r2.x = (dot((r6.xyzx).xyz,(r14.xyzx).xyz).xxxx).x;
    // 186: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 187: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 188: sample_l_indexable(texturecube)(float,float,float,float) r6.xyzw, r9.xyzx, t6.xyzw, s5, r5.z
    r6.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r5.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 189: mul r6.xyz, r6.xyzx, r6.wwww
    r6.xyz = ((r6.xyzx)*(r6.wwww)).xyz;
    // 190: mul r6.xyz, r6.xyzx, cb0[12].xyzx
    r6.xyz = ((r6.xyzx)*(source[12].xyzx)).xyz;
    // 191: mad r6.xyz, r6.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[12].wwww
    r6.xyz = ((r6.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[12].wwww)).xyz;
    // 192: dp3 r2.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: div r0.x, r2.z, r0.x
    r0.x = ((r2.zzzz)/(r0.xxxx)).x;
    // 194: mad r0.x, r3.w, l(5.000000), r0.x
    r0.x = ((r3.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r0.xxxx)).x;
    // 195: add_sat r0.x, r0.z, r0.x
    r0.x = (saturate((r0.zzzz)+(r0.xxxx))).x;
    // 196: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 197: mad r2.z, r0.x, l(-2.000000), l(3.000000)
    r2.z = ((r0.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).z;
    // 198: mul r0.x, r0.x, r0.x
    r0.x = ((r0.xxxx)*(r0.xxxx)).x;
    // 199: mul r0.x, r0.x, r2.z
    r0.x = ((r0.xxxx)*(r2.zzzz)).x;
    // 200: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 201: mul r0.x, r0.x, l(1.500000)
    r0.x = ((r0.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 202: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 203: mul r6.xyz, r0.xxxx, r6.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 204: mad r0.x, r0.y, r15.x, r15.y
    r0.x = ((r0.yyyy)*(r15.xxxx)+(r15.yyyy)).x;
    // 205: mad r0.x, r0.x, r0.y, r15.z
    r0.x = ((r0.xxxx)*(r0.yyyy)+(r15.zzzz)).x;
    // 206: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 207: max r0.x, r0.x, r0.y
    r0.x = (max(r0.xxxx,r0.yyyy)).x;
    // 208: mul r2.yzw, r2.yyyy, cb0[22].xxyz
    r2.yzw = ((r2.yyyy)*(source[22].xxyz)).yzw;
    // 209: mad r2.xyz, cb0[21].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[21].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 210: mul r2.xyz, r2.xyzx, cb0[23].wwww
    r2.xyz = ((r2.xyzx)*(source[23].wwww)).xyz;
    // 211: mul r2.xyz, r0.xxxx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r2.xyzx)).xyz;
    // 212: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 213: mul r6.xyz, r6.xyzx, r16.xyzx
    r6.xyz = ((r6.xyzx)*(r16.xyzx)).xyz;
    // 214: mad r2.xyz, r2.xyzx, r16.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r16.xyzx)+(r7.xyzx)).xyz;
    // 215: dp3 r0.y, r4.xyzx, r12.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 216: add r2.w, -|r12.z|, l(1.000000)
    r2.w = ((-(abs(r12.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 217: add r0.y, -|r0.y|, l(1.000000)
    r0.y = ((-(abs(r0.yyyy)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 218: mul r0.y, r0.y, r2.w
    r0.y = ((r0.yyyy)*(r2.wwww)).y;
    // 219: lt r2.w, |r0.y|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 220: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 221: mul r0.y, r0.y, l(1.500000)
    r0.y = ((r0.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 222: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 223: mul r4.xyz, r0.yyyy, cb0[4].xyzx
    r4.xyz = ((r0.yyyy)*(source[4].xyzx)).xyz;
    // 224: movc r4.xyz, r2.wwww, l(0,0,0,0), r4.xyzx
    r4.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xyzx)).xyz;
    // 225: add r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)+(source[3].xyzx)).xyz;
    // 226: mul r7.xyz, r0.zzzz, r18.xyzx
    r7.xyz = ((r0.zzzz)*(r18.xyzx)).xyz;
    // 227: mul r7.xyz, r11.xyzx, r7.xyzx
    r7.xyz = ((r11.xyzx)*(r7.xyzx)).xyz;
    // 228: mul r3.xyz, r3.xyzx, r7.xyzx
    r3.xyz = ((r3.xyzx)*(r7.xyzx)).xyz;
    // 229: mad r3.xyz, r6.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r6.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 230: mul r3.xyz, r3.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 231: add r0.x, -r0.w, l(1.000000)
    r0.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 232: mad r4.xyz, r3.xyzx, r0.xxxx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 233: mul r3.xyz, r0.wwww, r3.xyzx
    r3.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 234: mad r3.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r13.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r13.xyzx)).xyz;
    // 235: mul r5.yzw, r5.yyyy, cb0[22].xxyz
    r5.yzw = ((r5.yyyy)*(source[22].xxyz)).yzw;
    // 236: mad r5.xyz, r5.xxxx, cb0[21].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[21].xyzx)+(r5.yzwy)).xyz;
    // 237: mul r5.xyz, r5.xyzx, cb0[23].wwww
    r5.xyz = ((r5.xyzx)*(source[23].wwww)).xyz;
    // 238: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 239: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 240: add_sat r6.xyz, r6.xyzx, cb0[11].yyyy
    r6.xyz = (saturate((r6.xyzx)+(source[11].yyyy))).xyz;
    // 241: mul r0.y, r6.x, cb0[11].z
    r0.y = ((r6.xxxx)*(source[11].zzzz)).y;
    // 242: mul_sat r6.xyz, r6.xyzx, cb0[6].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[6].xyzx))).xyz;
    // 243: mul r0.y, r0.y, r1.w
    r0.y = ((r0.yyyy)*(r1.wwww)).y;
    // 244: mul r6.xyz, r6.xyzx, r0.yyyy
    r6.xyz = ((r6.xyzx)*(r0.yyyy)).xyz;
    // 245: mul r6.xyz, r0.zzzz, r6.xyzx
    r6.xyz = ((r0.zzzz)*(r6.xyzx)).xyz;
    // 246: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 247: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 248: mad r0.xyz, r1.xyzx, r0.xxxx, r4.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xxxx)+(r4.xyzx)).xyz;
    // 249: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 250: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r3.xyzx)).xyz;
    // 251: mul o1.xyz, r1.xyzx, v5.wwww
    output.targets[1].xyz = ((r1.xyzx)*(v5.wwww)).xyz;
    // 252: add r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)+(r2.xyzx)).xyz;
    // 253: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 254: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 255: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 256: ret
    return output;
}

// source.character.static-map-native-1102.v1 / source program cc61c184735e3a4b945cd4fc591c10f3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1102(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    source[26]=1.f;
    source[27]=1.f;
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
    // 7: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 9: mad r1.xyz, cb0[8].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[8].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 10: mul r2.xyz, cb0[5].xyzx, cb0[8].wwww
    r2.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 11: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 12: mul r2.xyz, cb0[6].xyzx, cb0[9].xxxx
    r2.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 13: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 15: mul r1.w, r2.z, cb0[9].y
    r1.w = ((r2.zzzz)*(source[9].yyyy)).w;
    // 16: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 17: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r2.z, r2.z, cb0[9].z
    r2.z = ((r2.zzzz)*(source[9].zzzz)).z;
    // 19: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 20: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 21: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 22: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xyzx, cb0[9].wwww
    r1.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 24: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 25: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 26: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 28: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 29: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 30: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 31: mul r2.x, r2.x, cb0[12].y
    r2.x = ((r2.xxxx)*(source[12].yyyy)).x;
    // 32: mul r2.y, r2.y, cb0[11].w
    r2.y = ((r2.yyyy)*(source[11].wwww)).y;
    // 33: log r2.z, |r2.x|
    r2.z = (log2(abs(r2.xxxx))).z;
    // 34: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 35: mul r2.z, r2.z, cb0[12].z
    r2.z = ((r2.zzzz)*(source[12].zzzz)).z;
    // 36: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 37: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 39: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 40: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 41: mad r1.xyz, r1.xyzx, r2.xxxx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)+(r3.xyzx)).xyz;
    // 42: mul r1.xyz, r2.xxxx, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)).xyz;
    // 43: max r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = (max(r1.xyzx,r2.xxxx)).xyz;
    // 44: dp3 r2.z, v7.xyzx, v7.xyzx
    r2.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 45: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 46: mul r3.xyz, r2.zzzz, v7.xyzx
    r3.xyz = ((r2.zzzz)*(v7.xyzx)).xyz;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 48: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 49: dp2 r3.w, r2.zwzz, r2.zwzz
    r3.w = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).w;
    // 50: mul r2.zw, r2.zzzw, cb0[8].xxxx
    r2.zw = ((r2.zzzw)*(source[8].xxxx)).zw;
    // 51: mul r4.xy, r2.zwzz, v2.wwww
    r4.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 52: add r2.z, -r3.w, l(1.000000)
    r2.z = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 53: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 54: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 55: add r4.z, r2.z, l(0.000010)
    r4.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 56: dp3 r2.z, r4.xyzx, r4.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 57: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 58: div r4.xyz, r4.xyzx, r2.zzzz
    r4.xyz = ((r4.xyzx)/(r2.zzzz)).xyz;
    // 59: dp3 r2.z, r4.xyzx, r4.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 60: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 61: mul r5.xyz, r2.zzzz, r4.xyzx
    r5.xyz = ((r2.zzzz)*(r4.xyzx)).xyz;
    // 62: dp3 r2.z, r3.xyzx, r5.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 63: dp3 r2.w, -r3.xyzx, r5.xyzx
    r2.w = (dot((-(r3.xyzx)).xyz,(r5.xyzx).xyz).xxxx).w;
    // 64: mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 65: mad r2.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 66: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 67: mul r6.xyz, r2.wwww, cb0[24].xyzx
    r6.xyz = ((r2.wwww)*(source[24].xyzx)).xyz;
    // 68: mad r6.xyz, r2.zzzz, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r2.zzzz)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 69: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 70: mul r7.xyz, r0.xyzx, r6.xyzx
    r7.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 71: dp2_sat r8.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 72: dp3_sat r8.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 73: dp3_sat r8.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 74: mul r8.xyz, r8.xyzx, r8.xyzx
    r8.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t8.xyzw, s5
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 76: mul r9.xyz, r9.xyzx, cb0[27].xyzx
    r9.xyz = ((r9.xyzx)*(source[27].xyzx)).xyz;
    // 77: dp3 r2.z, r9.xyzx, r8.xyzx
    r2.z = (dot((r9.xyzx).xyz,(r8.xyzx).xyz).xxxx).z;
    // 78: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 79: mul r8.xyz, r8.xyzx, cb0[26].xyzx
    r8.xyz = ((r8.xyzx)*(source[26].xyzx)).xyz;
    // 80: mul r10.xyz, r2.zzzz, r8.xyzx
    r10.xyz = ((r2.zzzz)*(r8.xyzx)).xyz;
    // 81: mad r7.xyz, r0.xyzx, r10.xyzx, r7.xyzx
    r7.xyz = ((r0.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 82: mul r7.xyz, r1.xyzx, r7.xyzx
    r7.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 83: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 84: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 85: mul r10.xyz, r2.wwww, v1.xyzx
    r10.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 86: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 87: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 88: mul r11.xyz, r2.wwww, v0.xyzx
    r11.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 89: mul r12.xyz, r10.zxyz, r11.yzxy
    r12.xyz = ((r10.zxyz)*(r11.yzxy)).xyz;
    // 90: mad r12.xyz, r10.yzxy, r11.zxyz, -r12.xyzx
    r12.xyz = ((r10.yzxy)*(r11.zxyz)+(-(r12.xyzx))).xyz;
    // 91: mul r12.xyz, r12.xyzx, v1.wwww
    r12.xyz = ((r12.xyzx)*(v1.wwww)).xyz;
    // 92: dp3 r13.y, r12.xyzx, r5.xyzx
    r13.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 93: dp3 r13.x, r11.xyzx, r5.xyzx
    r13.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 94: dp2 r14.z, r13.xyxx, cb0[15].xyxx
    r14.z = (dot((r13.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 95: mul r3.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r3.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 96: dp2 r14.x, r13.xyxx, r3.zwzz
    r14.x = (dot((r13.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 97: dp3 r14.y, r10.xyzx, r5.xyzx
    r14.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 98: mov r14.w, l(1.000000)
    r14.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 99: dp4 r13.x, cb0[16].xyzw, r14.xyzw
    r13.x = (dot((source[16].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).x;
    // 100: dp4 r13.y, cb0[17].xyzw, r14.xyzw
    r13.y = (dot((source[17].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).y;
    // 101: dp4 r13.z, cb0[18].xyzw, r14.xyzw
    r13.z = (dot((source[18].xyzw).xyzw,(r14.xyzw).xyzw).xxxx).z;
    // 102: mul r15.xyzw, r14.yzzx, r14.xyzz
    r15.xyzw = ((r14.yzzx)*(r14.xyzz)).xyzw;
    // 103: mul r2.w, r14.y, r14.y
    r2.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 104: mad r2.w, r14.x, r14.x, -r2.w
    r2.w = ((r14.xxxx)*(r14.xxxx)+(-(r2.wwww))).w;
    // 105: dp4 r14.x, cb0[19].xyzw, r15.xyzw
    r14.x = (dot((source[19].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).x;
    // 106: dp4 r14.y, cb0[20].xyzw, r15.xyzw
    r14.y = (dot((source[20].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).y;
    // 107: dp4 r14.z, cb0[21].xyzw, r15.xyzw
    r14.z = (dot((source[21].xyzw).xyzw,(r15.xyzw).xyzw).xxxx).z;
    // 108: add r13.xyz, r13.xyzx, r14.xyzx
    r13.xyz = ((r13.xyzx)+(r14.xyzx)).xyz;
    // 109: mad r13.xyz, cb0[22].xyzx, r2.wwww, r13.xyzx
    r13.xyz = ((source[22].xyzx)*(r2.wwww)+(r13.xyzx)).xyz;
    // 110: max r13.xyz, r13.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r13.xyz = (max(r13.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 111: mul r13.xyz, r13.xyzx, cb0[14].xyzx
    r13.xyz = ((r13.xyzx)*(source[14].xyzx)).xyz;
    // 112: mad r13.xyz, r13.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r13.xyz = ((r13.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 113: dp3 r2.w, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: log r4.w, |r2.y|
    r4.w = (log2(abs(r2.yyyy))).w;
    // 115: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 116: mul r4.w, r4.w, cb0[12].x
    r4.w = ((r4.wwww)*(source[12].xxxx)).w;
    // 117: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 118: movc r2.y, r2.y, l(0), r4.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).y;
    // 119: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 120: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 121: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 122: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 123: mul r14.xyz, r4.wwww, v6.xyzx
    r14.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 124: dp3 r4.w, r5.xyzx, r14.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 125: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 126: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r14.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r14.xyzx))).xyz;
    // 127: deriv_rtx_coarse r15.x, r4.w
    r15.x = (ddx_coarse(r4.wwww)).x;
    // 128: deriv_rty_coarse r15.y, r4.w
    r15.y = (ddy_coarse(r4.wwww)).y;
    // 129: dp2 r5.w, r15.xyxx, r15.xyxx
    r5.w = (dot((r15.xyxx).xy,(r15.xyxx).xy).xxxx).w;
    // 130: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 131: mad_sat r15.y, r5.w, l(0.300000), r2.y
    r15.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.yyyy))).y;
    // 132: mad r5.w, r15.y, l(0.200000), l(0.200000)
    r5.w = ((r15.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).w;
    // 133: div r2.w, r2.w, r5.w
    r2.w = ((r2.wwww)/(r5.wwww)).w;
    // 134: dp3 r6.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 135: mad r2.w, r6.w, l(5.000000), r2.w
    r2.w = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.wwww)).w;
    // 136: add r7.w, r0.w, cb0[11].y
    r7.w = ((r0.wwww)+(source[11].yyyy)).w;
    // 137: add r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)+(source[10].wwww)).w;
    // 138: sample_b_indexable(texture2d)(float,float,float,float) r8.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r8.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 139: mad r7.w, r8.w, -r7.w, r7.w
    r7.w = ((r8.wwww)*(-(r7.wwww))+(r7.wwww)).w;
    // 140: add_sat r1.w, r1.w, r7.w
    r1.w = (saturate((r1.wwww)+(r7.wwww))).w;
    // 141: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 142: add_sat r2.w, r1.w, r2.w
    r2.w = (saturate((r1.wwww)+(r2.wwww))).w;
    // 143: mad r7.w, r2.w, l(-2.000000), l(3.000000)
    r7.w = ((r2.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 144: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 145: mul r2.w, r2.w, r7.w
    r2.w = ((r2.wwww)*(r7.wwww)).w;
    // 146: log r2.w, r2.w
    r2.w = (log2(r2.wwww)).w;
    // 147: mul r2.w, r2.w, l(1.500000)
    r2.w = ((r2.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 148: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 149: mul r13.xyz, r2.wwww, r13.xyzx
    r13.xyz = ((r2.wwww)*(r13.xyzx)).xyz;
    // 150: mul r7.xyz, r7.xyzx, r13.xyzx
    r7.xyz = ((r7.xyzx)*(r13.xyzx)).xyz;
    // 151: add r2.w, -r0.w, l(1.000000)
    r2.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: mad_sat r0.w, r8.w, r2.w, r0.w
    r0.w = (saturate((r8.wwww)*(r2.wwww)+(r0.wwww))).w;
    // 153: mad r0.w, -r0.w, cb0[2].x, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 154: mul r2.w, r15.y, r15.y
    r2.w = ((r15.yyyy)*(r15.yyyy)).w;
    // 155: mad r7.w, r2.w, l(0.350000), l(1.000000)
    r7.w = ((r2.wwww)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 156: div_sat r0.w, r0.w, r7.w
    r0.w = (saturate((r0.wwww)/(r7.wwww))).w;
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
    // 167: mov_sat r7.w, cb0[10].y
    r7.w = (saturate(source[10].yyyy)).w;
    // 168: mad r16.xyz, -r7.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r16.xyz = ((-(r7.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 169: mul r7.w, r7.w, l(0.080000)
    r7.w = ((r7.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 170: mad r16.xyz, r1.wwww, r16.xyzx, r7.wwww
    r16.xyz = ((r1.wwww)*(r16.xyzx)+(r7.wwww)).xyz;
    // 171: max r17.xyz, r15.zzzz, r16.xyzx
    r17.xyz = (max(r15.zzzz,r16.xyzx)).xyz;
    // 172: add r17.xyz, -r16.xyzx, r17.xyzx
    r17.xyz = ((-(r16.xyzx))+(r17.xyzx)).xyz;
    // 173: mul_sat r7.w, r16.y, l(50.000000)
    r7.w = (saturate((r16.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 174: mul r17.xyz, r7.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r17.xyzx)).xyz;
    // 175: sample_indexable(texture2d)(float,float,float,float) r18.xy, r15.xyxx, t5.xyzw, s7
    r18.xy = ((float4(0.0,0.0,0.0,0.0)).xyzw).xy;
    // 176: add r8.w, r2.x, r15.x
    r8.w = ((r2.xxxx)+(r15.xxxx)).w;
    // 177: log r8.w, r8.w
    r8.w = (log2(r8.wwww)).w;
    // 178: mul r2.w, r2.w, r8.w
    r2.w = ((r2.wwww)*(r8.wwww)).w;
    // 179: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 180: add r2.x, r2.x, r2.w
    r2.x = ((r2.xxxx)+(r2.wwww)).x;
    // 181: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 182: mul r2.w, r15.y, l(5.000000)
    r2.w = ((r15.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 183: mul r15.xyz, r16.xyzx, r18.yyyy
    r15.xyz = ((r16.xyzx)*(r18.yyyy)).xyz;
    // 184: mad r15.xyz, r17.xyzx, r18.xxxx, r15.xyzx
    r15.xyz = ((r17.xyzx)*(r18.xxxx)+(r15.xyzx)).xyz;
    // 185: div r8.w, l(1.000000, 1.000000, 1.000000, 1.000000), r18.y
    r8.w = r18.y != 0.f ? 1.f / r18.y : 0.f;
    // 186: add r8.w, r8.w, l(-1.000000)
    r8.w = ((r8.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 187: mad r17.xyz, r16.xyzx, r8.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((r16.xyzx)*(r8.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 188: mad r18.xyz, -r15.xyzx, r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r18.xyz = ((-(r15.xyzx))*(r17.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 189: mul r15.xyz, r15.xyzx, r17.xyzx
    r15.xyz = ((r15.xyzx)*(r17.xyzx)).xyz;
    // 190: mul r8.w, r15.w, r15.w
    r8.w = ((r15.wwww)*(r15.wwww)).w;
    // 191: mul r8.w, r8.w, r8.w
    r8.w = ((r8.wwww)*(r8.wwww)).w;
    // 192: mul r9.w, r15.w, r8.w
    r9.w = ((r15.wwww)*(r8.wwww)).w;
    // 193: mad r8.w, -r8.w, r15.w, l(1.000000)
    r8.w = ((-(r8.wwww))*(r15.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 194: mul r17.xyz, r16.xyzx, r8.wwww
    r17.xyz = ((r16.xyzx)*(r8.wwww)).xyz;
    // 195: dp3 r8.w, r16.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r8.w = (dot((r16.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 196: mad r16.xyz, r8.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r16.xyz = ((r8.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 197: mad r17.xyz, r7.wwww, r9.wwww, r17.xyzx
    r17.xyz = ((r7.wwww)*(r9.wwww)+(r17.xyzx)).xyz;
    // 198: add r17.xyz, -r17.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r17.xyz = ((-(r17.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 199: mul r17.xyz, r17.xyzx, r17.xyzx
    r17.xyz = ((r17.xyzx)*(r17.xyzx)).xyz;
    // 200: mad r19.xyz, -r0.wwww, r17.xyzx, r18.xyzx
    r19.xyz = ((-(r0.wwww))*(r17.xyzx)+(r18.xyzx)).xyz;
    // 201: mul r18.xyz, r0.xyzx, r18.xyzx
    r18.xyz = ((r0.xyzx)*(r18.xyzx)).xyz;
    // 202: mul r17.xyz, r0.wwww, r17.xyzx
    r17.xyz = ((r0.wwww)*(r17.xyzx)).xyz;
    // 203: mul r17.xyz, r0.xyzx, r17.xyzx
    r17.xyz = ((r0.xyzx)*(r17.xyzx)).xyz;
    // 204: mad r17.xyz, -r17.xyzx, r1.wwww, r17.xyzx
    r17.xyz = ((-(r17.xyzx))*(r1.wwww)+(r17.xyzx)).xyz;
    // 205: mul r7.xyz, r7.xyzx, r19.xyzx
    r7.xyz = ((r7.xyzx)*(r19.xyzx)).xyz;
    // 206: mad r7.xyz, -r7.xyzx, r1.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r1.wwww)+(r7.xyzx)).xyz;
    // 207: dp2_sat r19.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r19.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 208: dp3_sat r19.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r19.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 209: dp3_sat r19.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r19.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 210: mul r19.xyz, r19.xyzx, r19.xyzx
    r19.xyz = ((r19.xyzx)*(r19.xyzx)).xyz;
    // 211: dp3 r7.w, r9.xyzx, r19.xyzx
    r7.w = (dot((r9.xyzx).xyz,(r19.xyzx).xyz).xxxx).w;
    // 212: add r2.z, r2.z, -r7.w
    r2.z = ((r2.zzzz)+(-(r7.wwww))).z;
    // 213: mad r2.y, r2.y, r2.z, r7.w
    r2.y = ((r2.yyyy)*(r2.zzzz)+(r7.wwww)).y;
    // 214: mad r6.xyz, r8.xyzx, r2.yyyy, r6.xyzx
    r6.xyz = ((r8.xyzx)*(r2.yyyy)+(r6.xyzx)).xyz;
    // 215: mad r2.y, r2.x, r16.x, r16.y
    r2.y = ((r2.xxxx)*(r16.xxxx)+(r16.yyyy)).y;
    // 216: mad r2.y, r2.y, r2.x, r16.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r16.zzzz)).y;
    // 217: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 218: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 219: mul r6.xyz, r2.xxxx, r6.xyzx
    r6.xyz = ((r2.xxxx)*(r6.xyzx)).xyz;
    // 220: dp3 r11.x, r11.xyzx, r5.xyzx
    r11.x = (dot((r11.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 221: dp3 r11.y, r12.xyzx, r5.xyzx
    r11.y = (dot((r12.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 222: dp3 r5.y, r10.xyzx, r5.xyzx
    r5.y = (dot((r10.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 223: dp2 r5.x, r11.xyxx, r3.zwzz
    r5.x = (dot((r11.xyxx).xy,(r3.zwzz).xy).xxxx).x;
    // 224: dp2 r5.z, r11.xyxx, cb0[15].xyxx
    r5.z = (dot((r11.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 225: sample_l_indexable(texturecube)(float,float,float,float) r10.xyzw, r5.xyzx, t6.xyzw, s6, r2.w
    r10.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r5.xyzx).xyz, (r2.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 226: mul r2.yzw, r10.xxyz, r10.wwww
    r2.yzw = ((r10.xxyz)*(r10.wwww)).yzw;
    // 227: mul r2.yzw, r2.yyzw, cb0[14].xxyz
    r2.yzw = ((r2.yyzw)*(source[14].xxyz)).yzw;
    // 228: mad r2.yzw, r2.yyzw, l(0.000000, 6.000000, 6.000000, 6.000000), cb0[14].wwww
    r2.yzw = ((r2.yyzw)*(float4(0.000000,6.000000,6.000000,6.000000))+(source[14].wwww)).yzw;
    // 229: dp3 r3.z, r2.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.z = (dot((r2.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 230: div r3.z, r3.z, r5.w
    r3.z = ((r3.zzzz)/(r5.wwww)).z;
    // 231: mad r3.z, r6.w, l(5.000000), r3.z
    r3.z = ((r6.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.zzzz)).z;
    // 232: add_sat r3.z, r1.w, r3.z
    r3.z = (saturate((r1.wwww)+(r3.zzzz))).z;
    // 233: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 234: mad r3.w, r3.z, l(-2.000000), l(3.000000)
    r3.w = ((r3.zzzz)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 235: mul r3.z, r3.z, r3.z
    r3.z = ((r3.zzzz)*(r3.zzzz)).z;
    // 236: mul r3.xyz, r3.xyzx, r3.xywx
    r3.xyz = ((r3.xyzx)*(r3.xywx)).xyz;
    // 237: log r3.z, r3.z
    r3.z = (log2(r3.zzzz)).z;
    // 238: mul r3.z, r3.z, l(1.500000)
    r3.z = ((r3.zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).z;
    // 239: exp r3.z, r3.z
    r3.z = (exp2(r3.zzzz)).z;
    // 240: mul r2.yzw, r2.yyzw, r3.zzzz
    r2.yzw = ((r2.yyzw)*(r3.zzzz)).yzw;
    // 241: mul r5.xyz, r6.xyzx, r2.yzwy
    r5.xyz = ((r6.xyzx)*(r2.yzwy)).xyz;
    // 242: mul r2.yzw, r2.yyzw, r15.xxyz
    r2.yzw = ((r2.yyzw)*(r15.xxyz)).yzw;
    // 243: mad r5.xyz, r5.xyzx, r15.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r15.xyzx)+(r7.xyzx)).xyz;
    // 244: mul r3.yzw, r3.yyyy, cb0[24].xxyz
    r3.yzw = ((r3.yyyy)*(source[24].xxyz)).yzw;
    // 245: mad r3.xyz, r3.xxxx, cb0[23].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[23].xyzx)+(r3.yzwy)).xyz;
    // 246: mul r3.xyz, r3.xyzx, cb0[25].wwww
    r3.xyz = ((r3.xyzx)*(source[25].wwww)).xyz;
    // 247: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 248: add_sat r6.xyz, -r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (saturate((-(r6.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 249: add_sat r6.xyz, r6.xyzx, cb0[13].xxxx
    r6.xyz = (saturate((r6.xyzx)+(source[13].xxxx))).xyz;
    // 250: mul r3.w, r6.x, cb0[13].y
    r3.w = ((r6.xxxx)*(source[13].yyyy)).w;
    // 251: mul_sat r6.xyz, r6.xyzx, cb0[7].xyzx
    r6.xyz = (saturate((r6.xyzx)*(source[7].xyzx))).xyz;
    // 252: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 253: mul r6.xyz, r6.xyzx, r3.wwww
    r6.xyz = ((r6.xyzx)*(r3.wwww)).xyz;
    // 254: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 255: mul r7.xyz, r1.wwww, r18.xyzx
    r7.xyz = ((r1.wwww)*(r18.xyzx)).xyz;
    // 256: mul r7.xyz, r13.xyzx, r7.xyzx
    r7.xyz = ((r13.xyzx)*(r7.xyzx)).xyz;
    // 257: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 258: mad r1.xyz, r2.yzwy, r2.xxxx, r1.xyzx
    r1.xyz = ((r2.yzwy)*(r2.xxxx)+(r1.xyzx)).xyz;
    // 259: mul r2.xyz, r3.xyzx, r6.xyzx
    r2.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 260: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 261: dp3 r1.w, r9.xyzx, r1.wwww
    r1.w = (dot((r9.xyzx).xyz,(r1.wwww).xyz).xxxx).w;
    // 262: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 263: mul r2.xyz, r0.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 264: mad r0.xyz, r0.xyzx, r3.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 265: dp3 r1.w, r4.xyzx, r14.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r14.xyzx).xyz).xxxx).w;
    // 266: add r2.x, -|r14.z|, l(1.000000)
    r2.x = ((-(abs(r14.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 267: add r1.w, -|r1.w|, l(1.000000)
    r1.w = ((-(abs(r1.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 268: mul r1.w, r1.w, r2.x
    r1.w = ((r1.wwww)*(r2.xxxx)).w;
    // 269: lt r2.x, |r1.w|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 270: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 271: mul r1.xyzw, r1.xyzw, l(0.300000, 0.300000, 0.300000, 1.500000)
    r1.xyzw = ((r1.xyzw)*(float4(0.300000,0.300000,0.300000,1.500000))).xyzw;
    // 272: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 273: mul r2.yzw, r1.wwww, cb0[4].xxyz
    r2.yzw = ((r1.wwww)*(source[4].xxyz)).yzw;
    // 274: movc r2.xyz, r2.xxxx, l(0,0,0,0), r2.yzwy
    r2.xyz = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.yzwy)).xyz;
    // 275: add r2.xyz, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(source[3].xyzx)).xyz;
    // 276: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 277: mad r2.xyz, r1.xyzx, r1.wwww, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 278: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 279: mul r3.xyz, r0.wwww, r0.xyzx
    r3.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 280: mad r0.xyz, r0.xyzx, r1.wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r1.wwww)+(r2.xyzx)).xyz;
    // 281: add r0.xyz, r0.xyzx, r5.xyzx
    r0.xyz = ((r0.xyzx)+(r5.xyzx)).xyz;
    // 282: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 283: mad r0.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r17.xyzx
    r0.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r17.xyzx)).xyz;
    // 284: mad r0.xyz, r3.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r0.xyzx
    r0.xyz = ((r3.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r0.xyzx)).xyz;
    // 285: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 286: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 287: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 288: ret
    return output;
}

// source.character.static-map-native-1102.v1 / source program 8156ec06b1677b4c93e8c5bfeaa588b4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1102(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1102(input);
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
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[14]=g_SourceCharacterEnvironmentColor;source[15]=g_SourceCharacterEnvironmentRotation;}
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f;
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
    // 7: dp3 r1.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 8: add r1.xyz, -r0.xyzx, r1.xxxx
    r1.xyz = ((-(r0.xyzx))+(r1.xxxx)).xyz;
    // 9: mad r1.xyz, cb0[8].zzzz, r1.xyzx, r0.xyzx
    r1.xyz = ((source[8].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 10: mul r2.xyz, cb0[5].xyzx, cb0[8].wwww
    r2.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 11: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 12: mul r2.xyz, cb0[6].xyzx, cb0[9].xxxx
    r2.xyz = ((source[6].xyzx)*(source[9].xxxx)).xyz;
    // 13: mad r0.xyz, r2.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 15: mul r1.w, r2.z, cb0[9].y
    r1.w = ((r2.zzzz)*(source[9].yyyy)).w;
    // 16: log r2.z, |r1.w|
    r2.z = (log2(abs(r1.wwww))).z;
    // 17: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 18: mul r2.z, r2.z, cb0[9].z
    r2.z = ((r2.zzzz)*(source[9].zzzz)).z;
    // 19: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 20: movc r1.w, r1.w, l(0), r2.z
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).w;
    // 21: min r2.z, r1.w, l(1.000000)
    r2.z = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 22: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 23: mul r1.xyz, r0.xyzx, cb0[9].wwww
    r1.xyz = ((r0.xyzx)*(source[9].wwww)).xyz;
    // 24: mad r0.xyz, cb0[10].xxxx, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[10].xxxx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 25: mad r0.xyz, r2.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 26: add r1.xyz, -cb0[3].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(source[3].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 27: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 28: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 29: mad r1.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r1.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 30: mad r3.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 31: mul r2.x, r2.x, cb0[12].y
    r2.x = ((r2.xxxx)*(source[12].yyyy)).x;
    // 32: mul r2.y, r2.y, cb0[11].w
    r2.y = ((r2.yyyy)*(source[11].wwww)).y;
    // 33: log r2.z, |r2.x|
    r2.z = (log2(abs(r2.xxxx))).z;
    // 34: lt r2.x, |r2.x|, l(0.000001)
    r2.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 35: mul r2.z, r2.z, cb0[12].z
    r2.z = ((r2.zzzz)*(source[12].zzzz)).z;
    // 36: exp r2.z, r2.z
    r2.z = (exp2(r2.zzzz)).z;
    // 37: min r2.z, r2.z, l(1.000000)
    r2.z = (min(r2.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 38: movc r2.x, r2.x, l(0), r2.z
    r2.x = ((asuint(r2.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.zzzz)).x;
    // 39: mad r1.xyz, r2.xxxx, r1.xyzx, r3.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 40: mad r3.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r3.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 41: mad r1.xyz, r1.xyzx, r2.xxxx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xxxx)+(r3.xyzx)).xyz;
    // 42: mul r1.xyz, r2.xxxx, r1.xyzx
    r1.xyz = ((r2.xxxx)*(r1.xyzx)).xyz;
    // 43: max r1.xyz, r1.xyzx, r2.xxxx
    r1.xyz = (max(r1.xyzx,r2.xxxx)).xyz;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, v4.xyxx, t0.zwxy, s0, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 45: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 46: dp2 r3.x, r2.zwzz, r2.zwzz
    r3.x = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).x;
    // 47: mul r2.zw, r2.zzzw, cb0[8].xxxx
    r2.zw = ((r2.zzzw)*(source[8].xxxx)).zw;
    // 48: mul r4.xy, r2.zwzz, v2.wwww
    r4.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 49: add r2.z, -r3.x, l(1.000000)
    r2.z = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: max r2.z, r2.z, l(0.000000)
    r2.z = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 51: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 52: add r4.z, r2.z, l(0.000010)
    r4.z = ((r2.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 53: dp3 r2.z, r4.xyzx, r4.xyzx
    r2.z = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 54: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 55: div r3.xyz, r4.xyzx, r2.zzzz
    r3.xyz = ((r4.xyzx)/(r2.zzzz)).xyz;
    // 56: dp3 r2.z, r3.xyzx, r3.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 57: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 58: mul r4.xyz, r2.zzzz, r3.xyzx
    r4.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 59: dp3 r2.z, v7.xyzx, v7.xyzx
    r2.z = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).z;
    // 60: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 61: mul r5.xyz, r2.zzzz, v7.xyzx
    r5.xyz = ((r2.zzzz)*(v7.xyzx)).xyz;
    // 62: dp3 r2.z, r5.xyzx, r4.xyzx
    r2.z = (dot((r5.xyzx).xyz,(r4.xyzx).xyz).xxxx).z;
    // 63: mad r2.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 64: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 65: mul r6.xyz, r2.wwww, cb0[24].xyzx
    r6.xyz = ((r2.wwww)*(source[24].xyzx)).xyz;
    // 66: mad r6.xyz, r2.zzzz, cb0[23].xyzx, r6.xyzx
    r6.xyz = ((r2.zzzz)*(source[23].xyzx)+(r6.xyzx)).xyz;
    // 67: mul r6.xyz, r6.xyzx, cb0[25].wwww
    r6.xyz = ((r6.xyzx)*(source[25].wwww)).xyz;
    // 68: mul r6.xyz, r0.xyzx, r6.xyzx
    r6.xyz = ((r0.xyzx)*(r6.xyzx)).xyz;
    // 69: mul r6.xyz, r1.xyzx, r6.xyzx
    r6.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 70: dp3 r2.z, v1.xyzx, v1.xyzx
    r2.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 71: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 72: mul r7.xyz, r2.zzzz, v1.xyzx
    r7.xyz = ((r2.zzzz)*(v1.xyzx)).xyz;
    // 73: dp3 r2.z, v0.xyzx, v0.xyzx
    r2.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 74: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 75: mul r8.xyz, r2.zzzz, v0.xyzx
    r8.xyz = ((r2.zzzz)*(v0.xyzx)).xyz;
    // 76: mul r9.xyz, r7.zxyz, r8.yzxy
    r9.xyz = ((r7.zxyz)*(r8.yzxy)).xyz;
    // 77: mad r9.xyz, r7.yzxy, r8.zxyz, -r9.xyzx
    r9.xyz = ((r7.yzxy)*(r8.zxyz)+(-(r9.xyzx))).xyz;
    // 78: mul r9.xyz, r9.xyzx, v1.wwww
    r9.xyz = ((r9.xyzx)*(v1.wwww)).xyz;
    // 79: dp3 r10.y, r9.xyzx, r4.xyzx
    r10.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 80: dp3 r10.x, r8.xyzx, r4.xyzx
    r10.x = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 81: dp2 r11.z, r10.xyxx, cb0[15].xyxx
    r11.z = (dot((r10.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 82: mul r2.zw, cb0[15].yyyx, l(0.000000, 0.000000, 1.000000, -1.000000)
    r2.zw = ((source[15].yyyx)*(float4(0.000000,0.000000,1.000000,-1.000000))).zw;
    // 83: dp2 r11.x, r10.xyxx, r2.zwzz
    r11.x = (dot((r10.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 84: dp3 r11.y, r7.xyzx, r4.xyzx
    r11.y = (dot((r7.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 85: mov r11.w, l(1.000000)
    r11.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 86: dp4 r10.x, cb0[16].xyzw, r11.xyzw
    r10.x = (dot((source[16].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).x;
    // 87: dp4 r10.y, cb0[17].xyzw, r11.xyzw
    r10.y = (dot((source[17].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).y;
    // 88: dp4 r10.z, cb0[18].xyzw, r11.xyzw
    r10.z = (dot((source[18].xyzw).xyzw,(r11.xyzw).xyzw).xxxx).z;
    // 89: mul r12.xyzw, r11.yzzx, r11.xyzz
    r12.xyzw = ((r11.yzzx)*(r11.xyzz)).xyzw;
    // 90: mul r3.w, r11.y, r11.y
    r3.w = ((r11.yyyy)*(r11.yyyy)).w;
    // 91: mad r3.w, r11.x, r11.x, -r3.w
    r3.w = ((r11.xxxx)*(r11.xxxx)+(-(r3.wwww))).w;
    // 92: dp4 r11.x, cb0[19].xyzw, r12.xyzw
    r11.x = (dot((source[19].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).x;
    // 93: dp4 r11.y, cb0[20].xyzw, r12.xyzw
    r11.y = (dot((source[20].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).y;
    // 94: dp4 r11.z, cb0[21].xyzw, r12.xyzw
    r11.z = (dot((source[21].xyzw).xyzw,(r12.xyzw).xyzw).xxxx).z;
    // 95: add r10.xyz, r10.xyzx, r11.xyzx
    r10.xyz = ((r10.xyzx)+(r11.xyzx)).xyz;
    // 96: mad r10.xyz, cb0[22].xyzx, r3.wwww, r10.xyzx
    r10.xyz = ((source[22].xyzx)*(r3.wwww)+(r10.xyzx)).xyz;
    // 97: max r10.xyz, r10.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r10.xyz = (max(r10.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 98: mul r10.xyz, r10.xyzx, cb0[14].xyzx
    r10.xyz = ((r10.xyzx)*(source[14].xyzx)).xyz;
    // 99: mad r10.xyz, r10.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[14].wwww
    r10.xyz = ((r10.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[14].wwww)).xyz;
    // 100: dp3 r3.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 101: log r4.w, |r2.y|
    r4.w = (log2(abs(r2.yyyy))).w;
    // 102: lt r2.y, |r2.y|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r2.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 103: mul r4.w, r4.w, cb0[12].x
    r4.w = ((r4.wwww)*(source[12].xxxx)).w;
    // 104: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 105: movc r2.y, r2.y, l(0), r4.w
    r2.y = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).y;
    // 106: max r2.y, r2.y, cb0[0].x
    r2.y = (max(r2.yyyy,source[0].xxxx)).y;
    // 107: min r2.y, r2.y, l(1.000000)
    r2.y = (min(r2.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 108: dp3 r4.w, v6.xyzx, v6.xyzx
    r4.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 109: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 110: mul r11.xyz, r4.wwww, v6.xyzx
    r11.xyz = ((r4.wwww)*(v6.xyzx)).xyz;
    // 111: dp3 r4.w, r4.xyzx, r11.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 112: deriv_rtx_coarse r12.x, r4.w
    r12.x = (ddx_coarse(r4.wwww)).x;
    // 113: deriv_rty_coarse r12.y, r4.w
    r12.y = (ddy_coarse(r4.wwww)).y;
    // 114: dp2 r5.w, r12.xyxx, r12.xyxx
    r5.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 115: sqrt r5.w, r5.w
    r5.w = (sqrt(r5.wwww)).w;
    // 116: mad_sat r12.y, r5.w, l(0.300000), r2.y
    r12.y = (saturate((r5.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r2.yyyy))).y;
    // 117: mad r2.y, r12.y, l(0.200000), l(0.200000)
    r2.y = ((r12.yyyy)*(float4(0.200000,0.200000,0.200000,0.200000))+(float4(0.200000,0.200000,0.200000,0.200000))).y;
    // 118: div r3.w, r3.w, r2.y
    r3.w = ((r3.wwww)/(r2.yyyy)).w;
    // 119: dp3 r5.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r5.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 120: mad r3.w, r5.w, l(5.000000), r3.w
    r3.w = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r3.wwww)).w;
    // 121: add r6.w, r0.w, cb0[11].y
    r6.w = ((r0.wwww)+(source[11].yyyy)).w;
    // 122: add r0.w, r0.w, cb0[10].w
    r0.w = ((r0.wwww)+(source[10].wwww)).w;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r7.w, v4.xyxx, t2.yzwx, s3, l(0.000000)
    r7.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 124: mad r6.w, r7.w, -r6.w, r6.w
    r6.w = ((r7.wwww)*(-(r6.wwww))+(r6.wwww)).w;
    // 125: add_sat r1.w, r1.w, r6.w
    r1.w = (saturate((r1.wwww)+(r6.wwww))).w;
    // 126: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 127: add_sat r3.w, r1.w, r3.w
    r3.w = (saturate((r1.wwww)+(r3.wwww))).w;
    // 128: mad r6.w, r3.w, l(-2.000000), l(3.000000)
    r6.w = ((r3.wwww)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 129: mul r3.w, r3.w, r3.w
    r3.w = ((r3.wwww)*(r3.wwww)).w;
    // 130: mul r3.w, r3.w, r6.w
    r3.w = ((r3.wwww)*(r6.wwww)).w;
    // 131: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 132: mul r3.w, r3.w, l(1.500000)
    r3.w = ((r3.wwww)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 133: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 134: mul r10.xyz, r3.wwww, r10.xyzx
    r10.xyz = ((r3.wwww)*(r10.xyzx)).xyz;
    // 135: mul r6.xyz, r6.xyzx, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 136: mul r13.xyz, r4.wwww, r4.xyzx
    r13.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 137: dp3 r3.w, -r5.xyzx, r4.xyzx
    r3.w = (dot((-(r5.xyzx)).xyz,(r4.xyzx).xyz).xxxx).w;
    // 138: mad r4.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 139: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 140: mad r13.xyz, r13.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r13.xyz = ((r13.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 141: add r3.w, r13.z, l(1.000000)
    r3.w = ((r13.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: add r4.z, r4.w, l(1.000000)
    r4.z = ((r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 144: mov_sat r4.w, r4.w
    r4.w = (saturate(r4.wwww)).w;
    // 145: log r4.w, r4.w
    r4.w = (log2(r4.wwww)).w;
    // 146: mul r4.w, r4.w, cb0[1].y
    r4.w = ((r4.wwww)*(source[1].yyyy)).w;
    // 147: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 148: mad_sat r4.w, r4.w, cb0[1].w, cb0[1].z
    r4.w = (saturate((r4.wwww)*(source[1].wwww)+(source[1].zzzz))).w;
    // 149: add_sat r12.x, -r3.w, r4.z
    r12.x = (saturate((-(r3.wwww))+(r4.zzzz))).x;
    // 150: sample_indexable(texture2d)(float,float,float,float) r12.zw, r12.xyxx, t5.zwxy, s6
    r12.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 151: add r14.xy, -r12.yxyy, l(1.000000, 1.000000, 0.000000, 0.000000)
    r14.xy = ((-(r12.yxyy))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 152: add r3.w, r2.x, r12.x
    r3.w = ((r2.xxxx)+(r12.xxxx)).w;
    // 153: log r3.w, r3.w
    r3.w = (log2(r3.wwww)).w;
    // 154: mov_sat r4.z, cb0[10].y
    r4.z = (saturate(source[10].yyyy)).z;
    // 155: mad r15.xyz, -r4.zzzz, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r15.xyz = ((-(r4.zzzz))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 156: mul r4.z, r4.z, l(0.080000)
    r4.z = ((r4.zzzz)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 157: mad r15.xyz, r1.wwww, r15.xyzx, r4.zzzz
    r15.xyz = ((r1.wwww)*(r15.xyzx)+(r4.zzzz)).xyz;
    // 158: max r14.xzw, r14.xxxx, r15.xxyz
    r14.xzw = (max(r14.xxxx,r15.xxyz)).xzw;
    // 159: add r14.xzw, -r15.xxyz, r14.xxzw
    r14.xzw = ((-(r15.xxyz))+(r14.xxzw)).xzw;
    // 160: mul_sat r4.z, r15.y, l(50.000000)
    r4.z = (saturate((r15.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 161: mul r14.xzw, r4.zzzz, r14.xxzw
    r14.xzw = ((r4.zzzz)*(r14.xxzw)).xzw;
    // 162: mul r16.xyz, r12.wwww, r15.xyzx
    r16.xyz = ((r12.wwww)*(r15.xyzx)).xyz;
    // 163: mad r14.xzw, r14.xxzw, r12.zzzz, r16.xxyz
    r14.xzw = ((r14.xxzw)*(r12.zzzz)+(r16.xxyz)).xzw;
    // 164: div r6.w, l(1.000000, 1.000000, 1.000000, 1.000000), r12.w
    r6.w = r12.w != 0.f ? 1.f / r12.w : 0.f;
    // 165: add r6.w, r6.w, l(-1.000000)
    r6.w = ((r6.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 166: mad r12.xzw, r15.xxyz, r6.wwww, l(1.000000, 0.000000, 1.000000, 1.000000)
    r12.xzw = ((r15.xxyz)*(r6.wwww)+(float4(1.000000,0.000000,1.000000,1.000000))).xzw;
    // 167: mad r16.xyz, -r14.xzwx, r12.xzwx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r16.xyz = ((-(r14.xzwx))*(r12.xzwx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mul r12.xzw, r12.xxzw, r14.xxzw
    r12.xzw = ((r12.xxzw)*(r14.xxzw)).xzw;
    // 169: mul r6.w, r14.y, r14.y
    r6.w = ((r14.yyyy)*(r14.yyyy)).w;
    // 170: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 171: mul r8.w, r14.y, r6.w
    r8.w = ((r14.yyyy)*(r6.wwww)).w;
    // 172: mad r6.w, -r6.w, r14.y, l(1.000000)
    r6.w = ((-(r6.wwww))*(r14.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: mul r14.xyz, r15.xyzx, r6.wwww
    r14.xyz = ((r15.xyzx)*(r6.wwww)).xyz;
    // 174: dp3 r6.w, r15.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r6.w = (dot((r15.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 175: mad r15.xyz, r6.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r15.xyz = ((r6.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 176: mad r14.xyz, r4.zzzz, r8.wwww, r14.xyzx
    r14.xyz = ((r4.zzzz)*(r8.wwww)+(r14.xyzx)).xyz;
    // 177: add r14.xyz, -r14.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r14.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 178: mul r14.xyz, r14.xyzx, r14.xyzx
    r14.xyz = ((r14.xyzx)*(r14.xyzx)).xyz;
    // 179: add r4.z, -r0.w, l(1.000000)
    r4.z = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 180: mad_sat r0.w, r7.w, r4.z, r0.w
    r0.w = (saturate((r7.wwww)*(r4.zzzz)+(r0.wwww))).w;
    // 181: mad r0.w, -r0.w, cb0[2].x, l(1.000000)
    r0.w = ((-(r0.wwww))*(source[2].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 182: mul r4.z, r12.y, r12.y
    r4.z = ((r12.yyyy)*(r12.yyyy)).z;
    // 183: mul r6.w, r12.y, l(5.000000)
    r6.w = ((r12.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 184: mad r7.w, r4.z, l(0.350000), l(1.000000)
    r7.w = ((r4.zzzz)*(float4(0.350000,0.350000,0.350000,0.350000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 185: mul r3.w, r3.w, r4.z
    r3.w = ((r3.wwww)*(r4.zzzz)).w;
    // 186: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 187: add r2.x, r2.x, r3.w
    r2.x = ((r2.xxxx)+(r3.wwww)).x;
    // 188: add_sat r2.x, r2.x, l(-1.000000)
    r2.x = (saturate((r2.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).x;
    // 189: div_sat r0.w, r0.w, r7.w
    r0.w = (saturate((r0.wwww)/(r7.wwww))).w;
    // 190: mad r17.xyz, -r0.wwww, r14.xyzx, r16.xyzx
    r17.xyz = ((-(r0.wwww))*(r14.xyzx)+(r16.xyzx)).xyz;
    // 191: mul r16.xyz, r0.xyzx, r16.xyzx
    r16.xyz = ((r0.xyzx)*(r16.xyzx)).xyz;
    // 192: mul r14.xyz, r14.xyzx, r0.wwww
    r14.xyz = ((r14.xyzx)*(r0.wwww)).xyz;
    // 193: mul r14.xyz, r0.xyzx, r14.xyzx
    r14.xyz = ((r0.xyzx)*(r14.xyzx)).xyz;
    // 194: mad r14.xyz, -r14.xyzx, r1.wwww, r14.xyzx
    r14.xyz = ((-(r14.xyzx))*(r1.wwww)+(r14.xyzx)).xyz;
    // 195: mul r6.xyz, r6.xyzx, r17.xyzx
    r6.xyz = ((r6.xyzx)*(r17.xyzx)).xyz;
    // 196: mad r6.xyz, -r6.xyzx, r1.wwww, r6.xyzx
    r6.xyz = ((-(r6.xyzx))*(r1.wwww)+(r6.xyzx)).xyz;
    // 197: dp3 r8.x, r8.xyzx, r13.xyzx
    r8.x = (dot((r8.xyzx).xyz,(r13.xyzx).xyz).xxxx).x;
    // 198: dp3 r8.y, r9.xyzx, r13.xyzx
    r8.y = (dot((r9.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 199: dp2 r9.x, r8.xyxx, r2.zwzz
    r9.x = (dot((r8.xyxx).xy,(r2.zwzz).xy).xxxx).x;
    // 200: dp2 r9.z, r8.xyxx, cb0[15].xyxx
    r9.z = (dot((r8.xyxx).xy,(source[15].xyxx).xy).xxxx).z;
    // 201: dp3 r9.y, r7.xyzx, r13.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r13.xyzx).xyz).xxxx).y;
    // 202: dp3 r2.z, r5.xyzx, r13.xyzx
    r2.z = (dot((r5.xyzx).xyz,(r13.xyzx).xyz).xxxx).z;
    // 203: mad r2.zw, r2.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r2.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 204: mul r2.zw, r2.zzzw, r2.zzzw
    r2.zw = ((r2.zzzw)*(r2.zzzw)).zw;
    // 205: sample_l_indexable(texturecube)(float,float,float,float) r7.xyzw, r9.xyzx, t6.xyzw, s5, r6.w
    r7.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r9.xyzx).xyz, (r6.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 206: mul r5.xyz, r7.xyzx, r7.wwww
    r5.xyz = ((r7.xyzx)*(r7.wwww)).xyz;
    // 207: mul r5.xyz, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((r5.xyzx)*(source[14].xyzx)).xyz;
    // 208: mad r5.xyz, r5.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[14].wwww)).xyz;
    // 209: dp3 r3.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 210: div r2.y, r3.w, r2.y
    r2.y = ((r3.wwww)/(r2.yyyy)).y;
    // 211: mad r2.y, r5.w, l(5.000000), r2.y
    r2.y = ((r5.wwww)*(float4(5.000000,5.000000,5.000000,5.000000))+(r2.yyyy)).y;
    // 212: add_sat r2.y, r1.w, r2.y
    r2.y = (saturate((r1.wwww)+(r2.yyyy))).y;
    // 213: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 214: mad r3.w, r2.y, l(-2.000000), l(3.000000)
    r3.w = ((r2.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 215: mul r2.y, r2.y, r2.y
    r2.y = ((r2.yyyy)*(r2.yyyy)).y;
    // 216: mul r2.y, r2.y, r3.w
    r2.y = ((r2.yyyy)*(r3.wwww)).y;
    // 217: log r2.y, r2.y
    r2.y = (log2(r2.yyyy)).y;
    // 218: mul r2.y, r2.y, l(1.500000)
    r2.y = ((r2.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 219: exp r2.y, r2.y
    r2.y = (exp2(r2.yyyy)).y;
    // 220: mul r5.xyz, r2.yyyy, r5.xyzx
    r5.xyz = ((r2.yyyy)*(r5.xyzx)).xyz;
    // 221: mad r2.y, r2.x, r15.x, r15.y
    r2.y = ((r2.xxxx)*(r15.xxxx)+(r15.yyyy)).y;
    // 222: mad r2.y, r2.y, r2.x, r15.z
    r2.y = ((r2.yyyy)*(r2.xxxx)+(r15.zzzz)).y;
    // 223: mul r2.y, r2.x, r2.y
    r2.y = ((r2.xxxx)*(r2.yyyy)).y;
    // 224: max r2.x, r2.y, r2.x
    r2.x = (max(r2.yyyy,r2.xxxx)).x;
    // 225: mul r7.xyz, r2.wwww, cb0[24].xyzx
    r7.xyz = ((r2.wwww)*(source[24].xyzx)).xyz;
    // 226: mad r2.yzw, cb0[23].xxyz, r2.zzzz, r7.xxyz
    r2.yzw = ((source[23].xxyz)*(r2.zzzz)+(r7.xxyz)).yzw;
    // 227: mul r2.yzw, r2.yyzw, cb0[25].wwww
    r2.yzw = ((r2.yyzw)*(source[25].wwww)).yzw;
    // 228: mul r2.yzw, r2.xxxx, r2.yyzw
    r2.yzw = ((r2.xxxx)*(r2.yyzw)).yzw;
    // 229: mul r2.yzw, r2.yyzw, r5.xxyz
    r2.yzw = ((r2.yyzw)*(r5.xxyz)).yzw;
    // 230: mul r5.xyz, r5.xyzx, r12.xzwx
    r5.xyz = ((r5.xyzx)*(r12.xzwx)).xyz;
    // 231: mad r2.yzw, r2.yyzw, r12.xxzw, r6.xxyz
    r2.yzw = ((r2.yyzw)*(r12.xxzw)+(r6.xxyz)).yzw;
    // 232: dp3 r3.x, r3.xyzx, r11.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 233: add r3.y, -|r11.z|, l(1.000000)
    r3.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 234: add r3.x, -|r3.x|, l(1.000000)
    r3.x = ((-(abs(r3.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 235: mul r3.x, r3.x, r3.y
    r3.x = ((r3.xxxx)*(r3.yyyy)).x;
    // 236: lt r3.y, |r3.x|, l(0.000001)
    r3.y = (asfloat((uint4)((abs(r3.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 237: log r3.x, |r3.x|
    r3.x = (log2(abs(r3.xxxx))).x;
    // 238: mul r3.x, r3.x, l(1.500000)
    r3.x = ((r3.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 239: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 240: mul r3.xzw, r3.xxxx, cb0[4].xxyz
    r3.xzw = ((r3.xxxx)*(source[4].xxyz)).xzw;
    // 241: movc r3.xyz, r3.yyyy, l(0,0,0,0), r3.xzwx
    r3.xyz = ((asuint(r3.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xzwx)).xyz;
    // 242: add r3.xyz, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(source[3].xyzx)).xyz;
    // 243: mul r6.xyz, r1.wwww, r16.xyzx
    r6.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 244: mul r6.xyz, r10.xyzx, r6.xyzx
    r6.xyz = ((r10.xyzx)*(r6.xyzx)).xyz;
    // 245: mul r1.xyz, r1.xyzx, r6.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)).xyz;
    // 246: mad r1.xyz, r5.xyzx, r2.xxxx, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r2.xxxx)+(r1.xyzx)).xyz;
    // 247: mul r1.xyz, r1.xyzx, l(0.300000, 0.300000, 0.300000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.300000,0.300000,0.300000,0.000000))).xyz;
    // 248: add r2.x, -r0.w, l(1.000000)
    r2.x = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 249: mad r3.xyz, r1.xyzx, r2.xxxx, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r2.xxxx)+(r3.xyzx)).xyz;
    // 250: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 251: mad r1.xyz, r1.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r14.xyzx
    r1.xyz = ((r1.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r14.xyzx)).xyz;
    // 252: mul r5.xyz, r4.yyyy, cb0[24].xyzx
    r5.xyz = ((r4.yyyy)*(source[24].xyzx)).xyz;
    // 253: mad r4.xyz, r4.xxxx, cb0[23].xyzx, r5.xyzx
    r4.xyz = ((r4.xxxx)*(source[23].xyzx)+(r5.xyzx)).xyz;
    // 254: mul r4.xyz, r4.xyzx, cb0[25].wwww
    r4.xyz = ((r4.xyzx)*(source[25].wwww)).xyz;
    // 255: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 256: add_sat r5.xyz, -r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (saturate((-(r5.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000)))).xyz;
    // 257: add_sat r5.xyz, r5.xyzx, cb0[13].xxxx
    r5.xyz = (saturate((r5.xyzx)+(source[13].xxxx))).xyz;
    // 258: mul r3.w, r5.x, cb0[13].y
    r3.w = ((r5.xxxx)*(source[13].yyyy)).w;
    // 259: mul_sat r5.xyz, r5.xyzx, cb0[7].xyzx
    r5.xyz = (saturate((r5.xyzx)*(source[7].xyzx))).xyz;
    // 260: mul r3.w, r3.w, r4.w
    r3.w = ((r3.wwww)*(r4.wwww)).w;
    // 261: mul r5.xyz, r5.xyzx, r3.wwww
    r5.xyz = ((r5.xyzx)*(r3.wwww)).xyz;
    // 262: mul r5.xyz, r1.wwww, r5.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 263: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 264: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 265: mad r3.xyz, r0.xyzx, r2.xxxx, r3.xyzx
    r3.xyz = ((r0.xyzx)*(r2.xxxx)+(r3.xyzx)).xyz;
    // 266: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 267: mad r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), r1.xyzx
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(r1.xyzx)).xyz;
    // 268: mul o1.xyz, r0.xyzx, v5.wwww
    output.targets[1].xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 269: add r0.xyz, r2.yzwy, r3.xyzx
    r0.xyz = ((r2.yzwy)+(r3.xyzx)).xyz;
    // 270: mad o0.xyz, v5.wwww, r0.xyzx, v5.xyzx
    output.targets[0].xyz = ((v5.wwww)*(r0.xyzx)+(v5.xyzx)).xyz;
    // 271: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 272: mov o1.w, l(1.000000)
    output.targets[1].w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 273: ret
    return output;
}

// source.character.static-map-native-1103.v1 / source program 973955ab8cc3df4095602c0a825a8a3c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1103(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[10].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f, r13=0.f, r14=0.f, r15=0.f, r16=0.f, r17=0.f, r18=0.f;
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
    // 6: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 7: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 8: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 9: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 10: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 11: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 12: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 13: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 14: mul r3.xyz, r3.xyzx, v1.wwww
    r3.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 16: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 17: mul r0.w, r4.z, cb0[11].z
    r0.w = ((r4.zzzz)*(source[11].zzzz)).w;
    // 18: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 19: mul r4.xy, r4.xyxx, cb0[8].xxxx
    r4.xy = ((r4.xyxx)*(source[8].xxxx)).xy;
    // 20: mul r4.xy, r4.xyxx, v2.wwww
    r4.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 21: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 23: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 24: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 29: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 30: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 31: dp3 r6.y, r3.xyzx, r5.xyzx
    r6.y = (dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 32: dp3 r6.x, r2.xyzx, r5.xyzx
    r6.x = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 33: dp2 r7.z, r6.xyxx, cb0[16].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 34: mul r8.xy, cb0[16].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[16].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 35: dp2 r7.x, r6.xyxx, r8.xyxx
    r7.x = (dot((r6.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 36: dp3 r7.y, r1.xyzx, r5.xyzx
    r7.y = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 37: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 38: dp4 r9.x, cb0[17].xyzw, r7.xyzw
    r9.x = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 39: dp4 r9.y, cb0[18].xyzw, r7.xyzw
    r9.y = (dot((source[18].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 40: dp4 r9.z, cb0[19].xyzw, r7.xyzw
    r9.z = (dot((source[19].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 41: mul r10.xyzw, r7.yzzx, r7.xyzz
    r10.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 42: dp4 r11.x, cb0[20].xyzw, r10.xyzw
    r11.x = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 43: dp4 r11.y, cb0[21].xyzw, r10.xyzw
    r11.y = (dot((source[21].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 44: dp4 r11.z, cb0[22].xyzw, r10.xyzw
    r11.z = (dot((source[22].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 45: add r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)+(r11.xyzx)).xyz;
    // 46: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 47: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 48: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 49: mad r7.xyz, cb0[23].xyzx, r1.wwww, r9.xyzx
    r7.xyz = ((source[23].xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 50: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // 52: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 53: mul r9.xyz, cb0[6].xyzx, cb0[11].wwww
    r9.xyz = ((source[6].xyzx)*(source[11].wwww)).xyz;
    // 54: mul r9.xyz, r0.xyzx, r9.xyzx
    r9.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 55: mul r10.xyz, cb0[7].xyzx, cb0[12].xxxx
    r10.xyz = ((source[7].xyzx)*(source[12].xxxx)).xyz;
    // 56: mad r0.xyz, r10.xyzx, r0.xyzx, -r9.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)+(-(r9.xyzx))).xyz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 58: mul r1.w, r10.z, cb0[12].y
    r1.w = ((r10.zzzz)*(source[12].yyyy)).w;
    // 59: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 60: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 61: mul r2.w, r2.w, cb0[12].z
    r2.w = ((r2.wwww)*(source[12].zzzz)).w;
    // 62: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 63: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 64: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 66: mad r0.xyz, r2.wwww, r0.xyzx, r9.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r9.xyzx)).xyz;
    // 67: mul r9.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r9.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 68: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 69: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 70: mul r11.xyz, r1.wwww, v5.xyzx
    r11.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 71: dp3 r1.w, r5.xyzx, r11.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r11.xyzx).xyz).xxxx).w;
    // 72: mul r12.xyz, r1.wwww, r5.xyzx
    r12.xyz = ((r1.wwww)*(r5.xyzx)).xyz;
    // 73: mad r12.xyz, r12.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r11.xyzx
    r12.xyz = ((r12.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r11.xyzx))).xyz;
    // 74: dp3 r3.y, r3.xyzx, r12.xyzx
    r3.y = (dot((r3.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 75: dp3 r3.x, r2.xyzx, r12.xyzx
    r3.x = (dot((r2.xyzx).xyz,(r12.xyzx).xyz).xxxx).x;
    // 76: mad r2.xy, cb0[11].xxxx, r3.xyxx, r9.xyxx
    r2.xy = ((source[11].xxxx)*(r3.xyxx)+(r9.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 78: mul r2.xyz, r2.xyzx, cb0[5].xyzx
    r2.xyz = ((r2.xyzx)*(source[5].xyzx)).xyz;
    // 79: mad r2.xyz, cb0[11].yyyy, r2.xyzx, r2.xyzx
    r2.xyz = ((source[11].yyyy)*(r2.xyzx)+(r2.xyzx)).xyz;
    // 80: add r2.xyz, r2.xyzx, -cb0[11].yyyy
    r2.xyz = ((r2.xyzx)+(-(source[11].yyyy))).xyz;
    // 81: mov_sat r9.xyz, r2.xyzx
    r9.xyz = (saturate(r2.xyzx)).xyz;
    // 82: mov_sat r2.xyz, -r2.xyzx
    r2.xyz = (saturate(-(r2.xyzx))).xyz;
    // 83: mad r2.xyz, -r0.wwww, r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r0.wwww))*(r2.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r0.xyz, r0.wwww, r9.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r9.xyzx)+(r0.xyzx)).xyz;
    // 85: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 86: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 87: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 88: mul r2.xyz, r0.xyzx, cb0[12].wwww
    r2.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 89: mad r0.xyz, cb0[13].xxxx, r0.xyzx, -r2.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(-(r2.xyzx))).xyz;
    // 90: mad r0.xyz, r2.wwww, r0.xyzx, r2.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 91: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 93: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 94: mov_sat r0.w, cb0[13].y
    r0.w = (saturate(source[13].yyyy)).w;
    // 95: mad r2.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r2.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 96: mul r2.w, r0.w, l(0.080000)
    r2.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 97: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 98: mad r2.xyz, r8.wwww, r2.xyzx, r2.wwww
    r2.xyz = ((r8.wwww)*(r2.xyzx)+(r2.wwww)).xyz;
    // 99: deriv_rtx_coarse r9.x, r1.w
    r9.x = (ddx_coarse(r1.wwww)).x;
    // 100: deriv_rty_coarse r9.y, r1.w
    r9.y = (ddy_coarse(r1.wwww)).y;
    // 101: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 103: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 104: mul r2.w, r10.y, cb0[13].w
    r2.w = ((r10.yyyy)*(source[13].wwww)).w;
    // 105: mul r3.z, r10.x, cb0[14].y
    r3.z = ((r10.xxxx)*(source[14].yyyy)).z;
    // 106: log r3.w, |r2.w|
    r3.w = (log2(abs(r2.wwww))).w;
    // 107: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 108: mul r3.w, r3.w, cb0[14].x
    r3.w = ((r3.wwww)*(source[14].xxxx)).w;
    // 109: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 110: movc r2.w, r2.w, l(0), r3.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 111: max r2.w, r2.w, cb0[0].w
    r2.w = (max(r2.wwww,source[0].wwww)).w;
    // 112: min r8.z, r2.w, l(1.000000)
    r8.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 113: mad_sat r9.y, r1.w, l(0.300000), r8.z
    r9.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 114: add r1.w, -r9.y, l(1.000000)
    r1.w = ((-(r9.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: max r10.xyz, r2.xyzx, r1.wwww
    r10.xyz = (max(r2.xyzx,r1.wwww)).xyz;
    // 116: add r10.xyz, -r2.xyzx, r10.xyzx
    r10.xyz = ((-(r2.xyzx))+(r10.xyzx)).xyz;
    // 117: mul_sat r1.w, r2.y, l(50.000000)
    r1.w = (saturate((r2.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 118: mul r10.xyz, r1.wwww, r10.xyzx
    r10.xyz = ((r1.wwww)*(r10.xyzx)).xyz;
    // 119: add r1.w, r12.z, l(1.000000)
    r1.w = ((r12.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 120: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: add_sat r9.x, r0.w, -r1.w
    r9.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 122: sample_indexable(texture2d)(float,float,float,float) r9.zw, r9.xyxx, t4.zwxy, s6
    r9.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 123: mul r13.xyz, r2.xyzx, r9.wwww
    r13.xyz = ((r2.xyzx)*(r9.wwww)).xyz;
    // 124: mad r10.xyz, r10.xyzx, r9.zzzz, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r9.zzzz)+(r13.xyzx)).xyz;
    // 125: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r9.w
    r0.w = r9.w != 0.f ? 1.f / r9.w : 0.f;
    // 126: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 127: mad r13.xyz, r2.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r2.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 128: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: mad r2.xyz, r0.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r2.xyz = ((r0.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 130: mad r14.xyz, -r10.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r10.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 131: mul r10.xyz, r10.xyzx, r13.xyzx
    r10.xyz = ((r10.xyzx)*(r13.xyzx)).xyz;
    // 132: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 133: dp2_sat r13.x, r5.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r13.x = (saturate(dot((r5.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 134: dp3_sat r13.y, r5.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r13.y = (saturate(dot((r5.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 135: dp3_sat r13.z, r5.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r13.z = (saturate(dot((r5.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 136: mul r13.xyz, r13.xyzx, r13.xyzx
    r13.xyz = ((r13.xyzx)*(r13.xyzx)).xyz;
    // 137: sample_indexable(texture2d)(float,float,float,float) r14.xyz, v3.zwzz, t7.xyzw, s4
    r14.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 138: mul r14.xyz, r14.xyzx, cb0[29].xyzx
    r14.xyz = ((r14.xyzx)*(source[29].xyzx)).xyz;
    // 139: dp3 r0.w, r14.xyzx, r13.xyzx
    r0.w = (dot((r14.xyzx).xyz,(r13.xyzx).xyz).xxxx).w;
    // 140: sample_indexable(texture2d)(float,float,float,float) r13.xyz, v3.zwzz, t6.xyzw, s4
    r13.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 141: mul r13.xyz, r13.xyzx, cb0[28].xyzx
    r13.xyz = ((r13.xyzx)*(source[28].xyzx)).xyz;
    // 142: mul r15.xyz, r0.wwww, r13.xyzx
    r15.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 143: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 144: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 145: mul r16.xyz, r1.wwww, v6.xyzx
    r16.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 146: dp3 r1.w, r16.xyzx, r5.xyzx
    r1.w = (dot((r16.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 147: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 148: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 149: mul r5.yzw, r5.yyyy, cb0[26].xxyz
    r5.yzw = ((r5.yyyy)*(source[26].xxyz)).yzw;
    // 150: mad r5.xyz, r5.xxxx, cb0[25].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[25].xyzx)+(r5.yzwy)).xyz;
    // 151: mul r5.xyz, r5.xyzx, cb0[27].wwww
    r5.xyz = ((r5.xyzx)*(source[27].wwww)).xyz;
    // 152: mul r16.xyz, r0.xyzx, r5.xyzx
    r16.xyz = ((r0.xyzx)*(r5.xyzx)).xyz;
    // 153: mad r15.xyz, r0.xyzx, r15.xyzx, r16.xyzx
    r15.xyz = ((r0.xyzx)*(r15.xyzx)+(r16.xyzx)).xyz;
    // 154: mad r16.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r16.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 155: mad r17.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r17.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 156: mad r18.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r18.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 157: log r1.w, |r3.z|
    r1.w = (log2(abs(r3.zzzz))).w;
    // 158: lt r2.w, |r3.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r3.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 159: mul r1.w, r1.w, cb0[14].z
    r1.w = ((r1.wwww)*(source[14].zzzz)).w;
    // 160: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 161: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 162: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 163: mad r17.xyz, r1.wwww, r17.xyzx, r18.xyzx
    r17.xyz = ((r1.wwww)*(r17.xyzx)+(r18.xyzx)).xyz;
    // 164: mad r16.xyz, r17.xyzx, r1.wwww, r16.xyzx
    r16.xyz = ((r17.xyzx)*(r1.wwww)+(r16.xyzx)).xyz;
    // 165: mul r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = ((r1.wwww)*(r16.xyzx)).xyz;
    // 166: max r16.xyz, r1.wwww, r16.xyzx
    r16.xyz = (max(r1.wwww,r16.xyzx)).xyz;
    // 167: mul r15.xyz, r15.xyzx, r16.xyzx
    r15.xyz = ((r15.xyzx)*(r16.xyzx)).xyz;
    // 168: mul r7.xyz, r7.xyzx, r15.xyzx
    r7.xyz = ((r7.xyzx)*(r15.xyzx)).xyz;
    // 169: mad r7.xyz, -r7.xyzx, r8.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.wwww)+(r7.xyzx)).xyz;
    // 170: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 171: dp2_sat r15.x, r12.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r15.x = (saturate(dot((r12.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 172: dp3_sat r15.y, r12.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r15.y = (saturate(dot((r12.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 173: dp3_sat r15.z, r12.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r15.z = (saturate(dot((r12.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 174: dp3 r1.y, r1.xyzx, r12.xyzx
    r1.y = (dot((r1.xyzx).xyz,(r12.xyzx).xyz).xxxx).y;
    // 175: mul r12.xyz, r15.xyzx, r15.xyzx
    r12.xyz = ((r15.xyzx)*(r15.xyzx)).xyz;
    // 176: dp3 r2.w, r14.xyzx, r12.xyzx
    r2.w = (dot((r14.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 177: add r0.w, r0.w, -r2.w
    r0.w = ((r0.wwww)+(-(r2.wwww))).w;
    // 178: mad r0.w, r8.z, r0.w, r2.w
    r0.w = ((r8.zzzz)*(r0.wwww)+(r2.wwww)).w;
    // 179: mad r5.xyz, r13.xyzx, r0.wwww, r5.xyzx
    r5.xyz = ((r13.xyzx)*(r0.wwww)+(r5.xyzx)).xyz;
    // 180: mul r12.xyz, r0.wwww, r13.xyzx
    r12.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 181: add r0.w, r1.w, r9.x
    r0.w = ((r1.wwww)+(r9.xxxx)).w;
    // 182: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 183: mul r2.w, r9.y, r9.y
    r2.w = ((r9.yyyy)*(r9.yyyy)).w;
    // 184: mul r3.z, r9.y, l(5.000000)
    r3.z = ((r9.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).z;
    // 185: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 186: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 187: add r0.w, r1.w, r0.w
    r0.w = ((r1.wwww)+(r0.wwww)).w;
    // 188: mov o5.y, r1.w
    output.targets[5].y = (r1.wwww).y;
    // 189: add_sat r0.w, r0.w, l(-1.000000)
    r0.w = (saturate((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 190: mad r1.w, r0.w, r2.x, r2.y
    r1.w = ((r0.wwww)*(r2.xxxx)+(r2.yyyy)).w;
    // 191: mad r1.w, r1.w, r0.w, r2.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r2.zzzz)).w;
    // 192: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 193: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 194: mul r2.xyz, r0.wwww, r5.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)).xyz;
    // 195: add r5.xyz, r5.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r5.xyz = ((r5.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 196: div r5.xyz, r12.xyzx, r5.xyzx
    r5.xyz = ((r12.xyzx)/(r5.xyzx)).xyz;
    // 197: dp3 r0.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 198: dp2 r1.x, r3.xyxx, r8.xyxx
    r1.x = (dot((r3.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 199: dp2 r1.z, r3.xyxx, cb0[16].xyxx
    r1.z = (dot((r3.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 200: sample_l_indexable(texturecube)(float,float,float,float) r1.xyzw, r1.xyzx, t5.xyzw, s5, r3.z
    r1.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r1.xyzx).xyz, (r3.zzzz).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 201: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 202: mul r1.xyz, r1.xyzx, cb0[15].xyzx
    r1.xyz = ((r1.xyzx)*(source[15].xyzx)).xyz;
    // 203: mad r1.xyz, r1.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 204: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 205: mad r2.xyz, r1.xyzx, r10.xyzx, r7.xyzx
    r2.xyz = ((r1.xyzx)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 206: mul r1.xyz, r10.xyzx, r1.xyzx
    r1.xyz = ((r10.xyzx)*(r1.xyzx)).xyz;
    // 207: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 208: dp3 r1.x, r4.xyzx, r11.xyzx
    r1.x = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 209: add r1.y, -|r11.z|, l(1.000000)
    r1.y = ((-(abs(r11.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 210: add r1.x, -|r1.x|, l(1.000000)
    r1.x = ((-(abs(r1.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 211: mul r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)*(r1.yyyy)).x;
    // 212: log r1.y, |r1.x|
    r1.y = (log2(abs(r1.xxxx))).y;
    // 213: mul r1.y, r1.y, l(1.500000)
    r1.y = ((r1.yyyy)*(float4(1.500000,1.500000,1.500000,1.500000))).y;
    // 214: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 215: mul r1.yzw, r1.yyyy, cb0[2].xxyz
    r1.yzw = ((r1.yyyy)*(source[2].xxyz)).yzw;
    // 216: lt r2.w, |r1.x|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 217: movc r1.yzw, r2.wwww, l(0,0,0,0), r1.yyzw
    r1.yzw = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyzw)).yzw;
    // 218: mad_sat r2.w, r1.x, cb0[8].w, -cb0[9].x
    r2.w = (saturate((r1.xxxx)*(source[8].wwww)+(-(source[9].xxxx)))).w;
    // 219: log r3.x, r2.w
    r3.x = (log2(r2.wwww)).x;
    // 220: lt r2.w, r2.w, l(0.000001)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 221: mul r3.x, r3.x, cb0[9].y
    r3.x = ((r3.xxxx)*(source[9].yyyy)).x;
    // 222: exp r3.x, r3.x
    r3.x = (exp2(r3.xxxx)).x;
    // 223: mul r3.xyz, r3.xxxx, cb0[3].xyzx
    r3.xyz = ((r3.xxxx)*(source[3].xyzx)).xyz;
    // 224: movc r3.xyz, r2.wwww, l(0,0,0,0), r3.xyzx
    r3.xyz = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.xyzx)).xyz;
    // 225: add r3.xyz, r3.xyzx, -cb0[3].xyzx
    r3.xyz = ((r3.xyzx)+(-(source[3].xyzx))).xyz;
    // 226: mad r3.xyz, cb0[3].wwww, r3.xyzx, cb0[3].xyzx
    r3.xyz = ((source[3].wwww)*(r3.xyzx)+(source[3].xyzx)).xyz;
    // 227: mul r4.xyz, cb0[4].xyzx, cb0[9].wwww
    r4.xyz = ((source[4].xyzx)*(source[9].wwww)).xyz;
    // 228: mul r4.xyz, r4.xyzx, cb0[10].zzzz
    r4.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 229: mul r4.xyz, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.xxxx)*(r4.xyzx)).xyz;
    // 230: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 231: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 232: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 233: mul r4.xyz, r4.xyzx, cb0[10].wwww
    r4.xyz = ((r4.xyzx)*(source[10].wwww)).xyz;
    // 234: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 235: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 236: add r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)+(r4.xyzx)).xyz;
    // 237: mad r1.xyz, r3.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.yzwy
    r1.xyz = ((r3.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.yzwy)).xyz;
    // 238: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 239: add r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)+(r1.xyzx)).xyz;
    // 240: mad o0.xyz, r0.xyzx, cb0[27].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 241: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 242: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 243: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 244: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 245: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 246: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 247: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 248: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 249: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 250: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 251: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 252: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 253: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 254: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 255: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 256: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
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

// source.character.static-map-native-1103.v1 / source program 5fea7a36aa773844888ae33487e245ea
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1103(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1103(input);
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
    source[10]=g_SourceCharacterBaseConstants[12];
    source[10].x=(sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0)))).x;
    source[10].y=((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))).x;
    source[10].z=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[11]=g_SourceCharacterBaseConstants[13];
    source[12]=g_SourceCharacterBaseConstants[14];
    source[13]=g_SourceCharacterBaseConstants[15];
    source[14]=g_SourceCharacterBaseConstants[16];
    if(g_SourceCharacterEnvironmentEnabled!=0u){source[15]=g_SourceCharacterEnvironmentColor;source[16]=g_SourceCharacterEnvironmentRotation;}
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
    // 8: mul r0.w, r1.z, cb0[11].z
    r0.w = ((r1.zzzz)*(source[11].zzzz)).w;
    // 9: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 10: mul r1.xy, r1.xyxx, cb0[8].xxxx
    r1.xy = ((r1.xyxx)*(source[8].xxxx)).xy;
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
    // 33: dp2 r7.z, r6.xyxx, cb0[16].xyxx
    r7.z = (dot((r6.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 34: mul r8.xy, cb0[16].yxyy, l(1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((source[16].yxyy)*(float4(1.000000,-1.000000,0.000000,0.000000))).xy;
    // 35: dp2 r7.x, r6.xyxx, r8.xyxx
    r7.x = (dot((r6.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 36: dp3 r7.y, r3.xyzx, r2.xyzx
    r7.y = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 37: mov r7.w, l(1.000000)
    r7.w = (float4(1.000000,1.000000,1.000000,1.000000)).w;
    // 38: dp4 r9.x, cb0[17].xyzw, r7.xyzw
    r9.x = (dot((source[17].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).x;
    // 39: dp4 r9.y, cb0[18].xyzw, r7.xyzw
    r9.y = (dot((source[18].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).y;
    // 40: dp4 r9.z, cb0[19].xyzw, r7.xyzw
    r9.z = (dot((source[19].xyzw).xyzw,(r7.xyzw).xyzw).xxxx).z;
    // 41: mul r10.xyzw, r7.yzzx, r7.xyzz
    r10.xyzw = ((r7.yzzx)*(r7.xyzz)).xyzw;
    // 42: dp4 r11.x, cb0[20].xyzw, r10.xyzw
    r11.x = (dot((source[20].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).x;
    // 43: dp4 r11.y, cb0[21].xyzw, r10.xyzw
    r11.y = (dot((source[21].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).y;
    // 44: dp4 r11.z, cb0[22].xyzw, r10.xyzw
    r11.z = (dot((source[22].xyzw).xyzw,(r10.xyzw).xyzw).xxxx).z;
    // 45: add r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)+(r11.xyzx)).xyz;
    // 46: mul r1.w, r7.y, r7.y
    r1.w = ((r7.yyyy)*(r7.yyyy)).w;
    // 47: mov r6.z, r7.y
    r6.z = (r7.yyyy).z;
    // 48: mad r1.w, r7.x, r7.x, -r1.w
    r1.w = ((r7.xxxx)*(r7.xxxx)+(-(r1.wwww))).w;
    // 49: mad r7.xyz, cb0[23].xyzx, r1.wwww, r9.xyzx
    r7.xyz = ((source[23].xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 50: max r7.xyz, r7.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xyz = (max(r7.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 51: mul r7.xyz, r7.xyzx, cb0[15].xyzx
    r7.xyz = ((r7.xyzx)*(source[15].xyzx)).xyz;
    // 52: mad r7.xyz, r7.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000), cb0[15].wwww
    r7.xyz = ((r7.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))+(source[15].wwww)).xyz;
    // 53: mul r9.xyz, cb0[6].xyzx, cb0[11].wwww
    r9.xyz = ((source[6].xyzx)*(source[11].wwww)).xyz;
    // 54: mul r9.xyz, r0.xyzx, r9.xyzx
    r9.xyz = ((r0.xyzx)*(r9.xyzx)).xyz;
    // 55: mul r10.xyz, cb0[7].xyzx, cb0[12].xxxx
    r10.xyz = ((source[7].xyzx)*(source[12].xxxx)).xyz;
    // 56: mad r0.xyz, r10.xyzx, r0.xyzx, -r9.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)+(-(r9.xyzx))).xyz;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r10.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r10.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 58: mul r1.w, r10.z, cb0[12].y
    r1.w = ((r10.zzzz)*(source[12].yyyy)).w;
    // 59: log r2.w, |r1.w|
    r2.w = (log2(abs(r1.wwww))).w;
    // 60: lt r1.w, |r1.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 61: mul r2.w, r2.w, cb0[12].z
    r2.w = ((r2.wwww)*(source[12].zzzz)).w;
    // 62: exp r2.w, r2.w
    r2.w = (exp2(r2.wwww)).w;
    // 63: movc r1.w, r1.w, l(0), r2.w
    r1.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.wwww)).w;
    // 64: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mul_sat r8.w, r1.w, cb2[3].w
    r8.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 66: mad r0.xyz, r2.wwww, r0.xyzx, r9.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r9.xyzx)).xyz;
    // 67: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 68: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 69: mul r9.xyz, r1.wwww, v5.xyzx
    r9.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 70: dp3 r1.w, r2.xyzx, r9.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 71: mul r11.xyz, r1.wwww, r2.xyzx
    r11.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 72: mad r11.xyz, r11.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r9.xyzx
    r11.xyz = ((r11.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r9.xyzx))).xyz;
    // 73: dp3 r5.y, r5.xyzx, r11.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 74: dp3 r5.x, r4.xyzx, r11.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 75: mul r4.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r4.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 76: mad r4.xy, cb0[11].xxxx, r5.xyxx, r4.xyxx
    r4.xy = ((source[11].xxxx)*(r5.xyxx)+(r4.xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t2.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 78: mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // 79: mad r4.xyz, cb0[11].yyyy, r4.xyzx, r4.xyzx
    r4.xyz = ((source[11].yyyy)*(r4.xyzx)+(r4.xyzx)).xyz;
    // 80: add r4.xyz, r4.xyzx, -cb0[11].yyyy
    r4.xyz = ((r4.xyzx)+(-(source[11].yyyy))).xyz;
    // 81: mov_sat r12.xyz, r4.xyzx
    r12.xyz = (saturate(r4.xyzx)).xyz;
    // 82: mov_sat r4.xyz, -r4.xyzx
    r4.xyz = (saturate(-(r4.xyzx))).xyz;
    // 83: mad r4.xyz, -r0.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mad r0.xyz, r0.wwww, r12.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r12.xyzx)+(r0.xyzx)).xyz;
    // 85: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 86: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 87: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 88: mul r4.xyz, r0.xyzx, cb0[12].wwww
    r4.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 89: mad r0.xyz, cb0[13].xxxx, r0.xyzx, -r4.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(-(r4.xyzx))).xyz;
    // 90: mad r0.xyz, r2.wwww, r0.xyzx, r4.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r4.xyzx)).xyz;
    // 91: add r4.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 92: mul r0.xyz, r0.xyzx, r4.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)).xyz;
    // 93: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 94: mov_sat r0.w, cb0[13].y
    r0.w = (saturate(source[13].yyyy)).w;
    // 95: mad r4.xyz, -r0.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r4.xyz = ((-(r0.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 96: mul r2.w, r0.w, l(0.080000)
    r2.w = ((r0.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 97: mov o3.xyzw, r0.xyzw
    output.targets[3].xyzw = (r0.xyzw).xyzw;
    // 98: mad r4.xyz, r8.wwww, r4.xyzx, r2.wwww
    r4.xyz = ((r8.wwww)*(r4.xyzx)+(r2.wwww)).xyz;
    // 99: deriv_rtx_coarse r12.x, r1.w
    r12.x = (ddx_coarse(r1.wwww)).x;
    // 100: deriv_rty_coarse r12.y, r1.w
    r12.y = (ddy_coarse(r1.wwww)).y;
    // 101: add r0.w, r1.w, l(1.000000)
    r0.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: dp2 r1.w, r12.xyxx, r12.xyxx
    r1.w = (dot((r12.xyxx).xy,(r12.xyxx).xy).xxxx).w;
    // 103: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 104: mul r2.w, r10.y, cb0[13].w
    r2.w = ((r10.yyyy)*(source[13].wwww)).w;
    // 105: mul r3.w, r10.x, cb0[14].y
    r3.w = ((r10.xxxx)*(source[14].yyyy)).w;
    // 106: log r4.w, |r2.w|
    r4.w = (log2(abs(r2.wwww))).w;
    // 107: lt r2.w, |r2.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r2.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 108: mul r4.w, r4.w, cb0[14].x
    r4.w = ((r4.wwww)*(source[14].xxxx)).w;
    // 109: exp r4.w, r4.w
    r4.w = (exp2(r4.wwww)).w;
    // 110: movc r2.w, r2.w, l(0), r4.w
    r2.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.wwww)).w;
    // 111: max r2.w, r2.w, cb0[0].w
    r2.w = (max(r2.wwww,source[0].wwww)).w;
    // 112: min r8.z, r2.w, l(1.000000)
    r8.z = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 113: mad_sat r10.y, r1.w, l(0.300000), r8.z
    r10.y = (saturate((r1.wwww)*(float4(0.300000,0.300000,0.300000,0.300000))+(r8.zzzz))).y;
    // 114: mov o2.zw, r8.zzzw
    output.targets[2].zw = (r8.zzzw).zw;
    // 115: add r1.w, -r10.y, l(1.000000)
    r1.w = ((-(r10.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 116: max r12.xyz, r4.xyzx, r1.wwww
    r12.xyz = (max(r4.xyzx,r1.wwww)).xyz;
    // 117: add r12.xyz, -r4.xyzx, r12.xyzx
    r12.xyz = ((-(r4.xyzx))+(r12.xyzx)).xyz;
    // 118: mul_sat r1.w, r4.y, l(50.000000)
    r1.w = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).w;
    // 119: mul r12.xyz, r1.wwww, r12.xyzx
    r12.xyz = ((r1.wwww)*(r12.xyzx)).xyz;
    // 120: add r1.w, r11.z, l(1.000000)
    r1.w = ((r11.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: min r1.w, r1.w, l(1.000000)
    r1.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add_sat r10.x, r0.w, -r1.w
    r10.x = (saturate((r0.wwww)+(-(r1.wwww)))).x;
    // 123: sample_indexable(texture2d)(float,float,float,float) r5.zw, r10.xyxx, t4.zwxy, s5
    r5.zw = ((float4(0.0,0.0,0.0,0.0)).zwxy).zw;
    // 124: mul r13.xyz, r4.xyzx, r5.wwww
    r13.xyz = ((r4.xyzx)*(r5.wwww)).xyz;
    // 125: mad r12.xyz, r12.xyzx, r5.zzzz, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r5.zzzz)+(r13.xyzx)).xyz;
    // 126: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r5.w
    r0.w = r5.w != 0.f ? 1.f / r5.w : 0.f;
    // 127: add r0.w, r0.w, l(-1.000000)
    r0.w = ((r0.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).w;
    // 128: mad r13.xyz, r4.xyzx, r0.wwww, l(1.000000, 1.000000, 1.000000, 0.000000)
    r13.xyz = ((r4.xyzx)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 129: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 130: mad r4.xyz, r0.wwww, l(2.040400, -4.795100, 2.755200, 0.000000), l(-0.332400, 0.641700, 0.690300, 0.000000)
    r4.xyz = ((r0.wwww)*(float4(2.040400,-4.795100,2.755200,0.000000))+(float4(-0.332400,0.641700,0.690300,0.000000))).xyz;
    // 131: mad r14.xyz, -r12.xyzx, r13.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r14.xyz = ((-(r12.xyzx))*(r13.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 132: mul r12.xyz, r12.xyzx, r13.xyzx
    r12.xyz = ((r12.xyzx)*(r13.xyzx)).xyz;
    // 133: mul r7.xyz, r7.xyzx, r14.xyzx
    r7.xyz = ((r7.xyzx)*(r14.xyzx)).xyz;
    // 134: mad r13.xyz, r0.xyzx, l(2.755200, 2.755200, 2.755200, 0.000000), l(0.690300, 0.690300, 0.690300, 0.000000)
    r13.xyz = ((r0.xyzx)*(float4(2.755200,2.755200,2.755200,0.000000))+(float4(0.690300,0.690300,0.690300,0.000000))).xyz;
    // 135: mad r14.xyz, r0.xyzx, l(2.040400, 2.040400, 2.040400, 0.000000), l(-0.332400, -0.332400, -0.332400, 0.000000)
    r14.xyz = ((r0.xyzx)*(float4(2.040400,2.040400,2.040400,0.000000))+(float4(-0.332400,-0.332400,-0.332400,0.000000))).xyz;
    // 136: mad r15.xyz, r0.xyzx, l(-4.795100, -4.795100, -4.795100, 0.000000), l(0.641700, 0.641700, 0.641700, 0.000000)
    r15.xyz = ((r0.xyzx)*(float4(-4.795100,-4.795100,-4.795100,0.000000))+(float4(0.641700,0.641700,0.641700,0.000000))).xyz;
    // 137: log r0.w, |r3.w|
    r0.w = (log2(abs(r3.wwww))).w;
    // 138: lt r1.w, |r3.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 139: mul r0.w, r0.w, cb0[14].z
    r0.w = ((r0.wwww)*(source[14].zzzz)).w;
    // 140: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 141: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 142: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 143: mad r14.xyz, r0.wwww, r14.xyzx, r15.xyzx
    r14.xyz = ((r0.wwww)*(r14.xyzx)+(r15.xyzx)).xyz;
    // 144: mad r13.xyz, r14.xyzx, r0.wwww, r13.xyzx
    r13.xyz = ((r14.xyzx)*(r0.wwww)+(r13.xyzx)).xyz;
    // 145: mul r13.xyz, r0.wwww, r13.xyzx
    r13.xyz = ((r0.wwww)*(r13.xyzx)).xyz;
    // 146: max r13.xyz, r0.wwww, r13.xyzx
    r13.xyz = (max(r0.wwww,r13.xyzx)).xyz;
    // 147: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 148: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 149: mul r14.xyz, r1.wwww, v6.xyzx
    r14.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 150: dp3 r1.w, r14.xyzx, r2.xyzx
    r1.w = (dot((r14.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 151: dp3 r2.x, r14.xyzx, r11.xyzx
    r2.x = (dot((r14.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 152: dp3 r3.y, r3.xyzx, r11.xyzx
    r3.y = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 153: mad r2.xy, r2.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r2.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 154: mad r2.zw, r1.wwww, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r2.zw = ((r1.wwww)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 155: mul r2.xyzw, r2.xyzw, r2.xyzw
    r2.xyzw = ((r2.xyzw)*(r2.xyzw)).xyzw;
    // 156: mul r11.xyz, r2.wwww, cb0[26].xyzx
    r11.xyz = ((r2.wwww)*(source[26].xyzx)).xyz;
    // 157: mad r11.xyz, r2.zzzz, cb0[25].xyzx, r11.xyzx
    r11.xyz = ((r2.zzzz)*(source[25].xyzx)+(r11.xyzx)).xyz;
    // 158: mul r11.xyz, r11.xyzx, cb0[27].wwww
    r11.xyz = ((r11.xyzx)*(source[27].wwww)).xyz;
    // 159: mul r11.xyz, r0.xyzx, r11.xyzx
    r11.xyz = ((r0.xyzx)*(r11.xyzx)).xyz;
    // 160: mul r11.xyz, r13.xyzx, r11.xyzx
    r11.xyz = ((r13.xyzx)*(r11.xyzx)).xyz;
    // 161: mul r7.xyz, r7.xyzx, r11.xyzx
    r7.xyz = ((r7.xyzx)*(r11.xyzx)).xyz;
    // 162: mad r7.xyz, -r7.xyzx, r8.wwww, r7.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.wwww)+(r7.xyzx)).xyz;
    // 163: dp2 r3.x, r5.xyxx, r8.xyxx
    r3.x = (dot((r5.xyxx).xy,(r8.xyxx).xy).xxxx).x;
    // 164: dp2 r3.z, r5.xyxx, cb0[16].xyxx
    r3.z = (dot((r5.xyxx).xy,(source[16].xyxx).xy).xxxx).z;
    // 165: mul r1.w, r10.y, l(5.000000)
    r1.w = ((r10.yyyy)*(float4(5.000000,5.000000,5.000000,5.000000))).w;
    // 166: mul r2.z, r10.y, r10.y
    r2.z = ((r10.yyyy)*(r10.yyyy)).z;
    // 167: sample_l_indexable(texturecube)(float,float,float,float) r3.xyzw, r3.xyzx, t5.xyzw, s4, r1.w
    r3.xyzw = (((g_SourceCharacterEnvironmentEnabled != 0u ? g_SourceCharacterEnvironmentCube.SampleLevel(SourceCharacterLookupSampler, (r3.xyzx).xyz, (r1.wwww).x) : float4(0.0,0.0,0.0,0.0))).xyzw).xyzw;
    // 168: mul r3.xyz, r3.xyzx, r3.wwww
    r3.xyz = ((r3.xyzx)*(r3.wwww)).xyz;
    // 169: mul r3.xyz, r3.xyzx, cb0[15].xyzx
    r3.xyz = ((r3.xyzx)*(source[15].xyzx)).xyz;
    // 170: mad r3.xyz, r3.xyzx, l(6.000000, 6.000000, 6.000000, 0.000000), cb0[15].wwww
    r3.xyz = ((r3.xyzx)*(float4(6.000000,6.000000,6.000000,0.000000))+(source[15].wwww)).xyz;
    // 171: add r1.w, r0.w, r10.x
    r1.w = ((r0.wwww)+(r10.xxxx)).w;
    // 172: log r1.w, r1.w
    r1.w = (log2(r1.wwww)).w;
    // 173: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 174: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 175: add r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)+(r1.wwww)).w;
    // 176: mov o5.y, r0.w
    output.targets[5].y = (r0.wwww).y;
    // 177: add_sat r0.w, r1.w, l(-1.000000)
    r0.w = (saturate((r1.wwww)+(float4(-1.000000,-1.000000,-1.000000,-1.000000)))).w;
    // 178: mad r1.w, r0.w, r4.x, r4.y
    r1.w = ((r0.wwww)*(r4.xxxx)+(r4.yyyy)).w;
    // 179: mad r1.w, r1.w, r0.w, r4.z
    r1.w = ((r1.wwww)*(r0.wwww)+(r4.zzzz)).w;
    // 180: mul r1.w, r0.w, r1.w
    r1.w = ((r0.wwww)*(r1.wwww)).w;
    // 181: max r0.w, r0.w, r1.w
    r0.w = (max(r0.wwww,r1.wwww)).w;
    // 182: mul r2.yzw, r2.yyyy, cb0[26].xxyz
    r2.yzw = ((r2.yyyy)*(source[26].xxyz)).yzw;
    // 183: mad r2.xyz, cb0[25].xyzx, r2.xxxx, r2.yzwy
    r2.xyz = ((source[25].xyzx)*(r2.xxxx)+(r2.yzwy)).xyz;
    // 184: mul r2.xyz, r2.xyzx, cb0[27].wwww
    r2.xyz = ((r2.xyzx)*(source[27].wwww)).xyz;
    // 185: mul r2.xyz, r0.wwww, r2.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 186: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 187: mad r3.xyz, r2.xyzx, r12.xyzx, r7.xyzx
    r3.xyz = ((r2.xyzx)*(r12.xyzx)+(r7.xyzx)).xyz;
    // 188: mul r2.xyz, r12.xyzx, r2.xyzx
    r2.xyz = ((r12.xyzx)*(r2.xyzx)).xyz;
    // 189: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 190: dp3 r0.w, r1.xyzx, r9.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 191: add r1.x, -|r9.z|, l(1.000000)
    r1.x = ((-(abs(r9.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 192: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 193: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 194: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 195: mul r1.x, r1.x, l(1.500000)
    r1.x = ((r1.xxxx)*(float4(1.500000,1.500000,1.500000,1.500000))).x;
    // 196: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 197: mul r1.xyz, r1.xxxx, cb0[2].xyzx
    r1.xyz = ((r1.xxxx)*(source[2].xyzx)).xyz;
    // 198: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 199: movc r1.xyz, r1.wwww, l(0,0,0,0), r1.xyzx
    r1.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 200: mad_sat r1.w, r0.w, cb0[8].w, -cb0[9].x
    r1.w = (saturate((r0.wwww)*(source[8].wwww)+(-(source[9].xxxx)))).w;
    // 201: log r2.x, r1.w
    r2.x = (log2(r1.wwww)).x;
    // 202: lt r1.w, r1.w, l(0.000001)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 203: mul r2.x, r2.x, cb0[9].y
    r2.x = ((r2.xxxx)*(source[9].yyyy)).x;
    // 204: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 205: mul r2.xyz, r2.xxxx, cb0[3].xyzx
    r2.xyz = ((r2.xxxx)*(source[3].xyzx)).xyz;
    // 206: movc r2.xyz, r1.wwww, l(0,0,0,0), r2.xyzx
    r2.xyz = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xyzx)).xyz;
    // 207: add r2.xyz, r2.xyzx, -cb0[3].xyzx
    r2.xyz = ((r2.xyzx)+(-(source[3].xyzx))).xyz;
    // 208: mad r2.xyz, cb0[3].wwww, r2.xyzx, cb0[3].xyzx
    r2.xyz = ((source[3].wwww)*(r2.xyzx)+(source[3].xyzx)).xyz;
    // 209: mul r4.xyz, cb0[4].xyzx, cb0[9].wwww
    r4.xyz = ((source[4].xyzx)*(source[9].wwww)).xyz;
    // 210: mul r4.xyz, r4.xyzx, cb0[10].zzzz
    r4.xyz = ((r4.xyzx)*(source[10].zzzz)).xyz;
    // 211: mul r4.xyz, r0.wwww, r4.xyzx
    r4.xyz = ((r0.wwww)*(r4.xyzx)).xyz;
    // 212: mul r4.xyz, r4.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000)
    r4.xyz = ((r4.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))).xyz;
    // 213: max r4.xyz, |r4.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r4.xyz = (max(abs(r4.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 214: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 215: mul r4.xyz, r4.xyzx, cb0[10].wwww
    r4.xyz = ((r4.xyzx)*(source[10].wwww)).xyz;
    // 216: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 217: min r4.xyz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 218: add r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)+(r4.xyzx)).xyz;
    // 219: mad r1.xyz, r2.xyzx, l(0.250000, 0.250000, 0.250000, 0.000000), r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(0.250000,0.250000,0.250000,0.000000))+(r1.xyzx)).xyz;
    // 220: add r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)+(source[1].xyzx)).xyz;
    // 221: add r1.xyz, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.xyzx)+(r1.xyzx)).xyz;
    // 222: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 223: mad o0.xyz, r0.xyzx, cb0[27].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[27].xyzx)+(r1.xyzx)).xyz;
    // 224: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 225: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 226: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 227: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
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
    // 237: ftou r0.x, cb0[24].z
    r0.x = (asfloat((uint4)(source[24].zzzz))).x;
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

// source.character.static-map-native-1104.v1 / source program eec0620f29551b4f996dd4c1d2884233
#else // SOURCE_CHARACTER_BASE_DISPATCH_CASES
    case 1100u: return SourceCharacterBase1100(input);
    case 1101u: return SourceCharacterBase1101(input);
    case 1102u: return SourceCharacterBase1102(input);
    case 1103u: return SourceCharacterBase1103(input);
#endif // SOURCE_CHARACTER_BASE_DISPATCH_CASES
