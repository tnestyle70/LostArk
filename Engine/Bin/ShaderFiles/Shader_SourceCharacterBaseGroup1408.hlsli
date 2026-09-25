SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1408(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterBaseConstants[0];
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[7]=1.f;
    source[8]=1.f;
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
    // 26: add r0.w, cb0[3].w, l(1.000000)
    r0.w = ((source[3].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 27: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 28: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 29: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t3.xyzw, s2
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 30: mul r2.xyz, r2.xyzx, cb0[8].xyzx
    r2.xyz = ((r2.xyzx)*(source[8].xyzx)).xyz;
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
    // 37: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t2.xyzw, s2
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 38: mul r2.xyz, r2.xyzx, cb0[7].xyzx
    r2.xyz = ((r2.xyzx)*(source[7].xyzx)).xyz;
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
    // 46: mul r4.xyz, r0.wwww, cb0[5].xyzx
    r4.xyz = ((r0.wwww)*(source[5].xyzx)).xyz;
    // 47: mad r4.xyz, r0.zzzz, cb0[4].xyzx, r4.xyzx
    r4.xyz = ((r0.zzzz)*(source[4].xyzx)+(r4.xyzx)).xyz;
    // 48: mul r4.xyz, r4.xyzx, cb0[6].wwww
    r4.xyz = ((r4.xyzx)*(source[6].wwww)).xyz;
    // 49: mul r5.xyz, cb0[1].xyzx, cb0[3].yyyy
    r5.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 51: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 52: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 53: mul r7.xyz, r4.xyzx, r5.xyzx
    r7.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 54: mad r0.yzw, r2.xxyz, r0.yyyy, r4.xxyz
    r0.yzw = ((r2.xxyz)*(r0.yyyy)+(r4.xxyz)).yzw;
    // 55: add r0.yzw, r0.yyzw, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r0.yyzw)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 56: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 57: mad r3.xyz, r5.xyzx, r3.xyzx, r7.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r7.xyzx)).xyz;
    // 58: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 59: mul r4.xyz, cb0[2].xyzx, cb0[3].zzzz
    r4.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 60: mul r4.xyz, r4.xyzx, r6.xyzx
    r4.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 61: mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 62: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 63: mad r3.xyz, r2.xyzx, r0.xxxx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r0.xxxx)+(r3.xyzx)).xyz;
    // 64: mul r0.xzw, r0.xxxx, r2.xxyz
    r0.xzw = ((r0.xxxx)*(r2.xxyz)).xzw;
    // 65: dp3 o4.x, r0.xzwx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.xzwx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 66: add r0.xzw, r3.xxyz, cb0[0].xxyz
    r0.xzw = ((r3.xxyz)+(source[0].xxyz)).xzw;
    // 67: mad o0.xyz, r5.xyzx, cb0[6].xyzx, r0.xzwx
    output.targets[0].xyz = ((r5.xyzx)*(source[6].xyzx)+(r0.xzwx)).xyz;
    // 68: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 69: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 70: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 71: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 72: mul r0.xzw, r0.xxxx, v1.xxyz
    r0.xzw = ((r0.xxxx)*(v1.xxyz)).xzw;
    // 73: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 74: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 75: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 76: mul r4.xyz, r0.wxzw, r2.yzxy
    r4.xyz = ((r0.wxzw)*(r2.yzxy)).xyz;
    // 77: mad r4.xyz, r0.zwxz, r2.zxyz, -r4.xyzx
    r4.xyz = ((r0.zwxz)*(r2.zxyz)+(-(r4.xyzx))).xyz;
    // 78: dp3 r5.z, r0.xzwx, r1.xyzx
    r5.z = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 79: dp3 r5.x, r2.xyzx, r1.xyzx
    r5.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 80: mul r0.xzw, r4.xxyz, v1.wwww
    r0.xzw = ((r4.xxyz)*(v1.wwww)).xzw;
    // 81: dp3 r5.y, r0.xzwx, r1.xyzx
    r5.y = (dot((r0.xzwx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 82: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 83: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 84: mul r0.xzw, r0.xxxx, r5.xxyz
    r0.xzw = ((r0.xxxx)*(r5.xxyz)).xzw;
    // 85: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 86: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xzwx|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xzwx)).xyz).xxxx).w;
    // 87: div r0.xz, r0.xxzx, r0.wwww
    r0.xz = ((r0.xxzx)/(r0.wwww)).xz;
    // 88: ge r1.yz, r0.xxzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxzx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 89: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 90: mad r1.yz, -|r0.zzxz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzxz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 91: movc r0.xz, r1.xxxx, r1.yyzy, r0.xxzx
    r0.xz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.xxzx)).xz;
    // 92: mad o2.xy, r0.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xzxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 93: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 94: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 95: mul o4.z, r0.y, r3.x
    output.targets[4].z = ((r0.yyyy)*(r3.xxxx)).z;
    // 96: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 97: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 98: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 99: ret
    return output;
}

// source.character.static-map-native-1408.v1 / source program b29b1d29a2878e47ace0bf77236d5e8f
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1408(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1408(input);
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
    // 26: mul r3.xyz, cb0[1].xyzx, cb0[2].yyyy
    r3.xyz = ((source[1].xyzx)*(source[2].yyyy)).xyz;
    // 27: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 28: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 29: mad r3.xyz, r1.xyzx, r2.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.xyzx)*(r2.xyzx)+(source[0].xyzx)).xyz;
    // 30: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 31: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 32: mad o0.xyz, r2.xyzx, cb0[5].xyzx, r3.xyzx
    output.targets[0].xyz = ((r2.xyzx)*(source[5].xyzx)+(r3.xyzx)).xyz;
    // 33: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 34: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 35: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 36: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 37: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 38: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 39: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 40: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 41: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 42: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 43: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 44: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 45: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 46: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 47: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 48: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 49: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 50: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 51: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 52: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 53: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 54: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 55: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 56: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 57: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 58: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 59: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 60: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 61: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 62: ret
    return output;
}

// source.character.static-map-native-1409.v1 / source program 1bb9b491a3f28e489e0746501f310072
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1409(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 50: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: add r0.yzw, -r6.xxyz, r0.yyyy
    r0.yzw = ((-(r6.xxyz))+(r0.yyyy)).yzw;
    // 52: mad r0.yzw, cb0[7].zzzz, r0.yyzw, r6.xxyz
    r0.yzw = ((source[7].zzzz)*(r0.yyzw)+(r6.xxyz)).yzw;
    // 53: mul r7.xyz, cb0[4].xyzx, cb0[7].wwww
    r7.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 54: mul r7.xyz, r5.xyzx, r7.xyzx
    r7.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 55: mad r0.yzw, cb0[8].xxxx, r0.yyzw, -r7.xxyz
    r0.yzw = ((source[8].xxxx)*(r0.yyzw)+(-(r7.xxyz))).yzw;
    // 56: mad r0.yzw, r0.xxxx, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r7.xxyz)).yzw;
    // 57: mad r0.yzw, r0.yyzw, cb2[4].wwww, cb2[4].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[4].wwww)+(passValues[4].xxyz)).yzw;
    // 58: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t3.xyzw, s3
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 59: mul r7.xyz, r7.xyzx, cb0[12].xyzx
    r7.xyz = ((r7.xyzx)*(source[12].xyzx)).xyz;
    // 60: mul r0.yzw, r0.yyzw, r7.xxyz
    r0.yzw = ((r0.yyzw)*(r7.xxyz)).yzw;
    // 61: mul r8.xyz, cb0[3].xyzx, cb0[7].xxxx
    r8.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // 62: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 63: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 64: mad r6.xyz, -r8.xyzx, r6.xyzx, r1.wwww
    r6.xyz = ((-(r8.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 65: mad r6.xyz, cb0[7].zzzz, r6.xyzx, r9.xyzx
    r6.xyz = ((source[7].zzzz)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 66: mul r8.xyz, cb0[2].xyzx, cb0[6].wwww
    r8.xyz = ((source[2].xyzx)*(source[6].wwww)).xyz;
    // 67: mad r6.xyz, -r5.xyzx, r8.xyzx, r6.xyzx
    r6.xyz = ((-(r5.xyzx))*(r8.xyzx)+(r6.xyzx)).xyz;
    // 68: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 69: mad r5.xyz, r0.xxxx, r6.xyzx, r5.xyzx
    r5.xyz = ((r0.xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 70: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 71: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 72: add r6.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 73: mad r1.xyz, r0.xxxx, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 74: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 75: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 76: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 77: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 78: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 79: mul r6.xyz, r0.xxxx, v6.xyzx
    r6.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 80: dp3 r0.x, r6.xyzx, r1.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 81: mad r6.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 83: mul r6.yzw, r6.yyyy, cb0[10].xxyz
    r6.yzw = ((r6.yyyy)*(source[10].xxyz)).yzw;
    // 84: mad r6.xyz, r6.xxxx, cb0[9].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[9].xyzx)+(r6.yzwy)).xyz;
    // 85: mul r6.xyz, r6.xyzx, cb0[11].wwww
    r6.xyz = ((r6.xyzx)*(source[11].wwww)).xyz;
    // 86: mul r8.xyz, r5.xyzx, r6.xyzx
    r8.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 87: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 88: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 89: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 90: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 91: sample_indexable(texture2d)(float,float,float,float) r10.xyz, v3.zwzz, t4.xyzw, s3
    r10.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 92: mul r10.xyz, r10.xyzx, cb0[13].xyzx
    r10.xyz = ((r10.xyzx)*(source[13].xyzx)).xyz;
    // 93: dp3 r0.x, r10.xyzx, r9.xyzx
    r0.x = (dot((r10.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 94: mul r9.xyz, r0.xxxx, r7.xyzx
    r9.xyz = ((r0.xxxx)*(r7.xyzx)).xyz;
    // 95: mad r6.xyz, r7.xyzx, r0.xxxx, r6.xyzx
    r6.xyz = ((r7.xyzx)*(r0.xxxx)+(r6.xyzx)).xyz;
    // 96: add r6.xyz, r6.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 97: div r6.xyz, r9.xyzx, r6.xyzx
    r6.xyz = ((r9.xyzx)/(r6.xyzx)).xyz;
    // 98: mad r7.xyz, r5.xyzx, r9.xyzx, r8.xyzx
    r7.xyz = ((r5.xyzx)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 99: dp3 r0.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 100: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 101: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 102: mul r6.xyz, r1.wwww, v5.xyzx
    r6.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 103: dp3 r1.w, r1.xyzx, r6.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 104: mul r8.xyz, r1.wwww, r1.xyzx
    r8.xyz = ((r1.wwww)*(r1.xyzx)).xyz;
    // 105: mad r6.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r6.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 106: dp2_sat r8.x, r6.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r6.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 107: dp3_sat r8.y, r6.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r6.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 108: dp3_sat r8.z, r6.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r6.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 109: log r6.xyz, r8.xyzx
    r6.xyz = (log2(r8.xyzx)).xyz;
    // 110: add r1.w, cb0[8].y, l(1.000000)
    r1.w = ((source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mul r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)*(r1.wwww)).xyz;
    // 112: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 113: dp3 r1.w, r10.xyzx, r6.xyzx
    r1.w = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 114: mad r6.xyz, r0.yzwy, r1.wwww, r7.xyzx
    r6.xyz = ((r0.yzwy)*(r1.wwww)+(r7.xyzx)).xyz;
    // 115: mul r0.yzw, r0.yyzw, r1.wwww
    r0.yzw = ((r0.yyzw)*(r1.wwww)).yzw;
    // 116: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 117: add r0.yzw, r6.xxyz, cb0[1].xxyz
    r0.yzw = ((r6.xxyz)+(source[1].xxyz)).yzw;
    // 118: mad o0.xyz, r5.xyzx, cb0[11].xyzx, r0.yzwy
    output.targets[0].xyz = ((r5.xyzx)*(source[11].xyzx)+(r0.yzwy)).xyz;
    // 119: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 120: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 121: dp3 r3.x, r3.xyzx, r1.xyzx
    r3.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 122: dp3 r3.y, r4.xyzx, r1.xyzx
    r3.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 123: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 124: dp3 r0.y, r3.xyzx, r3.xyzx
    r0.y = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 125: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 126: mul r0.yzw, r0.yyyy, r3.xxyz
    r0.yzw = ((r0.yyyy)*(r3.xxyz)).yzw;
    // 127: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 128: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 129: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 130: ge r1.yz, r0.yyzy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.yyzy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 131: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 132: mad r1.yz, -|r0.zzyz|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.zzyz)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 133: movc r0.yz, r1.xxxx, r1.yyzy, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r1.yyzy) : (r0.yyzy)).yz;
    // 134: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 135: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 136: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 137: mul o4.z, r0.x, r6.x
    output.targets[4].z = ((r0.xxxx)*(r6.xxxx)).z;
    // 138: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 139: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 140: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 141: ret
    return output;
}

// source.character.static-map-native-1409.v1 / source program 00349f1aa614464c902ca807b8c383d5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1409(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1409(input);
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
    // 50: mul r0.yzw, cb0[3].xxyz, cb0[6].xxxx
    r0.yzw = ((source[3].xxyz)*(source[6].xxxx)).yzw;
    // 51: mul r7.xyz, r6.xyzx, r0.yzwy
    r7.xyz = ((r6.xyzx)*(r0.yzwy)).xyz;
    // 52: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 53: mad r0.yzw, -r0.yyzw, r6.xxyz, r1.wwww
    r0.yzw = ((-(r0.yyzw))*(r6.xxyz)+(r1.wwww)).yzw;
    // 54: mad r0.yzw, cb0[6].zzzz, r0.yyzw, r7.xxyz
    r0.yzw = ((source[6].zzzz)*(r0.yyzw)+(r7.xxyz)).yzw;
    // 55: mul r6.xyz, cb0[2].xyzx, cb0[5].wwww
    r6.xyz = ((source[2].xyzx)*(source[5].wwww)).xyz;
    // 56: mad r0.yzw, -r5.xxyz, r6.xxyz, r0.yyzw
    r0.yzw = ((-(r5.xxyz))*(r6.xxyz)+(r0.yyzw)).yzw;
    // 57: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 58: mad r0.yzw, r0.xxxx, r0.yyzw, r5.xxyz
    r0.yzw = ((r0.xxxx)*(r0.yyzw)+(r5.xxyz)).yzw;
    // 59: mul r0.x, r0.x, l(0.650000)
    r0.x = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 60: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 61: add r5.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r1.xyz, r0.xxxx, r5.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r5.xyzx)+(r1.xyzx)).xyz;
    // 63: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 64: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 65: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 66: dp3 r0.x, v6.xyzx, v6.xyzx
    r0.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 67: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 68: mul r5.xyz, r0.xxxx, v6.xyzx
    r5.xyz = ((r0.xxxx)*(v6.xyzx)).xyz;
    // 69: dp3 r0.x, r5.xyzx, r1.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 70: mad r5.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 71: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 72: mul r5.yzw, r5.yyyy, cb0[8].xxyz
    r5.yzw = ((r5.yyyy)*(source[8].xxyz)).yzw;
    // 73: mad r5.xyz, r5.xxxx, cb0[7].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[7].xyzx)+(r5.yzwy)).xyz;
    // 74: mul r5.xyz, r5.xyzx, cb0[9].wwww
    r5.xyz = ((r5.xyzx)*(source[9].wwww)).xyz;
    // 75: mad r6.xyz, r5.xyzx, r0.yzwy, cb0[1].xyzx
    r6.xyz = ((r5.xyzx)*(r0.yzwy)+(source[1].xyzx)).xyz;
    // 76: mul r5.xyz, r0.yzwy, r5.xyzx
    r5.xyz = ((r0.yzwy)*(r5.xyzx)).xyz;
    // 77: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 78: mad o0.xyz, r0.yzwy, cb0[9].xyzx, r6.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[9].xyzx)+(r6.xyzx)).xyz;
    // 79: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 80: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 81: dp3 r0.x, r3.xyzx, r1.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 82: dp3 r0.y, r4.xyzx, r1.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 83: dp3 r0.z, r2.xyzx, r1.xyzx
    r0.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 84: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 85: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 86: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 87: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 88: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 89: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 90: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 91: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 92: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 93: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 94: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 95: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 96: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 97: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 98: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 99: ret
    return output;
}

// source.character.static-map-native-1410.v1 / source program 77b779aab9687245af05729263b8b866
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1410(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[13]=1.f;
    source[14]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: mul r0.xyzw, v4.xyxy, cb0[6].xxzz
    r0.xyzw = ((v4.xyxy)*(source[6].xxzz)).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: dp2 r1.x, r0.xyxx, r0.xyxx
    r1.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 5: mul r0.xy, r0.xyxx, cb0[6].yyyy
    r0.xy = ((r0.xyxx)*(source[6].yyyy)).xy;
    // 6: mul r2.xy, r0.xyxx, v2.wwww
    r2.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 7: add r0.x, -r1.x, l(1.000000)
    r0.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 12: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 13: div r1.xyz, r2.xyzx, r0.xxxx
    r1.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 17: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 18: mul r3.xy, r0.xyxx, cb0[6].wwww
    r3.xy = ((r0.xyxx)*(source[6].wwww)).xy;
    // 19: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 22: add r3.z, r0.x, l(0.000010)
    r3.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: add r0.xyz, -r1.xyzx, r3.xyzx
    r0.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 24: mul r0.w, r1.z, r1.z
    r0.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 25: mul r3.xyzw, v4.xyxy, cb0[8].xxyy
    r3.xyzw = ((v4.xyxy)*(source[8].xxyy)).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.zwzz, t3.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 27: add r3.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r3.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 28: mad r1.w, r3.z, l(2.000000), r1.w
    r1.w = ((r3.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(r1.wwww)).w;
    // 29: mad r2.w, r3.w, l(2.000000), r2.w
    r2.w = ((r3.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(r2.wwww)).w;
    // 30: add r3.z, -r1.w, l(1.000000)
    r3.z = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: add r1.w, r1.w, -cb0[7].y
    r1.w = ((r1.wwww)+(-(source[7].yyyy))).w;
    // 32: mul_sat r1.w, r1.w, cb0[7].w
    r1.w = (saturate((r1.wwww)*(source[7].wwww))).w;
    // 33: mul r3.z, r3.z, cb0[8].z
    r3.z = ((r3.zzzz)*(source[8].zzzz)).z;
    // 34: mad r3.z, r3.z, l(0.050000), l(-0.025000)
    r3.z = ((r3.zzzz)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).z;
    // 35: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 36: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 37: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 38: mad r3.xy, r3.zzzz, r4.xyxx, r3.xyxx
    r3.xy = ((r3.zzzz)*(r4.xyxx)+(r3.xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 41: mul r3.xyz, r3.xyzx, cb0[9].xxxx
    r3.xyz = ((r3.xyzx)*(source[9].xxxx)).xyz;
    // 42: mul_sat r0.w, r0.w, r5.w
    r0.w = (saturate((r0.wwww)*(r5.wwww))).w;
    // 43: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 44: add r3.w, r2.w, -cb0[7].y
    r3.w = ((r2.wwww)+(-(source[7].yyyy))).w;
    // 45: mad r2.w, -r3.w, cb0[7].w, r2.w
    r2.w = ((-(r3.wwww))*(source[7].wwww)+(r2.wwww)).w;
    // 46: mul r3.w, r3.w, cb0[7].w
    r3.w = ((r3.wwww)*(source[7].wwww)).w;
    // 47: mad_sat r0.w, r0.w, r2.w, r3.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r3.wwww))).w;
    // 48: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 49: dp3_sat r1.x, r4.xyzx, r0.xyzx
    r1.x = (saturate(dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx)).x;
    // 50: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 52: add r6.xyz, -cb0[1].xyzx, cb0[2].xyzx
    r6.xyz = ((-(source[1].xyzx))+(source[2].xyzx)).xyz;
    // 53: mad r1.xyz, r1.xxxx, r6.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xxxx)*(r6.xyzx)+(source[1].xyzx)).xyz;
    // 54: dp3 r2.w, r0.xyzx, r0.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r0.xyz, r0.xyzx, r2.wwww
    r0.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 57: dp3 r2.w, r0.xyzx, r4.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 58: mul r6.xyz, r0.xyzx, r2.wwww
    r6.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 59: mad r4.xyz, r6.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r4.xyzx
    r4.xyz = ((r6.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r4.xyzx))).xyz;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 61: mad r1.xyz, r6.xyzx, cb0[3].xyzx, r1.xyzx
    r1.xyz = ((r6.xyzx)*(source[3].xyzx)+(r1.xyzx)).xyz;
    // 62: mul r6.xyz, r6.xyzx, cb0[3].xyzx
    r6.xyz = ((r6.xyzx)*(source[3].xyzx)).xyz;
    // 63: mad r1.xyz, r1.xyzx, r6.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 64: mad r6.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), -r5.xyzx
    r6.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(-(r5.xyzx))).xyz;
    // 65: mul r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 66: mul r1.xyz, r1.xyzx, cb0[8].wwww
    r1.xyz = ((r1.xyzx)*(source[8].wwww)).xyz;
    // 67: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 68: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 69: mad r6.xyz, r2.xyzx, cb0[5].xyzx, -r5.xyzx
    r6.xyz = ((r2.xyzx)*(source[5].xyzx)+(-(r5.xyzx))).xyz;
    // 70: mad r5.xyz, r0.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r0.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 71: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 72: mad r2.xyz, cb0[9].yyyy, r2.xyzx, -r3.xyzx
    r2.xyz = ((source[9].yyyy)*(r2.xyzx)+(-(r3.xyzx))).xyz;
    // 73: mad r2.xyz, r0.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r0.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 74: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 75: mad r1.xyz, r0.wwww, r1.xyzx, cb0[0].xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)+(source[0].xyzx)).xyz;
    // 76: mad r2.xyz, r2.xyzx, cb2[4].wwww, cb2[4].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 77: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 78: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 79: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 80: dp3 r0.w, r3.xyzx, r0.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 81: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 82: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 83: mul r3.yzw, r3.yyyy, cb0[11].xxyz
    r3.yzw = ((r3.yyyy)*(source[11].xxyz)).yzw;
    // 84: mad r3.xyz, r3.xxxx, cb0[10].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[10].xyzx)+(r3.yzwy)).xyz;
    // 85: mul r3.xyz, r3.xyzx, cb0[12].wwww
    r3.xyz = ((r3.xyzx)*(source[12].wwww)).xyz;
    // 86: mul r6.xyz, r5.xyzx, r3.xyzx
    r6.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 87: dp2_sat r7.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 88: dp3_sat r7.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 89: dp3_sat r7.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 90: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 91: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t8.xyzw, s7
    r8.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 92: mul r8.xyz, r8.xyzx, cb0[14].xyzx
    r8.xyz = ((r8.xyzx)*(source[14].xyzx)).xyz;
    // 93: dp3 r0.w, r8.xyzx, r7.xyzx
    r0.w = (dot((r8.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 94: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t7.xyzw, s7
    r7.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 95: mul r7.xyz, r7.xyzx, cb0[13].xyzx
    r7.xyz = ((r7.xyzx)*(source[13].xyzx)).xyz;
    // 96: mul r9.xyz, r0.wwww, r7.xyzx
    r9.xyz = ((r0.wwww)*(r7.xyzx)).xyz;
    // 97: mad r3.xyz, r7.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r7.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 98: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 99: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 100: div r3.xyz, r9.xyzx, r3.xyzx
    r3.xyz = ((r9.xyzx)/(r3.xyzx)).xyz;
    // 101: mad r6.xyz, r5.xyzx, r9.xyzx, r6.xyzx
    r6.xyz = ((r5.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 102: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: dp2_sat r3.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 104: dp3_sat r3.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 105: dp3_sat r3.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 106: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 107: add r1.w, cb0[9].z, l(1.000000)
    r1.w = ((source[9].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 108: mul r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = ((r3.xyzx)*(r1.wwww)).xyz;
    // 109: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 110: dp3 r1.w, r8.xyzx, r3.xyzx
    r1.w = (dot((r8.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 111: mad r3.xyz, r2.xyzx, r1.wwww, r6.xyzx
    r3.xyz = ((r2.xyzx)*(r1.wwww)+(r6.xyzx)).xyz;
    // 112: mul r2.xyz, r1.wwww, r2.xyzx
    r2.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 113: dp3 o4.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)+(r3.xyzx)).xyz;
    // 115: mad o0.xyz, r5.xyzx, cb0[12].xyzx, r1.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(source[12].xyzx)+(r1.xyzx)).xyz;
    // 116: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 117: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 118: dp3 r1.x, v1.xyzx, v1.xyzx
    r1.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 119: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 120: mul r1.xyz, r1.xxxx, v1.xyzx
    r1.xyz = ((r1.xxxx)*(v1.xyzx)).xyz;
    // 121: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 122: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 123: mul r2.xyz, r1.wwww, v0.xyzx
    r2.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 124: mul r4.xyz, r1.zxyz, r2.yzxy
    r4.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 125: mad r4.xyz, r1.yzxy, r2.zxyz, -r4.xyzx
    r4.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r4.xyzx))).xyz;
    // 126: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 127: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 128: mul r2.xyz, r4.xyzx, v1.wwww
    r2.xyz = ((r4.xyzx)*(v1.wwww)).xyz;
    // 129: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 130: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 131: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 132: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 133: ge r1.x, l(0.000000), r0.z
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).x;
    // 134: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 135: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 136: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 137: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 138: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 139: movc r0.xy, r1.xxxx, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r1.xxxx) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 140: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 141: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 142: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 144: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 145: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 146: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 147: ret
    return output;
}

// source.character.static-map-native-1410.v1 / source program 2b35e3f274c37c4dad6152a5e2e02631
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1410(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1410(input);
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
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: mul r0.xyzw, v4.xyxy, cb0[6].xxzz
    r0.xyzw = ((v4.xyxy)*(source[6].xxzz)).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: dp2 r1.x, r0.xyxx, r0.xyxx
    r1.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 5: mul r0.xy, r0.xyxx, cb0[6].yyyy
    r0.xy = ((r0.xyxx)*(source[6].yyyy)).xy;
    // 6: mul r2.xy, r0.xyxx, v2.wwww
    r2.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 7: add r0.x, -r1.x, l(1.000000)
    r0.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 8: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 9: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 10: add r2.z, r0.x, l(0.000010)
    r2.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 12: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 13: div r1.xyz, r2.xyzx, r0.xxxx
    r1.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.zwzz, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 17: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 18: mul r3.xy, r0.xyxx, cb0[6].wwww
    r3.xy = ((r0.xyxx)*(source[6].wwww)).xy;
    // 19: add r0.x, -r0.z, l(1.000000)
    r0.x = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 22: add r3.z, r0.x, l(0.000010)
    r3.z = ((r0.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: add r0.xyz, -r1.xyzx, r3.xyzx
    r0.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 24: mul r0.w, r1.z, r1.z
    r0.w = ((r1.zzzz)*(r1.zzzz)).w;
    // 25: mul r3.xyzw, v4.xyxy, cb0[8].xxyy
    r3.xyzw = ((v4.xyxy)*(source[8].xxyy)).xyzw;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r3.zwzz, t3.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 27: add r3.zw, v2.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r3.zw = ((v2.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 28: mad r1.w, r3.z, l(2.000000), r1.w
    r1.w = ((r3.zzzz)*(float4(2.000000,2.000000,2.000000,2.000000))+(r1.wwww)).w;
    // 29: mad r2.w, r3.w, l(2.000000), r2.w
    r2.w = ((r3.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(r2.wwww)).w;
    // 30: add r3.z, -r1.w, l(1.000000)
    r3.z = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 31: add r1.w, r1.w, -cb0[7].y
    r1.w = ((r1.wwww)+(-(source[7].yyyy))).w;
    // 32: mul_sat r1.w, r1.w, cb0[7].w
    r1.w = (saturate((r1.wwww)*(source[7].wwww))).w;
    // 33: mul r3.z, r3.z, cb0[8].z
    r3.z = ((r3.zzzz)*(source[8].zzzz)).z;
    // 34: mad r3.z, r3.z, l(0.050000), l(-0.025000)
    r3.z = ((r3.zzzz)*(float4(0.050000,0.050000,0.050000,0.050000))+(float4(-0.025000,-0.025000,-0.025000,-0.025000))).z;
    // 35: dp3 r3.w, v5.xyzx, v5.xyzx
    r3.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 36: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 37: mul r4.xyz, r3.wwww, v5.xyzx
    r4.xyz = ((r3.wwww)*(v5.xyzx)).xyz;
    // 38: mad r3.xy, r3.zzzz, r4.xyxx, r3.xyxx
    r3.xy = ((r3.zzzz)*(r4.xyxx)+(r3.xyxx)).xy;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t4.xyzw, s4, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: mul_sat r0.w, r0.w, r3.w
    r0.w = (saturate((r0.wwww)*(r3.wwww))).w;
    // 41: mul r3.xyz, r3.xyzx, cb0[4].xyzx
    r3.xyz = ((r3.xyzx)*(source[4].xyzx)).xyz;
    // 42: add r3.w, r2.w, -cb0[7].y
    r3.w = ((r2.wwww)+(-(source[7].yyyy))).w;
    // 43: mad r2.w, -r3.w, cb0[7].w, r2.w
    r2.w = ((-(r3.wwww))*(source[7].wwww)+(r2.wwww)).w;
    // 44: mul r3.w, r3.w, cb0[7].w
    r3.w = ((r3.wwww)*(source[7].wwww)).w;
    // 45: mad_sat r0.w, r0.w, r2.w, r3.w
    r0.w = (saturate((r0.wwww)*(r2.wwww)+(r3.wwww))).w;
    // 46: mad r0.xyz, r0.wwww, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 47: dp3_sat r1.x, r4.xyzx, r0.xyzx
    r1.x = (saturate(dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx)).x;
    // 48: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 50: add r5.xyz, -cb0[1].xyzx, cb0[2].xyzx
    r5.xyz = ((-(source[1].xyzx))+(source[2].xyzx)).xyz;
    // 51: mad r1.xyz, r1.xxxx, r5.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xxxx)*(r5.xyzx)+(source[1].xyzx)).xyz;
    // 52: dp3 r2.w, r0.xyzx, r0.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r0.xyz, r0.xyzx, r2.wwww
    r0.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 55: dp3 r2.w, r0.xyzx, r4.xyzx
    r2.w = (dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 56: mul r4.zw, r0.xxxy, r2.wwww
    r4.zw = ((r0.xxxy)*(r2.wwww)).zw;
    // 57: mad r4.xy, r4.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), -r4.xyxx
    r4.xy = ((r4.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(-(r4.xyxx))).xy;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t5.xyzw, s5, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 59: mad r1.xyz, r4.xyzx, cb0[3].xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(source[3].xyzx)+(r1.xyzx)).xyz;
    // 60: mul r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // 61: mad r1.xyz, r1.xyzx, r4.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 62: mul r4.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r4.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 63: mad r1.xyz, r1.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000), -r3.xyzx
    r1.xyz = ((r1.xyzx)*(float4(0.500000,0.500000,0.500000,0.000000))+(-(r3.xyzx))).xyz;
    // 64: mad r1.xyz, r1.wwww, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 65: mul r3.xyz, r4.xyzx, cb0[8].wwww
    r3.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 66: mul r3.xyz, r1.wwww, r3.xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 67: add r1.w, -r0.w, l(1.000000)
    r1.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 68: mad r3.xyz, r1.wwww, r3.xyzx, cb0[0].xyzx
    r3.xyz = ((r1.wwww)*(r3.xyzx)+(source[0].xyzx)).xyz;
    // 69: mad r2.xyz, r2.xyzx, cb0[5].xyzx, -r1.xyzx
    r2.xyz = ((r2.xyzx)*(source[5].xyzx)+(-(r1.xyzx))).xyz;
    // 70: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 71: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
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
    // 78: mul r2.yzw, r2.yyyy, cb0[10].xxyz
    r2.yzw = ((r2.yyyy)*(source[10].xxyz)).yzw;
    // 79: mad r2.xyz, r2.xxxx, cb0[9].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[9].xyzx)+(r2.yzwy)).xyz;
    // 80: mul r2.xyz, r2.xyzx, cb0[11].wwww
    r2.xyz = ((r2.xyzx)*(source[11].wwww)).xyz;
    // 81: mad r3.xyz, r2.xyzx, r1.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r1.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 83: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 84: mad o0.xyz, r1.xyzx, cb0[11].xyzx, r3.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[11].xyzx)+(r3.xyzx)).xyz;
    // 85: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 86: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 87: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 88: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 89: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 90: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 93: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 94: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 95: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 96: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 97: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 98: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 99: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 100: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 101: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 102: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 103: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 104: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 105: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 106: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 107: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 108: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 109: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 110: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 111: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 113: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 114: ret
    return output;
}

// source.character.static-map-native-1411.v1 / source program 0db943c3c13bd446a47bfe3ddcb987e2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1411(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[13]=1.f;
    source[14]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, cb0[9].w
    r0.x = ((-(v2.wwww))+(source[9].wwww)).x;
    // 2: add r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)+(source[9].zzzz)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[1].x
    r0.x = ((r0.xxxx)*(source[1].xxxx)).x;
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
    // 26: mul r4.xy, r4.xyxx, cb0[6].xxxx
    r4.xy = ((r4.xyxx)*(source[6].xxxx)).xy;
    // 27: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 36: dp3 r5.y, r2.xyzx, r4.xyzx
    r5.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 37: dp3 r5.z, r0.yzwy, r4.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 38: mul r6.xy, cb0[0].xyxx, cb0[6].zzzz
    r6.xy = ((source[0].xyxx)*(source[6].zzzz)).xy;
    // 39: max r6.xy, -r6.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = (max(-(r6.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: min r6.xy, r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = (min(r6.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 41: mov r6.z, l(1.000000)
    r6.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 42: dp3 r3.w, r5.xyzx, r6.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 43: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mad r3.w, r3.w, l(0.500000), cb0[7].y
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].yyyy)).w;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 47: mul_sat r4.w, r4.w, r5.w
    r4.w = (saturate((r4.wwww)*(r5.wwww))).w;
    // 48: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r6.xy, v4.xyxx, cb0[7].zzzz
    r6.xy = ((v4.xyxx)*(source[7].zzzz)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r5.w, r6.w, r6.w
    r5.w = ((r6.wwww)*(r6.wwww)).w;
    // 52: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 53: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 54: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 55: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 56: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 57: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 58: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 59: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 60: add r7.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 62: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 63: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 64: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 65: dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 66: mul r7.xyz, r2.wwww, r4.xyzx
    r7.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 67: mad r3.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 68: mul r7.xyz, cb0[3].xyzx, cb0[7].wwww
    r7.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 69: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 70: mul r9.xyz, cb0[4].xyzx, cb0[8].xxxx
    r9.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 71: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 72: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: mad r9.xyz, -r9.xyzx, r6.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 74: mad r9.xyz, cb0[8].zzzz, r9.xyzx, r10.xyzx
    r9.xyz = ((source[8].zzzz)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 75: mad r7.xyz, -r5.xyzx, r7.xyzx, r9.xyzx
    r7.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r9.xyzx)).xyz;
    // 76: mad r7.xyz, r1.wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 77: mad r7.xyz, r7.xyzx, cb2[3].wwww, cb2[3].xyzx
    r7.xyz = ((r7.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 78: mul r8.xyz, cb0[5].xyzx, cb0[8].wwww
    r8.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 79: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 80: mad r6.xyz, cb0[9].xxxx, r6.xyzx, -r5.xyzx
    r6.xyz = ((source[9].xxxx)*(r6.xyzx)+(-(r5.xyzx))).xyz;
    // 81: mad r5.xyz, r1.wwww, r6.xyzx, r5.xyzx
    r5.xyz = ((r1.wwww)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 82: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 83: dp2_sat r6.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 84: dp3_sat r6.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 85: dp3_sat r6.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 86: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 87: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 88: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 89: mul r3.xyz, r6.xyzx, r6.xyzx
    r3.xyz = ((r6.xyzx)*(r6.xyzx)).xyz;
    // 90: add r1.w, cb0[9].y, l(1.000000)
    r1.w = ((source[9].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 91: log r6.xyz, r8.xyzx
    r6.xyz = (log2(r8.xyzx)).xyz;
    // 92: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 93: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 94: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t3.xyzw, s3
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 95: mul r8.xyz, r8.xyzx, cb0[13].xyzx
    r8.xyz = ((r8.xyzx)*(source[13].xyzx)).xyz;
    // 96: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t4.xyzw, s3
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 97: mul r9.xyz, r9.xyzx, cb0[14].xyzx
    r9.xyz = ((r9.xyzx)*(source[14].xyzx)).xyz;
    // 98: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 99: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 100: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 101: dp3 r2.w, r9.xyzx, r6.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 102: mul r6.xyz, r2.wwww, r5.xyzx
    r6.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 103: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 104: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 105: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 106: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 107: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 108: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 109: mul r9.yzw, r9.yyyy, cb0[11].xxyz
    r9.yzw = ((r9.yyyy)*(source[11].xxyz)).yzw;
    // 110: mad r9.xyz, r9.xxxx, cb0[10].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[10].xyzx)+(r9.yzwy)).xyz;
    // 111: mul r9.xyz, r9.xyzx, cb0[12].wwww
    r9.xyz = ((r9.xyzx)*(source[12].wwww)).xyz;
    // 112: mul r10.xyz, r7.xyzx, r9.xyzx
    r10.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 113: mad r10.xyz, r7.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r7.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 114: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 115: mad r5.xyz, r5.xyzx, r2.wwww, r10.xyzx
    r5.xyz = ((r5.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 116: add r9.xyz, r5.xyzx, cb0[2].xyzx
    r9.xyz = ((r5.xyzx)+(source[2].xyzx)).xyz;
    // 117: mad r9.xyz, r7.xyzx, cb0[12].xyzx, r9.xyzx
    r9.xyz = ((r7.xyzx)*(source[12].xyzx)+(r9.xyzx)).xyz;
    // 118: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 119: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 120: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 121: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 122: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 123: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 124: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 125: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 126: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 127: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 128: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 129: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 130: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 131: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 132: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 133: dp3 o4.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 134: dp3 o4.y, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 135: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 136: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 137: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 138: mul o4.z, r0.y, r5.x
    output.targets[4].z = ((r0.yyyy)*(r5.xxxx)).z;
    // 139: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 140: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 141: mov o3.xyz, r7.xyzx
    output.targets[3].xyz = (r7.xyzx).xyz;
    // 142: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 144: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 145: ret
    return output;
}

// source.character.static-map-native-1411.v1 / source program ed665177a1b5874cb8b53db40ae63cb7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1411(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1411(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: add r0.x, -v2.w, cb0[8].w
    r0.x = ((-(v2.wwww))+(source[8].wwww)).x;
    // 2: add r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)+(source[8].zzzz)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[1].x
    r0.x = ((r0.xxxx)*(source[1].xxxx)).x;
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
    // 23: mul r3.xy, r3.xyxx, cb0[5].xxxx
    r3.xy = ((r3.xyxx)*(source[5].xxxx)).xy;
    // 24: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: max r1.w, cb0[5].y, l(0.000000)
    r1.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 29: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 30: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 32: dp3 r4.x, r1.xyzx, r3.xyzx
    r4.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 33: dp3 r4.y, r2.xyzx, r3.xyzx
    r4.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 34: dp3 r4.z, r0.yzwy, r3.xyzx
    r4.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 35: mul r5.xy, cb0[0].xyxx, cb0[5].zzzz
    r5.xy = ((source[0].xyxx)*(source[5].zzzz)).xy;
    // 36: max r5.xy, -r5.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = (max(-(r5.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: min r5.xy, r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = (min(r5.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 38: mov r5.z, l(1.000000)
    r5.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 39: dp3 r3.w, r4.xyzx, r5.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 40: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mad r3.w, r3.w, l(0.500000), cb0[6].y
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].yyyy)).w;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r5.x, r3.z, r3.z
    r5.x = ((r3.zzzz)*(r3.zzzz)).x;
    // 44: mul_sat r4.w, r4.w, r5.x
    r4.w = (saturate((r4.wwww)*(r5.xxxx))).w;
    // 45: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 46: mul r5.xy, v4.xyxx, cb0[6].zzzz
    r5.xy = ((v4.xyxx)*(source[6].zzzz)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 49: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 50: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 51: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 52: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 53: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 54: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 55: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 56: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 57: add r6.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 58: mad r3.xyz, r2.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 59: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 60: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 61: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 62: mul r6.xyz, cb0[3].xyzx, cb0[6].wwww
    r6.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 63: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 64: mul r8.xyz, cb0[4].xyzx, cb0[7].xxxx
    r8.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 65: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 66: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: mad r5.xyz, -r8.xyzx, r5.xyzx, r2.wwww
    r5.xyz = ((-(r8.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 68: mad r5.xyz, cb0[7].zzzz, r5.xyzx, r9.xyzx
    r5.xyz = ((source[7].zzzz)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 69: mad r4.xyz, -r4.xyzx, r6.xyzx, r5.xyzx
    r4.xyz = ((-(r4.xyzx))*(r6.xyzx)+(r5.xyzx)).xyz;
    // 70: mad r4.xyz, r1.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 71: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 72: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 73: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 74: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 75: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 76: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 78: mul r5.yzw, r5.yyyy, cb0[10].xxyz
    r5.yzw = ((r5.yyyy)*(source[10].xxyz)).yzw;
    // 79: mad r5.xyz, r5.xxxx, cb0[9].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[9].xyzx)+(r5.yzwy)).xyz;
    // 80: mul r5.xyz, r5.xyzx, cb0[11].wwww
    r5.xyz = ((r5.xyzx)*(source[11].wwww)).xyz;
    // 81: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 82: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[2].xyzx)).xyz;
    // 83: mad r5.xyz, r4.xyzx, cb0[11].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 84: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 86: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 87: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 88: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 89: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 90: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 91: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 92: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 93: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 94: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 95: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 96: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 97: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 98: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 100: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 101: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 102: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 103: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 104: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 105: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 106: ret
    return output;
}

// source.character.static-map-native-1412.v1 / source program 0896651844bbca45bf3d134590fd5ebb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1412(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[13]=1.f;
    source[14]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, cb0[9].z
    r0.x = ((-(v2.wwww))+(source[9].zzzz)).x;
    // 2: add r0.x, r0.x, cb0[9].y
    r0.x = ((r0.xxxx)+(source[9].yyyy)).x;
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
    // 26: mul r4.xy, r4.xyxx, cb0[6].xxxx
    r4.xy = ((r4.xyxx)*(source[6].xxxx)).xy;
    // 27: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 33: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 35: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 36: dp3 r5.y, r2.xyzx, r4.xyzx
    r5.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 37: dp3 r5.z, r0.yzwy, r4.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 38: max r6.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r6.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 39: min r6.xyz, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (min(r6.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 40: dp3 r3.w, r5.xyzx, r6.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 41: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: mad r3.w, r3.w, l(0.500000), cb0[7].x
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).w;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 45: mul_sat r4.w, r4.w, r5.w
    r4.w = (saturate((r4.wwww)*(r5.wwww))).w;
    // 46: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: mul r6.xy, v4.xyxx, cb0[7].yyyy
    r6.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 48: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 49: mul r5.w, r6.w, r6.w
    r5.w = ((r6.wwww)*(r6.wwww)).w;
    // 50: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 51: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 52: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 53: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 54: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 55: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 56: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 57: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 58: add r7.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 59: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 60: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 61: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 62: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 63: dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 64: mul r7.xyz, r2.wwww, r4.xyzx
    r7.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 65: mad r3.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 66: mul r7.xyz, cb0[3].xyzx, cb0[7].zzzz
    r7.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 67: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 68: mul r9.xyz, cb0[4].xyzx, cb0[7].wwww
    r9.xyz = ((source[4].xyzx)*(source[7].wwww)).xyz;
    // 69: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 70: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 71: mad r9.xyz, -r9.xyzx, r6.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 72: mad r9.xyz, cb0[8].yyyy, r9.xyzx, r10.xyzx
    r9.xyz = ((source[8].yyyy)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 73: mad r5.xyz, -r5.xyzx, r7.xyzx, r9.xyzx
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r9.xyzx)).xyz;
    // 74: mad r5.xyz, r1.wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 75: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 77: mul r8.xyz, cb0[5].xyzx, cb0[8].zzzz
    r8.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 78: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 79: mad r6.xyz, cb0[8].wwww, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[8].wwww)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 80: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 81: mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 82: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 83: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 84: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 85: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 86: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 87: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 88: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 89: add r1.w, cb0[9].x, l(1.000000)
    r1.w = ((source[9].xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 90: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 91: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 92: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 93: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t4.xyzw, s4
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 94: mul r8.xyz, r8.xyzx, cb0[13].xyzx
    r8.xyz = ((r8.xyzx)*(source[13].xyzx)).xyz;
    // 95: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t5.xyzw, s4
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 96: mul r9.xyz, r9.xyzx, cb0[14].xyzx
    r9.xyz = ((r9.xyzx)*(source[14].xyzx)).xyz;
    // 97: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 98: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 99: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 100: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 101: mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 102: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 103: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 104: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 105: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 106: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 108: mul r9.yzw, r9.yyyy, cb0[11].xxyz
    r9.yzw = ((r9.yyyy)*(source[11].xxyz)).yzw;
    // 109: mad r9.xyz, r9.xxxx, cb0[10].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[10].xyzx)+(r9.yzwy)).xyz;
    // 110: mul r9.xyz, r9.xyzx, cb0[12].wwww
    r9.xyz = ((r9.xyzx)*(source[12].wwww)).xyz;
    // 111: mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 112: mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 113: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 114: mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 115: add r9.xyz, r6.xyzx, cb0[1].xyzx
    r9.xyz = ((r6.xyzx)+(source[1].xyzx)).xyz;
    // 116: mad r9.xyz, r5.xyzx, cb0[12].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[12].xyzx)+(r9.xyzx)).xyz;
    // 117: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 118: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 119: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 120: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 121: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 122: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 123: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 124: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 125: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 126: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 127: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 128: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 129: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 130: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 131: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 132: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 133: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 134: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 135: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 136: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 137: mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // 138: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 139: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 140: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 141: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 142: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 143: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 144: ret
    return output;
}

// source.character.static-map-native-1412.v1 / source program 1059eadbf4378040b541af654cd4b601
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1412(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1412(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[0];
    source[2]=g_SourceCharacterBaseConstants[1];
    source[3]=g_SourceCharacterBaseConstants[2];
    source[4]=g_SourceCharacterBaseConstants[3];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6]=g_SourceCharacterBaseConstants[6];
    source[7]=g_SourceCharacterBaseConstants[7];
    source[8]=g_SourceCharacterBaseConstants[8];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: add r0.x, -v2.w, cb0[8].z
    r0.x = ((-(v2.wwww))+(source[8].zzzz)).x;
    // 2: add r0.x, r0.x, cb0[8].y
    r0.x = ((r0.xxxx)+(source[8].yyyy)).x;
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
    // 23: mul r3.xy, r3.xyxx, cb0[5].xxxx
    r3.xy = ((r3.xyxx)*(source[5].xxxx)).xy;
    // 24: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 28: max r1.w, cb0[5].y, l(0.000000)
    r1.w = (max(source[5].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 29: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 30: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 32: dp3 r4.x, r1.xyzx, r3.xyzx
    r4.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 33: dp3 r4.y, r2.xyzx, r3.xyzx
    r4.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 34: dp3 r4.z, r0.yzwy, r3.xyzx
    r4.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 35: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 36: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: dp3 r3.w, r4.xyzx, r5.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 38: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: mad r3.w, r3.w, l(0.500000), cb0[6].x
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].xxxx)).w;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r5.x, r3.z, r3.z
    r5.x = ((r3.zzzz)*(r3.zzzz)).x;
    // 42: mul_sat r4.w, r4.w, r5.x
    r4.w = (saturate((r4.wwww)*(r5.xxxx))).w;
    // 43: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mul r5.xy, v4.xyxx, cb0[6].yyyy
    r5.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 47: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 48: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 49: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 50: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 51: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 52: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 53: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 54: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 55: add r6.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r3.xyz, r2.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 57: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 58: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 59: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 60: mul r6.xyz, cb0[3].xyzx, cb0[6].zzzz
    r6.xyz = ((source[3].xyzx)*(source[6].zzzz)).xyz;
    // 61: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 62: mul r8.xyz, cb0[4].xyzx, cb0[6].wwww
    r8.xyz = ((source[4].xyzx)*(source[6].wwww)).xyz;
    // 63: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 64: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 65: mad r5.xyz, -r8.xyzx, r5.xyzx, r2.wwww
    r5.xyz = ((-(r8.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 66: mad r5.xyz, cb0[7].yyyy, r5.xyzx, r9.xyzx
    r5.xyz = ((source[7].yyyy)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 67: mad r4.xyz, -r4.xyzx, r6.xyzx, r5.xyzx
    r4.xyz = ((-(r4.xyzx))*(r6.xyzx)+(r5.xyzx)).xyz;
    // 68: mad r4.xyz, r1.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 69: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 70: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 71: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 72: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 73: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 74: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 75: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 76: mul r5.yzw, r5.yyyy, cb0[10].xxyz
    r5.yzw = ((r5.yyyy)*(source[10].xxyz)).yzw;
    // 77: mad r5.xyz, r5.xxxx, cb0[9].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[9].xyzx)+(r5.yzwy)).xyz;
    // 78: mul r5.xyz, r5.xyzx, cb0[11].wwww
    r5.xyz = ((r5.xyzx)*(source[11].wwww)).xyz;
    // 79: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 80: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad r5.xyz, r4.xyzx, cb0[11].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[11].xyzx)+(r5.xyzx)).xyz;
    // 82: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 83: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 84: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 85: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 86: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 87: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 88: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 89: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 90: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 91: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 92: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 93: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 94: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 95: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 96: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 97: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 98: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 99: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 100: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 101: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 102: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 103: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 104: ret
    return output;
}

// source.character.static-map-native-1413.v1 / source program c47857581c0c3b4c95119c2b904b9ddc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked1413(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[3];
    source[6]=g_SourceCharacterBaseConstants[4];
    source[7]=g_SourceCharacterBaseConstants[5];
    source[8]=g_SourceCharacterBaseConstants[6];
    source[9]=g_SourceCharacterBaseConstants[7];
    source[10]=g_SourceCharacterBaseConstants[8];
    source[14]=1.f;
    source[15]=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
    // 1: add r0.x, -v2.w, l(1.000000)
    r0.x = ((-(v2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 2: add r0.x, r0.x, cb0[10].w
    r0.x = ((r0.xxxx)+(source[10].wwww)).x;
    // 3: add_sat r0.x, r0.x, cb0[10].z
    r0.x = (saturate((r0.xxxx)+(source[10].zzzz))).x;
    // 4: mul r0.x, r0.x, cb0[1].x
    r0.x = ((r0.xxxx)*(source[1].xxxx)).x;
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
    // 26: mul r4.zw, v4.xxxy, cb0[6].yyyy
    r4.zw = ((v4.xxxy)*(source[6].yyyy)).zw;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r4.zw, r4.zwzz, t1.zwxy, s1, l(0.000000)
    r4.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 28: mad r4.zw, r4.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r4.zw = ((r4.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 29: mul r4.zw, r4.zzzw, cb0[6].zzzz
    r4.zw = ((r4.zzzw)*(source[6].zzzz)).zw;
    // 30: mad r4.xy, cb0[6].xxxx, r4.xyxx, r4.zwzz
    r4.xy = ((source[6].xxxx)*(r4.xyxx)+(r4.zwzz)).xy;
    // 31: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 32: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 33: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 34: div r4.xyz, r5.xyzx, r1.wwww
    r4.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 35: max r1.w, cb0[6].w, l(0.000000)
    r1.w = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 36: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 37: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 39: dp3 r5.x, r1.xyzx, r4.xyzx
    r5.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 40: dp3 r5.y, r2.xyzx, r4.xyzx
    r5.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 41: dp3 r5.z, r0.yzwy, r4.xyzx
    r5.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 42: mul r6.xy, cb0[0].xyxx, cb0[7].xxxx
    r6.xy = ((source[0].xyxx)*(source[7].xxxx)).xy;
    // 43: max r6.xy, -r6.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = (max(-(r6.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 44: min r6.xy, r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = (min(r6.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 45: mov r6.z, l(1.000000)
    r6.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 46: dp3 r3.w, r5.xyzx, r6.xyzx
    r3.w = (dot((r5.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 47: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 48: mad r3.w, r3.w, l(0.500000), cb0[7].w
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).w;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r4.w, r4.z, r4.z
    r4.w = ((r4.zzzz)*(r4.zzzz)).w;
    // 51: mul_sat r4.w, r4.w, r5.w
    r4.w = (saturate((r4.wwww)*(r5.wwww))).w;
    // 52: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r6.xy, v4.xyxx, cb0[8].xxxx
    r6.xy = ((v4.xyxx)*(source[8].xxxx)).xy;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r5.w, r6.w, r6.w
    r5.w = ((r6.wwww)*(r6.wwww)).w;
    // 56: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 57: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 58: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 59: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 60: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 61: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 62: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 63: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 64: add r7.xyz, -r4.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r4.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 65: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 66: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 67: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 68: mul r4.xyz, r2.wwww, r4.xyzx
    r4.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 69: dp3 r2.w, r4.xyzx, r3.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 70: mul r7.xyz, r2.wwww, r4.xyzx
    r7.xyz = ((r2.wwww)*(r4.xyzx)).xyz;
    // 71: mad r3.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 72: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 73: add r7.xyz, -r5.xyzx, r2.wwww
    r7.xyz = ((-(r5.xyzx))+(r2.wwww)).xyz;
    // 74: mad r5.xyz, cb0[8].zzzz, r7.xyzx, r5.xyzx
    r5.xyz = ((source[8].zzzz)*(r7.xyzx)+(r5.xyzx)).xyz;
    // 75: mul r7.xyz, cb0[3].xyzx, cb0[8].wwww
    r7.xyz = ((source[3].xyzx)*(source[8].wwww)).xyz;
    // 76: mul r8.xyz, r5.xyzx, r7.xyzx
    r8.xyz = ((r5.xyzx)*(r7.xyzx)).xyz;
    // 77: mul r9.xyz, cb0[4].xyzx, cb0[9].xxxx
    r9.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 78: mul r10.xyz, r6.xyzx, r9.xyzx
    r10.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 79: dp3 r2.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 80: mad r9.xyz, -r9.xyzx, r6.xyzx, r2.wwww
    r9.xyz = ((-(r9.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 81: mad r9.xyz, cb0[9].zzzz, r9.xyzx, r10.xyzx
    r9.xyz = ((source[9].zzzz)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 82: mad r5.xyz, -r5.xyzx, r7.xyzx, r9.xyzx
    r5.xyz = ((-(r5.xyzx))*(r7.xyzx)+(r9.xyzx)).xyz;
    // 83: mad r5.xyz, r1.wwww, r5.xyzx, r8.xyzx
    r5.xyz = ((r1.wwww)*(r5.xyzx)+(r8.xyzx)).xyz;
    // 84: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 85: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 86: mul r8.xyz, cb0[5].xyzx, cb0[9].wwww
    r8.xyz = ((source[5].xyzx)*(source[9].wwww)).xyz;
    // 87: mul r7.xyz, r7.xyzx, r8.xyzx
    r7.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 88: mad r6.xyz, cb0[10].xxxx, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[10].xxxx)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 89: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 90: mad r6.xyz, r6.xyzx, cb2[4].wwww, cb2[4].xyzx
    r6.xyz = ((r6.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 91: dp2_sat r7.x, r4.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r4.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 92: dp3_sat r7.y, r4.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r4.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 93: dp3_sat r7.z, r4.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r4.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 94: dp2_sat r8.x, r3.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r3.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 95: dp3_sat r8.y, r3.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r3.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 96: dp3_sat r8.z, r3.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r3.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 97: mul r3.xyz, r7.xyzx, r7.xyzx
    r3.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 98: add r1.w, cb0[10].y, l(1.000000)
    r1.w = ((source[10].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: log r7.xyz, r8.xyzx
    r7.xyz = (log2(r8.xyzx)).xyz;
    // 100: mul r7.xyz, r1.wwww, r7.xyzx
    r7.xyz = ((r1.wwww)*(r7.xyzx)).xyz;
    // 101: exp r7.xyz, r7.xyzx
    r7.xyz = (exp2(r7.xyzx)).xyz;
    // 102: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t5.xyzw, s5
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 103: mul r8.xyz, r8.xyzx, cb0[14].xyzx
    r8.xyz = ((r8.xyzx)*(source[14].xyzx)).xyz;
    // 104: sample_indexable(texture2d)(float,float,float,float) r9.xyz, v3.zwzz, t6.xyzw, s5
    r9.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 105: mul r9.xyz, r9.xyzx, cb0[15].xyzx
    r9.xyz = ((r9.xyzx)*(source[15].xyzx)).xyz;
    // 106: dp3 r1.w, r9.xyzx, r3.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 107: mul r3.xyz, r1.wwww, r8.xyzx
    r3.xyz = ((r1.wwww)*(r8.xyzx)).xyz;
    // 108: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 109: dp3 r2.w, r9.xyzx, r7.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 110: mul r7.xyz, r2.wwww, r6.xyzx
    r7.xyz = ((r2.wwww)*(r6.xyzx)).xyz;
    // 111: dp3 r3.w, v7.xyzx, v7.xyzx
    r3.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 112: rsq r3.w, r3.w
    r3.w = (rsqrt(r3.wwww)).w;
    // 113: mul r9.xyz, r3.wwww, v7.xyzx
    r9.xyz = ((r3.wwww)*(v7.xyzx)).xyz;
    // 114: dp3 r3.w, r9.xyzx, r4.xyzx
    r3.w = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 115: mad r9.xy, r3.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r9.xy = ((r3.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 116: mul r9.xy, r9.xyxx, r9.xyxx
    r9.xy = ((r9.xyxx)*(r9.xyxx)).xy;
    // 117: mul r9.yzw, r9.yyyy, cb0[12].xxyz
    r9.yzw = ((r9.yyyy)*(source[12].xxyz)).yzw;
    // 118: mad r9.xyz, r9.xxxx, cb0[11].xyzx, r9.yzwy
    r9.xyz = ((r9.xxxx)*(source[11].xyzx)+(r9.yzwy)).xyz;
    // 119: mul r9.xyz, r9.xyzx, cb0[13].wwww
    r9.xyz = ((r9.xyzx)*(source[13].wwww)).xyz;
    // 120: mul r10.xyz, r5.xyzx, r9.xyzx
    r10.xyz = ((r5.xyzx)*(r9.xyzx)).xyz;
    // 121: mad r10.xyz, r5.xyzx, r3.xyzx, r10.xyzx
    r10.xyz = ((r5.xyzx)*(r3.xyzx)+(r10.xyzx)).xyz;
    // 122: mad r8.xyz, r8.xyzx, r1.wwww, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r1.wwww)+(r9.xyzx)).xyz;
    // 123: mad r6.xyz, r6.xyzx, r2.wwww, r10.xyzx
    r6.xyz = ((r6.xyzx)*(r2.wwww)+(r10.xyzx)).xyz;
    // 124: add r9.xyz, r6.xyzx, cb0[2].xyzx
    r9.xyz = ((r6.xyzx)+(source[2].xyzx)).xyz;
    // 125: mad r9.xyz, r5.xyzx, cb0[13].xyzx, r9.xyzx
    r9.xyz = ((r5.xyzx)*(source[13].xyzx)+(r9.xyzx)).xyz;
    // 126: mad o0.xyz, r9.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r9.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 127: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 128: dp3 r1.y, r2.xyzx, r4.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 129: dp3 r1.z, r0.yzwy, r4.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r4.xyzx).xyz).xxxx).z;
    // 130: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 131: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 132: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 133: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 134: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 135: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 136: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 137: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 138: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 139: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 140: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 141: dp3 o4.x, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 142: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 143: add r0.yzw, r8.xxyz, l(0.000000, 0.000010, 0.000010, 0.000010)
    r0.yzw = ((r8.xxyz)+(float4(0.000000,0.000010,0.000010,0.000010))).yzw;
    // 144: div r0.yzw, r3.xxyz, r0.yyzw
    r0.yzw = ((r3.xxyz)/(r0.yyzw)).yzw;
    // 145: dp3 r0.y, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 146: mul o4.z, r0.y, r6.x
    output.targets[4].z = ((r0.yyyy)*(r6.xxxx)).z;
    // 147: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 148: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 149: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 150: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 151: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 152: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 153: ret
    return output;
}

// source.character.static-map-native-1413.v1 / source program e6f4f2798e8cdd4c8bb52672b0cc86b0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase1413(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    if(input.hasBakedLighting)return SourceMapMonsterBaked1413(input);
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[0];
    source[3]=g_SourceCharacterBaseConstants[1];
    source[4]=g_SourceCharacterBaseConstants[2];
    source[5]=g_SourceCharacterBaseConstants[4];
    source[6]=g_SourceCharacterBaseConstants[5];
    source[7]=g_SourceCharacterBaseConstants[6];
    source[8]=g_SourceCharacterBaseConstants[7];
    source[9]=g_SourceCharacterBaseConstants[8];
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: add r0.x, -v2.w, cb0[9].w
    r0.x = ((-(v2.wwww))+(source[9].wwww)).x;
    // 2: add r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)+(source[9].zzzz)).x;
    // 3: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 4: mul r0.x, r0.x, cb0[1].x
    r0.x = ((r0.xxxx)*(source[1].xxxx)).x;
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
    // 23: mul r3.zw, v4.xxxy, cb0[5].yyyy
    r3.zw = ((v4.xxxy)*(source[5].yyyy)).zw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, r3.zwzz, t1.zwxy, s1, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 25: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 26: mul r3.zw, r3.zzzw, cb0[5].zzzz
    r3.zw = ((r3.zzzw)*(source[5].zzzz)).zw;
    // 27: mad r3.xy, cb0[5].xxxx, r3.xyxx, r3.zwzz
    r3.xy = ((source[5].xxxx)*(r3.xyxx)+(r3.zwzz)).xy;
    // 28: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 32: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 36: dp3 r4.x, r1.xyzx, r3.xyzx
    r4.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 37: dp3 r4.y, r2.xyzx, r3.xyzx
    r4.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 38: dp3 r4.z, r0.yzwy, r3.xyzx
    r4.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 39: mul r5.xy, cb0[0].xyxx, cb0[6].xxxx
    r5.xy = ((source[0].xyxx)*(source[6].xxxx)).xy;
    // 40: max r5.xy, -r5.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = (max(-(r5.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: min r5.xy, r5.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r5.xy = (min(r5.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: mov r5.z, l(1.000000)
    r5.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 43: dp3 r3.w, r4.xyzx, r5.xyzx
    r3.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 44: add r3.w, r3.w, l(1.000000)
    r3.w = ((r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: mad r3.w, r3.w, l(0.500000), cb0[6].w
    r3.w = ((r3.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].wwww)).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r5.x, r3.z, r3.z
    r5.x = ((r3.zzzz)*(r3.zzzz)).x;
    // 48: mul_sat r4.w, r4.w, r5.x
    r4.w = (saturate((r4.wwww)*(r5.xxxx))).w;
    // 49: add r4.w, -r4.w, l(1.000000)
    r4.w = ((-(r4.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mul r5.xy, v4.xyxx, cb0[7].xxxx
    r5.xy = ((v4.xyxx)*(source[7].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t3.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mul r5.w, r5.w, r5.w
    r5.w = ((r5.wwww)*(r5.wwww)).w;
    // 53: mul r4.w, r4.w, r5.w
    r4.w = ((r4.wwww)*(r5.wwww)).w;
    // 54: mul r5.w, r1.w, r4.w
    r5.w = ((r1.wwww)*(r4.wwww)).w;
    // 55: mad r3.w, r3.w, r5.w, r3.w
    r3.w = ((r3.wwww)*(r5.wwww)+(r3.wwww)).w;
    // 56: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 57: mul r5.w, r1.w, r2.w
    r5.w = ((r1.wwww)*(r2.wwww)).w;
    // 58: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 59: mad_sat r1.w, r4.w, r1.w, r5.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r5.wwww))).w;
    // 60: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 61: add r6.xyz, -r3.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r3.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r3.xyz, r2.wwww, r6.xyzx, r3.xyzx
    r3.xyz = ((r2.wwww)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 63: dp3 r2.w, r3.xyzx, r3.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 64: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 65: mul r3.xyz, r2.wwww, r3.xyzx
    r3.xyz = ((r2.wwww)*(r3.xyzx)).xyz;
    // 66: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 67: add r6.xyz, -r4.xyzx, r2.wwww
    r6.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 68: mad r4.xyz, cb0[7].zzzz, r6.xyzx, r4.xyzx
    r4.xyz = ((source[7].zzzz)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 69: mul r6.xyz, cb0[3].xyzx, cb0[7].wwww
    r6.xyz = ((source[3].xyzx)*(source[7].wwww)).xyz;
    // 70: mul r7.xyz, r4.xyzx, r6.xyzx
    r7.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 71: mul r8.xyz, cb0[4].xyzx, cb0[8].xxxx
    r8.xyz = ((source[4].xyzx)*(source[8].xxxx)).xyz;
    // 72: mul r9.xyz, r5.xyzx, r8.xyzx
    r9.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 73: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 74: mad r5.xyz, -r8.xyzx, r5.xyzx, r2.wwww
    r5.xyz = ((-(r8.xyzx))*(r5.xyzx)+(r2.wwww)).xyz;
    // 75: mad r5.xyz, cb0[8].zzzz, r5.xyzx, r9.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(r9.xyzx)).xyz;
    // 76: mad r4.xyz, -r4.xyzx, r6.xyzx, r5.xyzx
    r4.xyz = ((-(r4.xyzx))*(r6.xyzx)+(r5.xyzx)).xyz;
    // 77: mad r4.xyz, r1.wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 78: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 79: dp3 r1.w, v7.xyzx, v7.xyzx
    r1.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 80: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 81: mul r5.xyz, r1.wwww, v7.xyzx
    r5.xyz = ((r1.wwww)*(v7.xyzx)).xyz;
    // 82: dp3 r1.w, r5.xyzx, r3.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 83: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 84: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 85: mul r5.yzw, r5.yyyy, cb0[11].xxyz
    r5.yzw = ((r5.yyyy)*(source[11].xxyz)).yzw;
    // 86: mad r5.xyz, r5.xxxx, cb0[10].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[10].xyzx)+(r5.yzwy)).xyz;
    // 87: mul r5.xyz, r5.xyzx, cb0[12].wwww
    r5.xyz = ((r5.xyzx)*(source[12].wwww)).xyz;
    // 88: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 89: mad r5.xyz, r5.xyzx, r4.xyzx, cb0[2].xyzx
    r5.xyz = ((r5.xyzx)*(r4.xyzx)+(source[2].xyzx)).xyz;
    // 90: mad r5.xyz, r4.xyzx, cb0[12].xyzx, r5.xyzx
    r5.xyz = ((r4.xyzx)*(source[12].xyzx)+(r5.xyzx)).xyz;
    // 91: mad o0.xyz, r5.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r5.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 92: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 93: dp3 r1.y, r2.xyzx, r3.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 94: dp3 r1.z, r0.yzwy, r3.xyzx
    r1.z = (dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx).z;
    // 95: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 96: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 97: mul r0.yzw, r0.yyyy, r1.xxyz
    r0.yzw = ((r0.yyyy)*(r1.xxyz)).yzw;
    // 98: dp3 r1.x, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r1.x = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).x;
    // 99: div r0.yz, r0.yyzy, r1.xxxx
    r0.yz = ((r0.yyzy)/(r1.xxxx)).yz;
    // 100: ge r0.w, l(0.000000), r0.w
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).w;
    // 101: ge r1.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 102: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 103: mad r1.xy, -|r0.zyzz|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.zyzz)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 104: movc r0.yz, r0.wwww, r1.xxyx, r0.yyzy
    r0.yz = ((asuint(r0.wwww) != 0u) ? (r1.xxyx) : (r0.yyzy)).yz;
    // 105: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 106: dp3 o4.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 107: mov o0.w, r0.x
    output.targets[0].w = (r0.xxxx).w;
    // 108: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 109: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 110: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 111: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 112: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 113: ret
    return output;
}

// source.character.static-map-native-1500.v1 / source program f0d24fec647e5c4195be3c40a23b612c
