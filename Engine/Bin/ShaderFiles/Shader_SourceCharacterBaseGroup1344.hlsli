SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1400(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[9]=1.f;
    source[10]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, cb0[5].y
    r0.x = ((-(v2.wwww))+(source[5].yyyy)).x;
    // 2: add r0.x, r0.x, cb0[5].x
    r0.x = ((r0.xxxx)+(source[5].xxxx)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 22: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // 27: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 32: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 33: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 34: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 35: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 36: mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 39: mul r6.xyz, r5.xyzx, r6.xyzx
    r6.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 40: mad r6.xyz, r6.xyzx, cb2[3].wwww, cb2[3].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 41: mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // 42: mul r5.xyz, r5.xyzx, r7.xyzx
    r5.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 43: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 44: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 45: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 46: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 47: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 48: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 49: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 50: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 51: add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 52: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 53: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 54: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 55: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t2.xyzw, s2
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 56: mul r8.xyz, r8.xyzx, cb0[9].xyzx
    r8.xyz = ((r8.xyzx)*(source[9].xyzx)).xyz;
    // 57: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t3.xyzw, s2
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 58: mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // 59: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 60: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 61: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 62: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 63: mul r7.xyz, r2.wwww, r5.xyzx
    r7.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 64: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 65: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 66: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 67: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 68: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 69: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 70: mul r9.yzw, r9.yyyy, cb0[7].xxyz
    r9.yzw = ((r9.yyyy)*(source[7].xxyz)).yzw;
    // 71: mad r9.xyz, r9.xxxx, cb0[6].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[6].xyzx)+(r9.yzwy)).xyz;
    // 72: mul r9.xyz, r9.xyzx, cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(source[8].wwww)).xyz;
    // 73: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 74: mad r10.xyz, r6.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r6.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 75: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 76: mad r5.xyz, r5.xyzx, r2.wwww, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 77: add r9.xyz, r5.xyzx, cb0[1].xyzx
    r9.xyz = ((r5.xyzx)+(source[1].xyzx)).xyz;
    // 78: mad r9.xyz, r6.xyzx, cb0[8].xyzx, r9.xyzx
    r9.xyz = ((r6.xyzx)*(source[8].xyzx)+(r9.xyzx)).xyz;
    // 79: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 80: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 81: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 82: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 83: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 84: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 85: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 86: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 87: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 88: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 89: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 90: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 91: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 92: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 93: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 94: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 95: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 96: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 97: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 98: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 99: mul o4.z, r0.y, r5.x
    output.targets[4].z = ((r0.yyyy)*(r5.xxxx)).z;
    // 100: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 101: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 102: mov o3.xyz, r6.xyzx
    output.targets[3].xyz = (r6.xyzx).xyz;
    // 103: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 104: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 106: ret
    return output;
}

// source.character.static-map-native-1400.v1 / source program e1cb414df669fe4388b0d863864db470
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1400(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1400(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.x, -v2.w, cb0[4].y
    r0.x = ((-(v2.wwww))+(source[4].yyyy)).x;
    // 2: add r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)+(source[4].xxxx)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // 24: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 29: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 30: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: mul r5.xyz, cb0[2].xyzx, cb0[3].yyyy
    r5.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // 33: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 34: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 35: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 36: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 37: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 38: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 39: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 41: mul r5.yzw, r5.yyyy, cb0[6].xxyz
    r5.yzw = ((r5.yyyy)*(source[6].xxyz)).yzw;
    // 42: mad r5.xyz, r5.xxxx, cb0[5].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[5].xyzx)+(r5.yzwy)).xyz;
    // 43: mul r5.xyz, r5.xyzx, cb0[7].wwww
    r5.xyz = ((r5.xyzx)*(source[7].wwww)).xyz;
    // 44: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 45: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad r5.xyz, r4.xyzx, cb0[7].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[7].xyzx)+(r5.xyzx)).xyz;
    // 47: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 48: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 49: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 50: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 51: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 54: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 55: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 56: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 57: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 58: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 59: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 60: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 61: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 62: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 63: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 64: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 65: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 66: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 67: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 68: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 69: ret
    return output;
}

// source.character.static-map-native-1401.v1 / source program 63e66660d04ed94ebd19c0377a126a16
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1401(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[9]=1.f;
    source[10]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, cb0[5].y
    r0.x = ((-(v2.wwww))+(source[5].yyyy)).x;
    // 2: add r0.x, r0.x, cb0[5].x
    r0.x = ((r0.xxxx)+(source[5].xxxx)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 22: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: mul r4.xy, r4.xyxx, cb0[4].xxxx
    r4.xy = ((r4.xyxx)*(source[4].xxxx)).xy;
    // 27: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 32: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 33: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 34: dp3 r1.w, r4.xyzx, r3.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 35: mul r5.xyz, r1.wwww, r4.xyzx
    r5.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 36: mad r3.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: mul r6.xyz, cb0[2].xyzx, cb0[4].yyyy
    r6.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 39: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 40: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: mul r7.xyz, cb0[3].xyzx, cb0[4].zzzz
    r7.xyz = ((source[3].xyzx)*(source[4].zzzz)).xyz;
    // 43: mul r6.xyz, r6.xyzx, r7.xyzx
    r6.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 44: mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 45: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 46: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 47: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 48: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 49: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 50: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 51: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 52: add r1.w, cb0[4].w, l(1.000000)
    r1.w = ((source[4].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 54: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 55: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 56: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t3.xyzw, s3
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 57: mul r8.xyz, r8.xyzx, cb0[9].xyzx
    r8.xyz = ((r8.xyzx)*(source[9].xyzx)).xyz;
    // 58: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t4.xyzw, s3
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 59: mul r9.xyz, r9.xyzx, cb0[10].xyzx
    r9.xyz = ((r9.xyzx)*(source[10].xyzx)).xyz;
    // 60: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 61: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 62: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 63: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 64: mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 65: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 66: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 67: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 68: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 69: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 70: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 71: mul r9.yzw, r9.yyyy, cb0[7].xxyz
    r9.yzw = ((r9.yyyy)*(source[7].xxyz)).yzw;
    // 72: mad r9.xyz, r9.xxxx, cb0[6].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[6].xyzx)+(r9.yzwy)).xyz;
    // 73: mul r9.xyz, r9.xyzx, cb0[8].wwww
    r9.xyz = ((r9.xyzx)*(source[8].wwww)).xyz;
    // 74: mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 75: mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 76: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 77: mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 78: add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // 79: mad r9.xyz, r5.xyzx, cb0[8].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[8].xyzx)+(r9.xyzx)).xyz;
    // 80: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 81: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 82: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 83: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 84: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 85: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 86: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 87: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 88: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 89: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 90: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 91: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 92: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 93: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 94: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 95: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 96: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 97: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 98: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 99: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 100: mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // 101: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 102: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 103: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 104: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 106: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 107: ret
    return output;
}

// source.character.static-map-native-1401.v1 / source program e1cb414df669fe4388b0d863864db470
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1401(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1401(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: add r0.x, -v2.w, cb0[4].y
    r0.x = ((-(v2.wwww))+(source[4].yyyy)).x;
    // 2: add r0.x, r0.x, cb0[4].x
    r0.x = ((r0.xxxx)+(source[4].xxxx)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // 24: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: dp3 r1.w, r3.xyzx, r3.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 29: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 30: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 31: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 32: mul r5.xyz, cb0[2].xyzx, cb0[3].yyyy
    r5.xyz = ((source[2].xyzx)*(source[3].yyyy)).xyz;
    // 33: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 34: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 35: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 36: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 37: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 38: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 39: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 41: mul r5.yzw, r5.yyyy, cb0[6].xxyz
    r5.yzw = ((r5.yyyy)*(source[6].xxyz)).yzw;
    // 42: mad r5.xyz, r5.xxxx, cb0[5].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[5].xyzx)+(r5.yzwy)).xyz;
    // 43: mul r5.xyz, r5.xyzx, cb0[7].wwww
    r5.xyz = ((r5.xyzx)*(source[7].wwww)).xyz;
    // 44: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 45: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 46: mad r5.xyz, r4.xyzx, cb0[7].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[7].xyzx)+(r5.xyzx)).xyz;
    // 47: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 48: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 49: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 50: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 51: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 54: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 55: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 56: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 57: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 58: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 59: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 60: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 61: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 62: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 63: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 64: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 65: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 66: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 67: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 68: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 69: ret
    return output;
}

// source.character.static-map-native-1402.v1 / source program 4b3a261b5c8ed1438620dbbcdac20e3e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1402(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[8]=1.f;
    source[9]=1.f;
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
    // 9: mul r0.w, r2.y, cb0[3].z
    r0.w = ((r2.yyyy)*(source[3].zzzz)).w;
    // 10: mad r0.xyz, r0.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r0.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 11: mad r1.xyz, cb0[3].wwww, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r1.xyz = ((source[3].wwww)*(source[1].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
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
    // 21: mul r3.xy, r2.yzyy, cb0[3].xxxx
    r3.xy = ((r2.yzyy)*(source[3].xxxx)).xy;
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
    // 35: mul r3.xyz, cb0[2].xyzx, cb0[4].xxxx
    r3.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 36: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.yyyy
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.yyyy))).xyz;
    // 37: mul r5.xyz, r2.xxxx, r3.xyzx
    r5.xyz = ((r2.xxxx)*(r3.xyzx)).xyz;
    // 38: mad r1.yzw, r5.xxyz, r4.xxyz, r1.yyyy
    r1.yzw = ((r5.xxyz)*(r4.xxyz)+(r1.yyyy)).yzw;
    // 39: mul r1.yzw, r1.yyzw, cb0[6].xxyz
    r1.yzw = ((r1.yyzw)*(source[6].xxyz)).yzw;
    // 40: mad r4.xyz, r2.xxxx, r3.xyzx, -r1.xxxx
    r4.xyz = ((r2.xxxx)*(r3.xyzx)+(-(r1.xxxx))).xyz;
    // 41: mad r4.xyz, r5.xyzx, r4.xyzx, r1.xxxx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)+(r1.xxxx)).xyz;
    // 42: mad r1.xyz, r4.xyzx, cb0[5].xyzx, r1.yzwy
    r1.xyz = ((r4.xyzx)*(source[5].xyzx)+(r1.yzwy)).xyz;
    // 43: mul r1.xyz, r1.xyzx, cb0[7].wwww
    r1.xyz = ((r1.xyzx)*(source[7].wwww)).xyz;
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
    // 51: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 52: mul r5.xyz, r5.xyzx, cb0[9].xyzx
    r5.xyz = ((r5.xyzx)*(source[9].xyzx)).xyz;
    // 53: dp3 r0.w, r5.xyzx, r3.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 54: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t3.xyzw, s3
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 55: mul r3.xyz, r3.xyzx, cb0[8].xyzx
    r3.xyz = ((r3.xyzx)*(source[8].xyzx)).xyz;
    // 56: mul r6.xyz, r0.wwww, r3.xyzx
    r6.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 57: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].xyzx)).xyz;
    // 59: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 60: div r1.xyz, r6.xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)/(r1.xyzx)).xyz;
    // 61: mad r4.xyz, r0.xyzx, r6.xyzx, r4.xyzx
    r4.xyz = ((r0.xyzx)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 63: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 64: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 65: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 66: dp3 r1.w, r2.yzwy, r1.xyzx
    r1.w = (dot((r2.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 67: mul r6.xyz, r1.wwww, r2.yzwy
    r6.xyz = ((r1.wwww)*(r2.yzwy)).xyz;
    // 68: mad r1.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 69: dp2_sat r6.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 70: dp3_sat r6.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 71: dp3_sat r6.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 72: mul r1.xyz, r6.xyzx, r6.xyzx
    r1.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 73: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 74: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 75: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 76: dp3 r1.x, r5.xyzx, r1.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 77: mad r1.yzw, r3.xxyz, r1.xxxx, r4.xxyz
    r1.yzw = ((r3.xxyz)*(r1.xxxx)+(r4.xxyz)).yzw;
    // 78: mul r3.xyz, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.xxxx)*(r3.xyzx)).xyz;
    // 79: dp3 o4.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 80: add r3.xyz, r1.yzwy, cb0[0].xyzx
    r3.xyz = ((r1.yzwy)+(source[0].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, cb0[7].xyzx, r3.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)+(r3.xyzx)).xyz;
    // 82: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 83: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 84: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 85: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 86: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 87: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 88: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 89: mul r3.xyz, r1.xxxx, v0.xyzx
    r3.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 90: mul r4.xyz, r0.zxyz, r3.yzxy
    r4.xyz = ((r0.zxyz)*(r3.yzxy)).xyz;
    // 91: mad r4.xyz, r0.yzxy, r3.zxyz, -r4.xyzx
    r4.xyz = ((r0.yzxy)*(r3.zxyz)+(-(r4.xyzx))).xyz;
    // 92: dp3 r0.z, r0.xyzx, r2.yzwy
    r0.z = (dot((r0.xyzx).xyz,(r2.yzwy).xyz).xxxx).z;
    // 93: dp3 r0.x, r3.xyzx, r2.yzwy
    r0.x = (dot((r3.xyzx).xyz,(r2.yzwy).xyz).xxxx).x;
    // 94: mul r3.xyz, r4.xyzx, v1.wwww
    r3.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 95: dp3 r0.y, r3.xyzx, r2.yzwy
    r0.y = (dot((r3.xyzx).xyz,(r2.yzwy).xyz).xxxx).y;
    // 96: dp3 r1.x, r0.xyzx, r0.xyzx
    r1.x = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 97: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 98: mul r0.xyz, r0.xyzx, r1.xxxx
    r0.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 99: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 100: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 101: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 102: ge r2.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 103: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 104: mad r2.xy, -|r0.yxyy|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.yxyy)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 105: movc r0.xy, r1.xxxx, r2.xyxx, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r2.xyxx) : (r0.xyxx)).xy;
    // 106: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 108: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: mul o4.z, r0.w, r1.y
    output.targets[4].z = ((r0.wwww)*(r1.yyyy)).z;
    // 110: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 111: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 113: ret
    return output;
}

// source.character.static-map-native-1402.v1 / source program e63037d563a708418bc9ee84f27980e5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1402(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1402(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
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

// source.character.static-map-native-1403.v1 / source program b743d80328550249bd5f49dab27e51cd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1403(SOURCE_CHARACTER_NATIVE_INPUT input)
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

// source.character.static-map-native-1403.v1 / source program fff4f25164212f47a9c7d39bbaf4148b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1403(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1403(input);
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

// source.character.static-map-native-1404.v1 / source program 22cf319234fd824f8a988fc5016cbca6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1404(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[8]=1.f;
    source[9]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 5: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 6: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 7: mul r1.xy, r1.xyxx, cb0[3].xxxx
    r1.xy = ((r1.xyxx)*(source[3].xxxx)).xy;
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
    // 16: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mul r2.xyz, r0.wwww, r1.xyzx
    r2.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 21: mad r0.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 22: dp2_sat r2.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 23: dp3_sat r2.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 24: dp3_sat r2.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 25: log r0.xyz, r2.xyzx
    r0.xyz = (log2(r2.xyzx)).xyz;
    // 26: add r0.w, cb0[4].y, l(1.000000)
    r0.w = ((source[4].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 28: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 29: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t4.xyzw, s3
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 30: mul r2.xyz, r2.xyzx, cb0[9].xyzx
    r2.xyz = ((r2.xyzx)*(source[9].xyzx)).xyz;
    // 31: dp3 r0.x, r2.xyzx, r0.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 32: dp2_sat r3.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 33: dp3_sat r3.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 34: dp3_sat r3.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 35: mul r0.yzw, r3.xxyz, r3.xxyz
    r0.yzw = ((r3.xxyz)*(r3.xxyz)).yzw;
    // 36: dp3 r0.y, r2.xyzx, r0.yzwy
    r0.y = (dot((r2.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 37: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t3.xyzw, s3
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[8].xyzx
    r2.xyz = ((r2.xyzx)*(source[8].xyzx)).xyz;
    // 39: mul r3.xyz, r0.yyyy, r2.xyzx
    r3.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 40: dp3 r0.z, v6.xyzx, v6.xyzx
    r0.z = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).z;
    // 41: rsq r0.z, r0.z
    r0.z = (rsqrt(r0.zzzz)).z;
    // 42: mul r4.xyz, r0.zzzz, v6.xyzx
    r4.xyz = ((r0.zzzz)*(v6.xyzx)).xyz;
    // 43: dp3 r0.z, r4.xyzx, r1.xyzx
    r0.z = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 44: mad r0.zw, r0.zzzz, l(0.000000, 0.000000, 0.500000, -0.500000), l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r0.zzzz)*(float4(0.000000,0.000000,0.500000,-0.500000))+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 45: mul r0.zw, r0.zzzw, r0.zzzw
    r0.zw = ((r0.zzzw)*(r0.zzzw)).zw;
    // 46: mul r4.xyz, r0.wwww, cb0[6].xyzx
    r4.xyz = ((r0.wwww)*(source[6].xyzx)).xyz;
    // 47: mad r4.xyz, r0.zzzz, cb0[5].xyzx, r4.xyzx
    r4.xyz = ((r0.zzzz)*(source[5].xyzx)+(r4.xyzx)).xyz;
    // 48: mul r4.xyz, r4.xyzx, cb0[7].wwww
    r4.xyz = ((r4.xyzx)*(source[7].wwww)).xyz;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 50: dp3 r0.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 51: add r6.xyz, -r5.xyzx, r0.zzzz
    r6.xyz = ((-(r5.xyzx))+(r0.zzzz)).xyz;
    // 52: mad r5.xyz, cb0[3].zzzz, r6.xyzx, r5.xyzx
    r5.xyz = ((source[3].zzzz)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 53: mul r6.xyz, cb0[1].xyzx, cb0[3].wwww
    r6.xyz = ((source[1].xyzx)*(source[3].wwww)).xyz;
    // 54: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 55: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 56: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 57: mad r0.yzw, r2.xxyz, r0.yyyy, r4.xxyz
    r0.yzw = ((r2.xxyz)*(r0.yyyy)+(r4.xxyz)).yzw;
    // 58: add r0.yzw, r0.yyzw, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 59: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 60: mad r3.xyz, r5.xyzx, r3.xyzx, r6.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 61: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 62: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 63: mul r6.xyz, cb0[2].xyzx, cb0[4].xxxx
    r6.xyz = ((source[2].xyzx)*(source[4].xxxx)).xyz;
    // 64: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 65: mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 66: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 67: mad r3.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 68: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 69: dp3 o4.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 70: add r0.xzw, r3.xxyz, cb0[0].xxyz
    r0.xzw = ((r3.xxyz)+(source[0].xxyz)).xzw;
    // 71: mad o0.xyz, r5.xyzx, cb0[7].xyzx, r0.xzwx
    output.targets[0].xyz = ((r5.xyzx)*(source[7].xyzx)+(r0.xzwx)).xyz;
    // 72: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 73: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 74: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 75: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 76: mul r0.xzw, r0.xxxx, v1.xxyz
    r0.xzw = ((r0.xxxx)*(v1.xxyz)).xzw;
    // 77: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 78: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 79: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 80: mul r4.xyz, r0.wxzw, r2.yzxy
    r4.xyz = ((r0.wxzw)*(r2.yzxy)).xyz;
    // 81: mad r4.xyz, r0.zwxz, r2.zxyz, -r4.xyzx
    r4.xyz = ((r0.zwxz)*(r2.zxyz)+(-(r4.xyzx))).xyz;
    // 82: dp3 r5.z, r0.xzwx, r1.xyzx
    r5.z = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 83: dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 84: mul r0.xzw, r4.xxyz, v1.wwww
    r0.xzw = ((r4.xxyz)*(v1.wwww)).xzw;
    // 85: dp3 r5.y, r0.xzwx, r1.xyzx
    r5.y = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 86: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 87: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 88: mul r0.xzw, r0.xxxx, r5.xxyz
    r0.xzw = ((r0.xxxx)*(r5.xxyz)).xzw;
    // 89: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 90: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 91: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 92: ge r1.yz, r0.xxzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxzx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 93: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 94: mad r1.yz, -|r0.zzxz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzxz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 95: movc r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.xxzx)).xz;
    // 96: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 97: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 98: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 99: mul o4.z, r0.y, r3.x
    output.targets[4].z = ((r0.yyyy)*(r3.xxxx)).z;
    // 100: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 101: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 102: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 103: ret
    return output;
}

// source.character.static-map-native-1404.v1 / source program 4ce41b63ec42d047bf43d253145941c3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1404(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1404(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[3];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[2].xxxx
    r0.xy = ((r0.xyxx)*(source[2].xxxx)).xy;
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
    // 15: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 16: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 18: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 19: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 20: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 22: mul r1.yzw, r1.yyyy, cb0[4].xxyz
    r1.yzw = ((r1.yyyy)*(source[4].xxyz)).yzw;
    // 23: mad r1.xyz, r1.xxxx, cb0[3].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[3].xyzx)+(r1.yzwy)).xyz;
    // 24: mul r1.xyz, r1.xyzx, cb0[5].wwww
    r1.xyz = ((r1.xyzx)*(source[5].wwww)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 26: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r3.xyz, -r2.xyzx, r0.wwww
    r3.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 28: mad r2.xyz, cb0[2].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[2].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 29: mul r3.xyz, cb0[1].xyzx, cb0[2].wwww
    r3.xyz = ((source[1].xyzx)*(source[2].wwww)).xyz;
    // 30: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 31: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 32: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 33: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 34: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 35: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 36: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 37: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 38: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 39: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 40: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 41: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 42: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 43: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 44: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 45: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 46: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 47: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 48: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 49: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 50: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 51: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 52: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 53: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 54: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 55: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 56: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 57: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 58: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 59: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 60: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 61: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 62: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 63: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 64: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 65: ret
    return output;
}

// source.character.static-map-native-1405.v1 / source program 700489a099e5dc49960940cce336ce4b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1405(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[0]=float4(1.f,0.f,0.f,1.f);
    source[11]=1.f;
    source[12]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: mul r0.x, cb0[7].x, l(0.017453)
    r0.x = ((source[7].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).x;
    // 2: sincos null, r0.x, r0.x
    r0.x = (cos(r0.xxxx)).x;
    // 3: max r0.x, r0.x, l(-0.999990)
    r0.x = (max(r0.xxxx,float4(-0.999990,-0.999990,-0.999990,-0.999990))).x;
    // 4: min r0.x, r0.x, l(0.999990)
    r0.x = (min(r0.xxxx,float4(0.999990,0.999990,0.999990,0.999990))).x;
    // 5: mad r0.x, -r0.x, l(0.500000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 6: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, l(0.693147)
    r0.x = ((r0.xxxx)*(float4(0.693147,0.693147,0.693147,0.693147))).x;
    // 8: div r0.x, l(-0.301030), r0.x
    r0.x = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.xxxx)).x;
    // 9: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 10: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 11: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 12: mad r0.y, -r0.y, l(0.499000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 13: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 14: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 15: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 16: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r0.y, r0.x, cb0[7].z
    r0.y = ((r0.xxxx)*(source[7].zzzz)).y;
    // 18: mov r1.x, v4.y
    r1.x = (v4.yyyy).x;
    // 19: mov r1.y, cb0[5].z
    r1.y = (source[5].zzzz).y;
    // 20: add r0.zw, -r1.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r1.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 21: ge r1.x, v4.y, cb0[5].z
    r1.x = (asfloat((uint4)((v4.yyyy)>=(source[5].zzzz)) * 0xffffffffu)).x;
    // 22: movc r0.z, r1.x, r0.z, v4.y
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (v4.yyyy)).z;
    // 23: movc r0.w, r1.x, r0.w, cb0[5].z
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (source[5].zzzz)).w;
    // 24: movc_sat r1.x, r1.x, cb0[5].w, cb0[6].x
    r1.x = (saturate((asuint(r1.xxxx) != 0u) ? (source[5].wwww) : (source[6].xxxx))).x;
    // 25: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 26: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 28: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 29: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 30: mul_sat r0.z, r0.z, cb0[6].y
    r0.z = (saturate((r0.zzzz)*(source[6].yyyy))).z;
    // 31: mad r0.w, -cb0[7].z, r0.x, r0.z
    r0.w = ((-(source[7].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 32: mad r0.y, cb0[7].z, r0.w, r0.y
    r0.y = ((source[7].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 33: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 34: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 35: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 36: mad r2.xyz, r2.xyzx, r0.yyyy, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(r0.yyyy)+(-(r1.xyzx))).xyz;
    // 37: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 41: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 42: add r0.xyw, r0.xyxw, -r1.xyxz
    r0.xyw = ((r0.xyxw)+(-(r1.xyxz))).xyw;
    // 43: add r2.x, -r0.z, l(1.000000)
    r2.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 44: mad_sat r0.z, -r2.x, r1.w, r0.z
    r0.z = (saturate((-(r2.xxxx))*(r1.wwww)+(r0.zzzz))).z;
    // 45: mad r0.xyz, r0.zzzz, r0.xywx, r1.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.xyzx)).xyz;
    // 46: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 47: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 48: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 51: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 52: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 53: mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // 54: mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 55: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 56: mul r2.xyz, r0.xyzx, r1.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 57: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t2.xyzw, s1
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 58: mul r3.xyz, r3.xyzx, cb0[12].xyzx
    r3.xyz = ((r3.xyzx)*(source[12].xyzx)).xyz;
    // 59: dp3 r0.w, r3.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 60: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t1.xyzw, s1
    r3.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 61: mul r3.xyz, r3.xyzx, cb0[11].xyzx
    r3.xyz = ((r3.xyzx)*(source[11].xyzx)).xyz;
    // 62: mul r4.xyz, r0.wwww, r3.xyzx
    r4.xyz = ((r0.wwww)*(r3.xyzx)).xyz;
    // 63: mad r1.xyz, r3.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 64: add r1.xyz, r1.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r1.xyz = ((r1.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 65: div r1.xyz, r4.xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)/(r1.xyzx)).xyz;
    // 66: mad r2.xyz, r0.xyzx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 67: dp3 r0.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 68: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 69: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 70: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 71: mad r1.xyz, r0.xyzx, cb0[10].xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(source[10].xyzx)+(r1.xyzx)).xyz;
    // 72: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 73: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 74: mov o0.w, cb0[0].x
    output.targets[0].w = (source[0].xxxx).w;
    // 75: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 76: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 77: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 78: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 79: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 80: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 81: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 82: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 83: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 84: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 85: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 86: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 87: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 88: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 89: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 90: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 91: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 92: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 93: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 94: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 95: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 96: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 97: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 98: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 99: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 100: ret
    return output;
}

// source.character.static-map-native-1405.v1 / source program 73f9d1701f27b4458312ee589079a592
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1405(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1405(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: mul r0.x, cb0[7].x, l(0.017453)
    r0.x = ((source[7].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).x;
    // 2: sincos null, r0.x, r0.x
    r0.x = (cos(r0.xxxx)).x;
    // 3: max r0.x, r0.x, l(-0.999990)
    r0.x = (max(r0.xxxx,float4(-0.999990,-0.999990,-0.999990,-0.999990))).x;
    // 4: min r0.x, r0.x, l(0.999990)
    r0.x = (min(r0.xxxx,float4(0.999990,0.999990,0.999990,0.999990))).x;
    // 5: mad r0.x, -r0.x, l(0.500000), l(0.500000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 6: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 7: mul r0.x, r0.x, l(0.693147)
    r0.x = ((r0.xxxx)*(float4(0.693147,0.693147,0.693147,0.693147))).x;
    // 8: div r0.x, l(-0.301030), r0.x
    r0.x = ((float4(-0.301030,-0.301030,-0.301030,-0.301030))/(r0.xxxx)).x;
    // 9: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 10: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 11: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 12: mad r0.y, -r0.y, l(0.499000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 13: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 14: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 15: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 16: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 17: mul r0.y, r0.x, cb0[7].z
    r0.y = ((r0.xxxx)*(source[7].zzzz)).y;
    // 18: mov r1.x, v4.y
    r1.x = (v4.yyyy).x;
    // 19: mov r1.y, cb0[5].z
    r1.y = (source[5].zzzz).y;
    // 20: add r0.zw, -r1.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r1.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 21: ge r1.x, v4.y, cb0[5].z
    r1.x = (asfloat((uint4)((v4.yyyy)>=(source[5].zzzz)) * 0xffffffffu)).x;
    // 22: movc r0.z, r1.x, r0.z, v4.y
    r0.z = ((asuint(r1.xxxx) != 0u) ? (r0.zzzz) : (v4.yyyy)).z;
    // 23: movc r0.w, r1.x, r0.w, cb0[5].z
    r0.w = ((asuint(r1.xxxx) != 0u) ? (r0.wwww) : (source[5].zzzz)).w;
    // 24: movc_sat r1.x, r1.x, cb0[5].w, cb0[6].x
    r1.x = (saturate((asuint(r1.xxxx) != 0u) ? (source[5].wwww) : (source[6].xxxx))).x;
    // 25: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 26: add r0.w, -r1.x, l(1.000000)
    r0.w = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 28: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 29: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 30: mul_sat r0.z, r0.z, cb0[6].y
    r0.z = (saturate((r0.zzzz)*(source[6].yyyy))).z;
    // 31: mad r0.w, -cb0[7].z, r0.x, r0.z
    r0.w = ((-(source[7].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 32: mad r0.y, cb0[7].z, r0.w, r0.y
    r0.y = ((source[7].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 33: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 34: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 35: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 36: mad r2.xyz, r2.xyzx, r0.yyyy, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(r0.yyyy)+(-(r1.xyzx))).xyz;
    // 37: mad r0.xyw, r0.xxxx, r2.xyxz, r1.xyxz
    r0.xyw = ((r0.xxxx)*(r2.xyxz)+(r1.xyxz)).xyw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul r2.xyz, r1.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 40: mad r1.xyz, -r1.xyzx, cb0[2].xyzx, r1.xyzx
    r1.xyz = ((-(r1.xyzx))*(source[2].xyzx)+(r1.xyzx)).xyz;
    // 41: mad r1.xyz, r1.wwww, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 42: add r0.xyw, r0.xyxw, -r1.xyxz
    r0.xyw = ((r0.xyxw)+(-(r1.xyxz))).xyw;
    // 43: add r2.x, -r0.z, l(1.000000)
    r2.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 44: mad_sat r0.z, -r2.x, r1.w, r0.z
    r0.z = (saturate((-(r2.xxxx))*(r1.wwww)+(r0.zzzz))).z;
    // 45: mad r0.xyz, r0.zzzz, r0.xywx, r1.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r1.xyzx)).xyz;
    // 46: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 47: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 48: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 49: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 50: mul r0.w, r0.w, v7.z
    r0.w = ((r0.wwww)*(v7.zzzz)).w;
    // 51: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 52: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 53: mul r1.yzw, r1.yyyy, cb0[9].xxyz
    r1.yzw = ((r1.yyyy)*(source[9].xxyz)).yzw;
    // 54: mad r1.xyz, r1.xxxx, cb0[8].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[8].xyzx)+(r1.yzwy)).xyz;
    // 55: mul r1.xyz, r1.xyzx, cb0[10].wwww
    r1.xyz = ((r1.xyzx)*(source[10].wwww)).xyz;
    // 56: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 57: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 58: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mad r1.xyz, r0.xyzx, cb0[10].xyzx, r2.xyzx
    r1.xyz = ((r0.xyzx)*(source[10].xyzx)+(r2.xyzx)).xyz;
    // 60: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 61: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 62: mov o0.w, cb0[0].x
    output.targets[0].w = (source[0].xxxx).w;
    // 63: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 64: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 65: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 66: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 67: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 68: mul r1.xyz, r0.wwww, v0.zxyz
    r1.xyz = ((r0.wwww)*(v0.zxyz)).xyz;
    // 69: mul r0.y, r0.y, r1.y
    r0.y = ((r0.yyyy)*(r1.yyyy)).y;
    // 70: mad r0.x, r0.x, r1.z, -r0.y
    r0.x = ((r0.xxxx)*(r1.zzzz)+(-(r0.yyyy))).x;
    // 71: mov r1.z, r0.z
    r1.z = (r0.zzzz).z;
    // 72: mul r1.y, r0.x, v1.w
    r1.y = ((r0.xxxx)*(v1.wwww)).y;
    // 73: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 74: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 75: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 76: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).w;
    // 77: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 78: ge r0.z, l(0.000000), r0.z
    r0.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).z;
    // 79: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 80: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 81: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 82: movc r0.xy, r0.zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.zzzz) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 83: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 84: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 85: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 86: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 87: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 88: ret
    return output;
}

// source.character.static-map-native-1406.v1 / source program cd6e3d25eb7a2e49b8b94fcdc2202868
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1406(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[11]=1.f;
    source[12]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, cb0[7].w
    r0.x = ((-(v2.wwww))+(source[7].wwww)).x;
    // 2: add r0.x, r0.x, cb0[7].z
    r0.x = ((r0.xxxx)+(source[7].zzzz)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 22: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // 27: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: max r1.w, cb0[5].y, l(0.000000)
    r1.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 38: mul_sat r4.w, r4.w, r5.w
    r4.w = (saturate((r4.wwww)*(r5.wwww))).w;
    // 39: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: mul r6.xy, v4.xyxx, cb0[5].zzzz
    r6.xy = ((v4.xyxx)*(source[5].zzzz)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r5.w, r6.w, r6.w
    r5.w = ((r6.wwww)*(r6.wwww)).w;
    // 43: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 44: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 45: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 46: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 47: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 48: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 49: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 50: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 51: add r7.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 52: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 53: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 54: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 55: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 56: dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 57: mul r7.xyz, r2.wwww, r4.xyzx
    r7.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 58: mad r3.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 59: mul r7.xyz, cb0[2].xyzx, cb0[5].wwww
    r7.xyz = ((source[2].xyzx)*(source[5].wwww)).xyz;
    // 60: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 61: mul r9.xyz, cb0[3].xyzx, cb0[6].xxxx
    r9.xyz = ((source[3].xyzx)*(source[6].xxxx)).xyz;
    // 62: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 63: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: mad r9.xyz, -r9.xyzx, r6.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 65: mad r9.xyz, cb0[6].zzzz, r9.xyzx, r10.xyzx
    r9.xyz = ((source[6].zzzz)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 66: mad r5.xyz, -r5.xyzx, r7.xyzx, r9.xyzx
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r9.xyzx)).xyz;
    // 67: mad r5.xyz, r1.wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 68: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mul r8.xyz, cb0[4].xyzx, cb0[6].wwww
    r8.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 71: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 72: mad r6.xyz, cb0[7].xxxx, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[7].xxxx)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 73: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 74: mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 75: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 76: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 77: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 78: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 79: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 80: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 81: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 82: add r1.w, cb0[7].y, l(1.000000)
    r1.w = ((source[7].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 84: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 85: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 86: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 87: mul r8.xyz, r8.xyzx, cb0[11].xyzx
    r8.xyz = ((r8.xyzx)*(source[11].xyzx)).xyz;
    // 88: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 89: mul r9.xyz, r9.xyzx, cb0[12].xyzx
    r9.xyz = ((r9.xyzx)*(source[12].xyzx)).xyz;
    // 90: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 91: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 92: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 93: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 94: mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 95: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 96: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 97: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 98: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 99: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 100: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 101: mul r9.yzw, r9.yyyy, cb0[9].xxyz
    r9.yzw = ((r9.yyyy)*(source[9].xxyz)).yzw;
    // 102: mad r9.xyz, r9.xxxx, cb0[8].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[8].xyzx)+(r9.yzwy)).xyz;
    // 103: mul r9.xyz, r9.xyzx, cb0[10].wwww
    r9.xyz = ((r9.xyzx)*(source[10].wwww)).xyz;
    // 104: mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 105: mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 106: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 107: mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 108: add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // 109: mad r9.xyz, r5.xyzx, cb0[10].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[10].xyzx)+(r9.xyzx)).xyz;
    // 110: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 111: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 112: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 113: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 114: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 115: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 116: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 117: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 118: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 119: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 120: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 121: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 122: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 123: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 124: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 125: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 126: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 127: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 128: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 129: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 130: mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // 131: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 132: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 133: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 134: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 135: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 136: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 137: ret
    return output;
}

// source.character.static-map-native-1406.v1 / source program 9e98f003bb97c546a7cdd727a839b4a4
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1406(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1406(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: add r0.x, -v2.w, cb0[6].w
    r0.x = ((-(v2.wwww))+(source[6].wwww)).x;
    // 2: add r0.x, r0.x, cb0[6].z
    r0.x = ((r0.xxxx)+(source[6].zzzz)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 5: lt r0.y, r0.x, l(0.003000)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).y;
    // 6: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 7: dp3 r0.y, v1.xyzx, v1.xyzx
    r0.y = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).y;
    // 8: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 9: mul r0.yzw, r0.yyyy, v1.xxyz
    r0.yzw = ((r0.yyyy)*(v1.xxyz)).yzw;
    // 10: dp3 r1.x, v0.xyzx, v0.xyzx
    r1.x = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).x;
    // 11: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 12: mul r1.xyz, r1.xxxx, v0.xyzx
    r1.xyz = ((r1.xxxx)*(v0.xyzx)).xyz;
    // 13: mul r2.xyz, r0.wyzw, r1.yzxy
    r2.xyz = ((r0.wyzw)*(r1.yzxy)).xyz;
    // 14: mad r2.xyz, r0.zwyz, r1.zxyz, -r2.xyzx
    r2.xyz = ((r0.zwyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 15: mul r2.xyz, r2.xyzx, v1.wwww
    r2.xyz = ((r2.xyzx)*(v1.wwww)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 17: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r3.xy, r3.xyxx, cb0[4].xxxx
    r3.xy = ((r3.xyxx)*(source[4].xxxx)).xy;
    // 24: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: max r1.w, cb0[4].y, l(0.000000)
    r1.w = (max(source[4].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 29: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 30: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 32: add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 34: mul r5.x, r3.z, r3.z
    r5.x = ((r3.zzzz)*(r3.zzzz)).x;
    // 35: mul_sat r4.w, r4.w, r5.x
    r4.w = (saturate((r4.wwww)*(r5.xxxx))).w;
    // 36: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: mul r5.xy, v4.xyxx, cb0[4].zzzz
    r5.xy = ((v4.xyxx)*(source[4].zzzz)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 40: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 41: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 42: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 43: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 44: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 45: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 46: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 47: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 48: add r6.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 49: mad r3.xyz, r2.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 50: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 51: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 52: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 53: mul r6.xyz, cb0[2].xyzx, cb0[4].wwww
    r6.xyz = ((source[2].xyzx)*(source[4].wwww)).xyz;
    // 54: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 55: mul r8.xyz, cb0[3].xyzx, cb0[5].xxxx
    r8.xyz = ((source[3].xyzx)*(source[5].xxxx)).xyz;
    // 56: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 57: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 58: mad r5.xyz, -r8.xyzx, r5.xyzx, r2.wwww
    r5.xyz = ((-(r8.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 59: mad r5.xyz, cb0[5].zzzz, r5.xyzx, r9.xyzx
    r5.xyz = ((source[5].zzzz)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 60: mad r4.xyz, -r4.xyzx, r6.xyzx, r5.xyzx
    r4.xyz = ((-(r4.xyzx))*(r6.xyzx)+(r5.xyzx)).xyz;
    // 61: mad r4.xyz, r1.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 62: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 63: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 64: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 65: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 66: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 67: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 68: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 69: mul r5.yzw, r5.yyyy, cb0[8].xxyz
    r5.yzw = ((r5.yyyy)*(source[8].xxyz)).yzw;
    // 70: mad r5.xyz, r5.xxxx, cb0[7].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[7].xyzx)+(r5.yzwy)).xyz;
    // 71: mul r5.xyz, r5.xyzx, cb0[9].wwww
    r5.xyz = ((r5.xyzx)*(source[9].wwww)).xyz;
    // 72: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 73: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 74: mad r5.xyz, r4.xyzx, cb0[9].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[9].xyzx)+(r5.xyzx)).xyz;
    // 75: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 76: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 77: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 78: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 79: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 80: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 81: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 82: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 83: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 84: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 85: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 86: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 87: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 88: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 89: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 90: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 91: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 92: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 93: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 94: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 95: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 96: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 97: ret
    return output;
}

// source.character.static-map-native-1407.v1 / source program 5c671d30bc73594c8d33cd808f0ea4d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1407(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    source[13]=1.f;
    source[14]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 3: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 4: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
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
    // 10: add r0.z, -cb0[8].x, l(1.000000)
    r0.z = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
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
    // 24: mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // 25: mad r1.x, r0.y, r1.y, -r1.x
    r1.x = ((r0.yyyy)*(r1.yyyy)+(-(r1.xxxx))).x;
    // 26: mul r0.z, r1.x, v1.w
    r0.z = ((r1.xxxx)*(v1.wwww)).z;
    // 27: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 28: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 29: mul r1.xyw, r1.xxxx, v6.xyxz
    r1.xyw = ((r1.xxxx)*(v6.xyxz)).xyw;
    // 30: mad r1.xyw, r1.wwww, l(0.000000, 0.000000, 0.000000, 2.000000), -r1.xyxw
    r1.xyw = ((r1.wwww)*(float4(0.000000,0.000000,0.000000,2.000000))+(-(r1.xyxw))).xyw;
    // 31: add r2.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 32: add r3.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 33: dp2 r4.x, cb0[2].xyxx, r3.xyxx
    r4.x = (dot((source[2].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 34: dp2 r4.y, cb0[3].xyxx, r3.xyxx
    r4.y = (dot((source[3].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 35: add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 36: mul r3.xy, r3.xyxx, cb0[4].xyxx
    r3.xy = ((r3.xyxx)*(source[4].xyxx)).xy;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 38: dp3 r2.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 39: add r4.xyz, -r3.xyzx, r2.wwww
    r4.xyz = ((-(r3.xyzx))+(r2.wwww)).xyz;
    // 40: mad r3.xyz, cb0[7].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 41: mul r4.xyz, r3.xyzx, cb0[7].yyyy
    r4.xyz = ((r3.xyzx)*(source[7].yyyy)).xyz;
    // 42: mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // 43: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 44: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 45: mul r3.xyz, r3.xyzx, cb0[7].zzzz
    r3.xyz = ((r3.xyzx)*(source[7].zzzz)).xyz;
    // 46: mul r3.xyz, r3.xyzx, cb0[6].xyzx
    r3.xyz = ((r3.xyzx)*(source[6].xyzx)).xyz;
    // 47: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 48: dp2_sat r4.x, r1.ywyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r4.x = (saturate(dot((r1.ywyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 49: dp3_sat r4.y, r1.xywx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r4.y = (saturate(dot((r1.xywx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 50: dp3_sat r4.z, r1.xywx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r4.z = (saturate(dot((r1.xywx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 51: add r1.x, cb0[7].w, l(1.000000)
    r1.x = ((source[7].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: log r4.xyz, r4.xyzx
    r4.xyz = (log2(r4.xyzx)).xyz;
    // 53: mul r1.xyw, r1.xxxx, r4.xyxz
    r1.xyw = ((r1.xxxx)*(r4.xyxz)).xyw;
    // 54: exp r1.xyw, r1.xyxw
    r1.xyw = (exp2(r1.xyxw)).xyw;
    // 55: sample_indexable(texture2d)(float,float,float,float) r4.xyz, v3.zwzz, t3.xyzw, s3
    r4.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 56: mul r4.xyz, r4.xyzx, cb0[13].xyzx
    r4.xyz = ((r4.xyzx)*(source[13].xyzx)).xyz;
    // 57: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t4.xyzw, s3
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 58: mul r5.xyz, r5.xyzx, cb0[14].xyzx
    r5.xyz = ((r5.xyzx)*(source[14].xyzx)).xyz;
    // 59: dp3 r2.w, r5.xyzx, l(0.333333, 0.333333, 0.333333, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.333333,0.333333,0.333333,0.000000)).xyz).xxxx).w;
    // 60: mul r6.xyz, r2.wwww, r4.xyzx
    r6.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 61: mul r7.xyz, r3.xyzx, r4.xyzx
    r7.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 62: dp3 r1.x, r5.xyzx, r1.xywx
    r1.x = (dot((r5.xyzx).xyz,(r1.xywx).xyz).xxxx).x;
    // 63: mul r5.xyz, r1.xxxx, r7.xyzx
    r5.xyz = ((r1.xxxx)*(r7.xyzx)).xyz;
    // 64: dp3 r1.y, v7.xyzx, v7.xyzx
    r1.y = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).y;
    // 65: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 66: mul r1.y, r1.y, v7.z
    r1.y = ((r1.yyyy)*(v7.zzzz)).y;
    // 67: mad r1.yw, r1.yyyy, l(0.000000, 0.500000, 0.000000, -0.500000), l(0.000000, 0.500000, 0.000000, 0.500000)
    r1.yw = ((r1.yyyy)*(float4(0.000000,0.500000,0.000000,-0.500000))+(float4(0.000000,0.500000,0.000000,0.500000))).yw;
    // 68: mul r1.yw, r1.yyyw, r1.yyyw
    r1.yw = ((r1.yyyw)*(r1.yyyw)).yw;
    // 69: mul r8.xyz, r1.wwww, cb0[11].xyzx
    r8.xyz = ((r1.wwww)*(source[11].xyzx)).xyz;
    // 70: mad r8.xyz, r1.yyyy, cb0[10].xyzx, r8.xyzx
    r8.xyz = ((r1.yyyy)*(source[10].xyzx)+(r8.xyzx)).xyz;
    // 71: mul r8.xyz, r8.xyzx, cb0[12].wwww
    r8.xyz = ((r8.xyzx)*(source[12].wwww)).xyz;
    // 72: mul r9.xyz, r2.xyzx, r8.xyzx
    r9.xyz = ((r2.xyzx)*(r8.xyzx)).xyz;
    // 73: mad r9.xyz, r2.xyzx, r6.xyzx, r9.xyzx
    r9.xyz = ((r2.xyzx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 74: mad r4.xyz, r4.xyzx, r2.wwww, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r2.wwww)+(r8.xyzx)).xyz;
    // 75: mad r1.xyw, r7.xyxz, r1.xxxx, r9.xyxz
    r1.xyw = ((r7.xyxz)*(r1.xxxx)+(r9.xyxz)).xyw;
    // 76: add r7.xyz, r1.xywx, cb0[1].xyzx
    r7.xyz = ((r1.xywx)+(source[1].xyzx)).xyz;
    // 77: mad r7.xyz, r2.xyzx, cb0[12].xyzx, r7.xyzx
    r7.xyz = ((r2.xyzx)*(source[12].xyzx)+(r7.xyzx)).xyz;
    // 78: mad o0.xyz, r7.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r7.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 79: mov r0.y, r1.z
    r0.y = (r1.zzzz).y;
    // 80: dp3 r1.z, r0.yzwy, r0.yzwy
    r1.z = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).z;
    // 81: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 82: mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // 83: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).z;
    // 84: div r0.yz, r0.yyzy, r1.zzzz
    r0.yz = ((r0.yyzy)/(r1.zzzz)).yz;
    // 85: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 86: ge r7.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r7.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 87: movc r7.xy, r7.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r7.xy = ((asuint(r7.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 88: mad r7.xy, -|r0.zyzz|, r7.xyxx, r7.xyxx
    r7.xy = ((-(abs(r0.zyzz)))*(r7.xyxx)+(r7.xyxx)).xy;
    // 89: movc r0.yz, r0.wwww, r7.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r7.xxyx) : (r0.yyzy)).yz;
    // 90: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 91: mul_sat o3.w, cb0[7].w, l(0.002000)
    output.targets[3].w = (saturate((source[7].wwww)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 92: dp3 o4.x, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 93: dp3 o4.y, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 94: add r0.yzw, r4.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r4.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 95: div r0.yzw, r6.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)/(r0.yyzw)).yzw;
    // 96: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 97: mul o4.z, r0.y, r1.x
    output.targets[4].z = ((r0.yyyy)*(r1.xxxx)).z;
    // 98: ftou r0.y, cb0[9].z
    r0.y = (asfloat((uint4)(source[9].zzzz))).y;
    // 99: bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // 100: utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // 101: mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 102: mul_sat r0.yzw, r3.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r3.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // 103: sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // 104: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 105: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 106: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 107: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 108: ret
    return output;
}

// source.character.static-map-native-1407.v1 / source program d25977be2346854b9b5bf041418ff66d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1407(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1407(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[8];
    source[8]=g_SourceCharacterBaseConstants[9];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r0.x = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 2: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 3: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 4: sample_l_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s0, l(0.000000)
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
    // 10: add r0.z, -cb0[8].x, l(1.000000)
    r0.z = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
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
    // 24: mul r1.x, r0.z, r1.x
    r1.x = ((r0.zzzz)*(r1.xxxx)).x;
    // 25: mad r1.x, r0.y, r1.y, -r1.x
    r1.x = ((r0.yyyy)*(r1.yyyy)+(-(r1.xxxx))).x;
    // 26: mul r0.z, r1.x, v1.w
    r0.z = ((r1.xxxx)*(v1.wwww)).z;
    // 27: add r1.xyw, -cb0[1].xyxz, l(1.000000, 1.000000, 0.000000, 1.000000)
    r1.xyw = ((-(source[1].xyxz))+(float4(1.000000,1.000000,0.000000,1.000000))).xyw;
    // 28: add r2.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r2.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 29: dp2 r3.x, cb0[2].xyxx, r2.xyxx
    r3.x = (dot((source[2].xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 30: dp2 r3.y, cb0[3].xyxx, r2.xyxx
    r3.y = (dot((source[3].xyxx).xy,(r2.xyxx).xy).xxxx).y;
    // 31: add r2.xy, r3.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r3.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 32: mul r2.xy, r2.xyxx, cb0[4].xyxx
    r2.xy = ((r2.xyxx)*(source[4].xyxx)).xy;
    // 33: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 34: dp3 r2.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: add r3.xyz, -r2.xyzx, r2.wwww
    r3.xyz = ((-(r2.xyzx))+(r2.wwww)).xyz;
    // 36: mad r2.xyz, cb0[7].xxxx, r3.xyzx, r2.xyzx
    r2.xyz = ((source[7].xxxx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 37: mul r3.xyz, r2.xyzx, cb0[7].yyyy
    r3.xyz = ((r2.xyzx)*(source[7].yyyy)).xyz;
    // 38: mul r3.xyz, r3.xyzx, cb0[5].xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // 39: mul r1.xyw, r1.xyxw, r3.xyxz
    r1.xyw = ((r1.xyxw)*(r3.xyxz)).xyw;
    // 40: mad r1.xyw, r1.xyxw, cb2[3].wwww, cb2[3].xyxz
    r1.xyw = ((r1.xyxw)*(passValues[3].wwww)+(passValues[3].xyxz)).xyw;
    // 41: mul r2.xyz, r2.xyzx, cb0[7].zzzz
    r2.xyz = ((r2.xyzx)*(source[7].zzzz)).xyz;
    // 42: mul r2.xyz, r2.xyzx, cb0[6].xyzx
    r2.xyz = ((r2.xyzx)*(source[6].xyzx)).xyz;
    // 43: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 44: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 45: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 46: mul r2.w, r2.w, v7.z
    r2.w = ((r2.wwww)*(v7.zzzz)).w;
    // 47: mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 48: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 49: mul r3.yzw, r3.yyyy, cb0[11].xxyz
    r3.yzw = ((r3.yyyy)*(source[11].xxyz)).yzw;
    // 50: mad r3.xyz, r3.xxxx, cb0[10].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(r3.yzwy)).xyz;
    // 51: mul r3.xyz, r3.xyzx, cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(source[12].wwww)).xyz;
    // 52: mul r4.xyz, r1.xywx, r3.xyzx
    r4.xyz = ((r1.xywx)*(r3.xyzx)).xyz;
    // 53: mad r3.xyz, r3.xyzx, r1.xywx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(r1.xywx)+(source[1].xyzx)).xyz;
    // 54: mad r3.xyz, r1.xywx, cb0[12].xyzx, r3.xyzx
    r3.xyz = ((r1.xywx)*(source[12].xyzx)+(r3.xyzx)).xyz;
    // 55: mad o0.xyz, r3.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r3.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 56: mov r0.y, r1.z
    r0.y = (r1.zzzz).y;
    // 57: dp3 r1.z, r0.yzwy, r0.yzwy
    r1.z = (dot((r0.yzwy).xyz,(r0.yzwy).xyz).xxxx).z;
    // 58: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 59: mul r0.yzw, r0.yyzw, r1.zzzz
    r0.yzw = ((r0.yyzw)*(r1.zzzz)).yzw;
    // 60: dp3 r1.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).z;
    // 61: div r0.yz, r0.yyzy, r1.zzzz
    r0.yz = ((r0.yyzy)/(r1.zzzz)).yz;
    // 62: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 63: ge r3.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r3.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 64: movc r3.xy, r3.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r3.xy = ((asuint(r3.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 65: mad r3.xy, -|r0.zyzz|, r3.xyxx, r3.xyxx
    r3.xy = ((-(abs(r0.zyzz)))*(r3.xyxx)+(r3.xyxx)).xy;
    // 66: movc r0.yz, r0.wwww, r3.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r3.xxyx) : (r0.yyzy)).yz;
    // 67: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 68: mul_sat o3.w, cb0[7].w, l(0.002000)
    output.targets[3].w = (saturate((source[7].wwww)*(float4(0.002000,0.002000,0.002000,0.002000)))).w;
    // 69: dp3 o4.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 70: ftou r0.y, cb0[9].z
    r0.y = (asfloat((uint4)(source[9].zzzz))).y;
    // 71: bfi r0.y, l(5), l(0), r0.y, l(32)
    r0.y = (SourceCharacterBitInsert(uint4(5u,5u,5u,5u),uint4(0u,0u,0u,0u),asuint(r0.yyyy),uint4(32u,32u,32u,32u))).y;
    // 72: utof r0.y, r0.y
    r0.y = ((float4)(asuint(r0.yyyy))).y;
    // 73: mul o5.w, r0.y, l(0.003922)
    output.targets[5].w = ((r0.yyyy)*(float4(0.003922,0.003922,0.003922,0.003922))).w;
    // 74: mul_sat r0.yzw, r2.xxyz, l(0.000000, 0.100000, 0.100000, 0.100000)
    r0.yzw = (saturate((r2.xxyz)*(float4(0.000000,0.100000,0.100000,0.100000)))).yzw;
    // 75: sqrt o5.xyz, r0.yzwy
    output.targets[5].xyz = (sqrt(r0.yzwy)).xyz;
    // 76: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 77: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 78: mov o3.xyz, r1.xywx
    output.targets[3].xyz = (r1.xywx).xyz;
    // 79: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 80: ret
    return output;
}

// source.character.static-map-native-1408.v1 / source program c232cafc31ed0449b63072a6f092e36d
