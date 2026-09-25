SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked224(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[10]=1.f; source[11]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[5].xxxx
    r0.xy = ((r0.xyxx)*(source[5].xxxx)).xy;
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
    // 13: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 16: dp3 r0.w, r0.xyzx, r1.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 17: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 18: add r1.w, -|r1.z|, l(1.000000)
    r1.w = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul r0.w, r0.w, r1.w
    r0.w = ((r0.wwww)*(r1.wwww)).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t1.yzwx, s3, l(0.000000)
    r1.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 21: mul r1.w, r1.w, cb0[6].y
    r1.w = ((r1.wwww)*(source[6].yyyy)).w;
    // 22: mad r0.w, r0.w, r1.w, -r1.w
    r0.w = ((r0.wwww)*(r1.wwww)+(-(r1.wwww))).w;
    // 23: mad_sat r0.w, cb0[6].z, r0.w, r1.w
    r0.w = (saturate((source[6].zzzz)*(r0.wwww)+(r1.wwww))).w;
    // 24: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 25: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 28: dp3 r0.w, r0.xyzx, r1.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 29: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 30: mad r1.xyz, r2.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r2.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 31: dp2_sat r2.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 32: dp3_sat r2.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 33: dp3_sat r2.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 34: mul r1.xyz, r2.xyzx, r2.xyzx
    r1.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 35: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 36: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 37: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 38: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t5.xyzw, s4
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 39: mul r2.xyz, r2.xyzx, cb0[11].xyzx
    r2.xyz = ((r2.xyzx)*(source[11].xyzx)).xyz;
    // 40: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 41: dp2_sat r1.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 42: dp3_sat r1.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 43: dp3_sat r1.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 44: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 45: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 46: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t4.wxyz, s4
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 47: mul r1.yzw, r1.yyzw, cb0[10].xxyz
    r1.yzw = ((r1.yyzw)*(source[10].xxyz)).yzw;
    // 48: mul r2.xyz, r1.xxxx, r1.yzwy
    r2.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 49: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r3.xyz, r2.wwww, v7.xyzx
    r3.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 52: dp3 r2.w, r3.xyzx, r0.xyzx
    r2.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 53: mad r3.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 54: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 55: mul r3.yzw, r3.yyyy, cb0[8].xxyz
    r3.yzw = ((r3.yyyy)*(source[8].xxyz)).yzw;
    // 56: mad r3.xyz, r3.xxxx, cb0[7].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[7].xyzx)+(r3.yzwy)).xyz;
    // 57: mul r3.xyz, r3.xyzx, cb0[9].wwww
    r3.xyz = ((r3.xyzx)*(source[9].wwww)).xyz;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 59: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 60: add r5.xyz, -r4.xyzx, r2.wwww
    r5.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 61: mad r4.xyz, cb0[5].wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((source[5].wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 62: mul r5.xyz, cb0[4].xyzx, cb0[6].xxxx
    r5.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // 63: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 64: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 65: mul r5.xyz, r3.xyzx, r4.xyzx
    r5.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 66: mad r3.xyz, r1.yzwy, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.yzwy)*(r1.xxxx)+(r3.xyzx)).xyz;
    // 67: mul r1.xyz, r1.yzwy, cb2[4].xyzx
    r1.xyz = ((r1.yzwy)*(passValues[4].xyzx)).xyz;
    // 68: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 69: div r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)/(r3.xyzx)).xyz;
    // 70: mad r2.xyz, r4.xyzx, r2.xyzx, r5.xyzx
    r2.xyz = ((r4.xyzx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 71: mad r2.xyz, r1.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 72: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 73: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 74: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 75: mul o4.z, r0.w, r2.x
    output.targets[4].z = ((r0.wwww)*(r2.xxxx)).z;
    // 76: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s1, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 78: mul r3.xyz, cb0[3].xyzx, cb0[5].yyyy
    r3.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 79: mad r1.xyz, r1.xyzx, r3.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // 80: add r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)+(r1.xyzx)).xyz;
    // 81: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: mad r1.xyz, r4.xyzx, cb0[9].xyzx, r1.xyzx
    r1.xyz = ((r4.xyzx)*(source[9].xyzx)+(r1.xyzx)).xyz;
    // 83: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 84: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 85: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 86: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 87: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 88: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 91: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 92: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 93: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 94: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 95: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 96: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 97: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 98: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 99: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
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
    // 109: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 110: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 111: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 112: ret
    return output;
}

// source.character.static-map-native-224.v1 / source program dc93e0d9a29a094ca0c513b9f2755ff9
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase224(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5].x=g_SourceCharacterBaseConstants[5].x;
    source[5].y=g_SourceCharacterBaseConstants[5].y;
    source[5].z=g_SourceCharacterBaseConstants[5].z;
    source[5].w=g_SourceCharacterBaseConstants[5].w;
    source[6].x=g_SourceCharacterBaseConstants[6].x;
    source[6].y=g_SourceCharacterBaseConstants[6].y;
    source[6].z=g_SourceCharacterBaseConstants[6].z;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 2: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: dp2 r0.z, r0.xyxx, r0.xyxx
    r0.z = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).z;
    // 4: mul r0.xy, r0.xyxx, cb0[5].xxxx
    r0.xy = ((r0.xyxx)*(source[5].xxxx)).xy;
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
    // 13: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 16: dp3 r0.w, r0.xyzx, r1.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 17: add r1.x, -|r1.z|, l(1.000000)
    r1.x = ((-(abs(r1.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 18: add r0.w, -|r0.w|, l(1.000000)
    r0.w = ((-(abs(r0.wwww)))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 19: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r1.x = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).x;
    // 21: mul r1.x, r1.x, cb0[6].y
    r1.x = ((r1.xxxx)*(source[6].yyyy)).x;
    // 22: mad r0.w, r0.w, r1.x, -r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)+(-(r1.xxxx))).w;
    // 23: mad_sat r0.w, cb0[6].z, r0.w, r1.x
    r0.w = (saturate((source[6].zzzz)*(r0.wwww)+(r1.xxxx))).w;
    // 24: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 25: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 26: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 27: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 28: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 29: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 30: mul r1.xyz, r0.wwww, v7.xyzx
    r1.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 31: dp3 r0.w, r1.xyzx, r0.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 32: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 33: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 34: mul r1.yzw, r1.yyyy, cb0[8].xxyz
    r1.yzw = ((r1.yyyy)*(source[8].xxyz)).yzw;
    // 35: mad r1.xyz, r1.xxxx, cb0[7].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[7].xyzx)+(r1.yzwy)).xyz;
    // 36: mul r1.xyz, r1.xyzx, cb0[9].wwww
    r1.xyz = ((r1.xyzx)*(source[9].wwww)).xyz;
    // 37: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 39: mul r3.xyz, cb0[3].xyzx, cb0[5].yyyy
    r3.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 40: mad r2.xyz, r2.xyzx, r3.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 44: mad r3.xyz, cb0[5].wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((source[5].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 45: mul r4.xyz, cb0[4].xyzx, cb0[6].xxxx
    r4.xyz = ((source[4].xyzx)*(source[6].xxxx)).xyz;
    // 46: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 47: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 48: mad r2.xyz, r1.xyzx, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 49: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 50: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 51: mad r1.xyz, r3.xyzx, cb0[9].xyzx, r2.xyzx
    r1.xyz = ((r3.xyzx)*(source[9].xyzx)+(r2.xyzx)).xyz;
    // 52: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 53: mad o0.xyz, r1.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 54: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 55: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 56: mul r1.xyz, r0.wwww, v1.xyzx
    r1.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 57: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 58: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 59: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 60: mul r3.xyz, r1.zxyz, r2.yzxy
    r3.xyz = ((r1.zxyz)*(r2.yzxy)).xyz;
    // 61: mad r3.xyz, r1.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r1.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 62: dp3 r1.z, r1.xyzx, r0.xyzx
    r1.z = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 63: dp3 r1.x, r2.xyzx, r0.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 64: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 65: dp3 r1.y, r2.xyzx, r0.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 66: dp3 r0.x, r1.xyzx, r1.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 67: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 68: mul r0.xyz, r0.xxxx, r1.xyzx
    r0.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 69: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 70: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 71: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 72: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 73: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 74: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 75: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 76: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 77: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 78: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 79: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 80: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 81: ret
    return output;
}

// source.character.static-map-native-225.v1 / source program 5055c1632d1e6e4486b175a1b021b0a3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked225(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[13]=1.f; source[14]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
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

// source.character.static-map-native-225.v1 / source program 84885c256921b44da3b51c41f462e064
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase225(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
    source[6].x=g_SourceCharacterBaseConstants[7].x;
    source[6].y=g_SourceCharacterBaseConstants[7].y;
    source[6].z=g_SourceCharacterBaseConstants[7].z;
    source[6].w=g_SourceCharacterBaseConstants[7].w;
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
    source[8].x=g_SourceCharacterBaseConstants[9].x;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
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

// source.character.static-map-native-226.v1 / source program c77620631886f0448151c80fb3d916da
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked226(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[9]=1.f; source[10]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[3].xyzx, cb0[5].yyyy
    r1.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r1.xyz, r0.wwww, v6.xyzx
    r1.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: dp2 r0.w, r2.xyxx, r2.xyxx
    r0.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 11: mul r2.xy, r2.xyxx, cb0[5].xxxx
    r2.xy = ((r2.xyxx)*(source[5].xxxx)).xy;
    // 12: mul r2.xy, r2.xyxx, v2.wwww
    r2.xy = ((r2.xyxx)*(v2.wwww)).xy;
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
    // 23: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 24: mul r3.xyz, r0.wwww, r2.xyzx
    r3.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 25: mad r1.xyz, r3.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r3.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 26: dp2_sat r3.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 27: dp3_sat r3.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 28: dp3_sat r3.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 29: mul r1.xyz, r3.xyzx, r3.xyzx
    r1.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 30: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 31: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 32: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 33: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t4.xyzw, s3
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 34: mul r3.xyz, r3.xyzx, cb0[10].xyzx
    r3.xyz = ((r3.xyzx)*(source[10].xyzx)).xyz;
    // 35: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 36: dp2_sat r1.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 37: dp3_sat r1.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 38: dp3_sat r1.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 39: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 40: dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 41: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t3.wxyz, s3
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 42: mul r1.yzw, r1.yyzw, cb0[9].xxyz
    r1.yzw = ((r1.yyzw)*(source[9].xxyz)).yzw;
    // 43: mul r3.xyz, r1.xxxx, r1.yzwy
    r3.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 44: dp3 r2.w, v7.xyzx, v7.xyzx
    r2.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 45: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 46: mul r4.xyz, r2.wwww, v7.xyzx
    r4.xyz = ((r2.wwww)*(v7.xyzx)).xyz;
    // 47: dp3 r2.w, r4.xyzx, r2.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 48: mad r4.xy, r2.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r2.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 49: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 50: mul r4.yzw, r4.yyyy, cb0[7].xxyz
    r4.yzw = ((r4.yyyy)*(source[7].xxyz)).yzw;
    // 51: mad r4.xyz, r4.xxxx, cb0[6].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[6].xyzx)+(r4.yzwy)).xyz;
    // 52: mul r4.xyz, r4.xyzx, cb0[8].wwww
    r4.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 53: mul r5.xyz, cb0[4].xyzx, cb0[5].zzzz
    r5.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 56: mul_sat r2.w, r6.w, cb0[5].w
    r2.w = (saturate((r6.wwww)*(source[5].wwww))).w;
    // 57: mul o0.w, r2.w, cb0[0].x
    output.targets[0].w = ((r2.wwww)*(source[0].xxxx)).w;
    // 58: mad r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 59: mul r6.xyz, r4.xyzx, r5.xyzx
    r6.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 60: mad r4.xyz, r1.yzwy, r1.xxxx, r4.xyzx
    r4.xyz = ((r1.yzwy)*(r1.xxxx)+(r4.xyzx)).xyz;
    // 61: mul r1.xyz, r1.yzwy, cb2[4].xyzx
    r1.xyz = ((r1.yzwy)*(passValues[4].xyzx)).xyz;
    // 62: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 63: div r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)/(r4.xyzx)).xyz;
    // 64: mad r3.xyz, r5.xyzx, r3.xyzx, r6.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 65: mad r3.xyz, r1.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((r1.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 66: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 67: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 68: dp3 r0.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 69: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 70: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 71: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 72: mad r0.xyz, r5.xyzx, cb0[8].xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(source[8].xyzx)+(r0.xyzx)).xyz;
    // 73: mov o3.xyz, r5.xyzx
    output.targets[3].xyz = (r5.xyzx).xyz;
    // 74: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
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
    // 80: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 81: mul r3.xyz, r0.zxyz, r1.yzxy
    r3.xyz = ((r0.zxyz)*(r1.yzxy)).xyz;
    // 82: mad r3.xyz, r0.yzxy, r1.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r1.zxyz)+(-(r3.xyzx))).xyz;
    // 83: dp3 r0.z, r0.xyzx, r2.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 84: dp3 r0.x, r1.xyzx, r2.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 85: mul r1.xyz, r3.xyzx, v1.wwww
    r1.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 86: dp3 r0.y, r1.xyzx, r2.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
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
    // 100: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 101: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 102: ret
    return output;
}

// source.character.static-map-native-226.v1 / source program 8ca0ad4a4be1d2428ddbac179218d5e1
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase226(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5].x=g_SourceCharacterBaseConstants[5].x;
    source[5].y=g_SourceCharacterBaseConstants[5].y;
    source[5].z=g_SourceCharacterBaseConstants[5].z;
    source[5].w=g_SourceCharacterBaseConstants[5].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s1, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 3: mul r1.xyz, cb0[3].xyzx, cb0[5].yyyy
    r1.xyz = ((source[3].xyzx)*(source[5].yyyy)).xyz;
    // 4: mad r0.xyz, r0.xyzx, r1.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)+(source[1].xyzx)).xyz;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: dp2 r0.w, r1.xyxx, r1.xyxx
    r0.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 8: mul r1.xy, r1.xyxx, cb0[5].xxxx
    r1.xy = ((r1.xyxx)*(source[5].xxxx)).xy;
    // 9: mul r1.xy, r1.xyxx, v2.wwww
    r1.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 10: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 11: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 12: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 13: add r1.z, r0.w, l(0.000010)
    r1.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 15: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 16: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 17: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 18: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 19: mul r1.xyz, r0.wwww, r1.xyzx
    r1.xyz = ((r0.wwww)*(r1.xyzx)).xyz;
    // 20: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 21: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 22: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 23: dp3 r0.w, r2.xyzx, r1.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 24: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 26: mul r2.yzw, r2.yyyy, cb0[7].xxyz
    r2.yzw = ((r2.yyyy)*(source[7].xxyz)).yzw;
    // 27: mad r2.xyz, r2.xxxx, cb0[6].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[6].xyzx)+(r2.yzwy)).xyz;
    // 28: mul r2.xyz, r2.xyzx, cb0[8].wwww
    r2.xyz = ((r2.xyzx)*(source[8].wwww)).xyz;
    // 29: mul r3.xyz, cb0[4].xyzx, cb0[5].zzzz
    r3.xyz = ((source[4].xyzx)*(source[5].zzzz)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 32: mul_sat r0.w, r4.w, cb0[5].w
    r0.w = (saturate((r4.wwww)*(source[5].wwww))).w;
    // 33: mul o0.w, r0.w, cb0[0].x
    output.targets[0].w = ((r0.wwww)*(source[0].xxxx)).w;
    // 34: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 35: mad r0.xyz, r2.xyzx, r3.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.xyzx)+(r0.xyzx)).xyz;
    // 36: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 37: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 38: mad r0.xyz, r3.xyzx, cb0[8].xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(source[8].xyzx)+(r0.xyzx)).xyz;
    // 39: mov o3.xyz, r3.xyzx
    output.targets[3].xyz = (r3.xyzx).xyz;
    // 40: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 41: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 42: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 43: mul r0.xyz, r0.xxxx, v1.xyzx
    r0.xyz = ((r0.xxxx)*(v1.xyzx)).xyz;
    // 44: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 45: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 46: mul r2.xyz, r0.wwww, v0.xyzx
    r2.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 47: mul r3.xyz, r0.zxyz, r2.yzxy
    r3.xyz = ((r0.zxyz)*(r2.yzxy)).xyz;
    // 48: mad r3.xyz, r0.yzxy, r2.zxyz, -r3.xyzx
    r3.xyz = ((r0.yzxy)*(r2.zxyz)+(-(r3.xyzx))).xyz;
    // 49: dp3 r0.z, r0.xyzx, r1.xyzx
    r0.z = (dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 50: dp3 r0.x, r2.xyzx, r1.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 51: mul r2.xyz, r3.xyzx, v1.wwww
    r2.xyz = ((r3.xyzx)*(v1.wwww)).xyz;
    // 52: dp3 r0.y, r2.xyzx, r1.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 53: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 54: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 55: mul r0.xyz, r0.wwww, r0.xyzx
    r0.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 56: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 57: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 58: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 59: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 60: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 61: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 62: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 63: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 64: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 65: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 66: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 67: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 68: ret
    return output;
}

// source.character.static-map-native-227.v1 / source program 0244d5fffc12e644b4414c72efff4494
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked227(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=1.f; source[18]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: mul r1.zw, r1.xxxy, cb0[9].xxxx
    r1.zw = ((r1.xxxy)*(source[9].xxxx)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.zw, r1.zzzw, cb0[9].yyyy
    r1.zw = ((r1.zzzw)*(source[9].yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mul r0.w, r2.z, cb0[11].y
    r0.w = ((r2.zzzz)*(source[11].yyyy)).w;
    // 11: mad r1.zw, cb0[8].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[8].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 12: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 13: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 15: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 16: add r2.z, r2.x, l(0.000010)
    r2.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 18: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 19: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 20: div r2.xyz, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.xyzx)/(r1.zzzz)).xyz;
    // 21: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 22: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 23: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 24: dp3 r4.z, r3.xyzx, r2.xyzx
    r4.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 25: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 26: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 27: mul r5.xyz, r1.zzzz, v0.xyzx
    r5.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 28: mul r6.xyz, r3.zxyz, r5.yzxy
    r6.xyz = ((r3.zxyz)*(r5.yzxy)).xyz;
    // 29: mad r6.xyz, r3.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r3.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 30: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 31: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 32: dp3 r4.x, r5.xyzx, r2.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 33: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 34: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 35: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 36: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 39: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: mul r7.xy, v4.xyxx, cb0[10].zzzz
    r7.xy = ((v4.xyxx)*(source[10].zzzz)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 44: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 45: max r0.z, cb0[9].z, l(0.000000)
    r0.z = (max(source[9].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 46: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 47: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 48: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 49: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 50: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 52: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 53: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 54: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 55: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 56: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 57: mad r2.xyz, r0.yyyy, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 58: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 59: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 60: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 61: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r8.xyz, r0.yyyy, v5.xyzx
    r8.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 64: dp3 r0.y, r2.xyzx, r8.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 65: mul r9.xyz, r0.yyyy, r2.xyzx
    r9.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 66: mad r8.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r8.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 67: dp3 r9.x, r5.xyzx, r8.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 68: dp3 r5.x, r5.xyzx, r2.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 69: dp3 r9.y, r6.xyzx, r8.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 70: dp3 r5.y, r6.xyzx, r2.xyzx
    r5.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 71: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 72: mad r0.yz, cb0[10].wwww, r9.xxyx, r0.yyzy
    r0.yz = ((source[10].wwww)*(r9.xxyx)+(r0.yyzy)).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mul r6.xyz, r6.xyzx, cb0[4].xyzx
    r6.xyz = ((r6.xyzx)*(source[4].xyzx)).xyz;
    // 75: mad r6.xyz, cb0[11].xxxx, r6.xyzx, r6.xyzx
    r6.xyz = ((source[11].xxxx)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 76: add r6.xyz, r6.xyzx, -cb0[11].xxxx
    r6.xyz = ((r6.xyzx)+(-(source[11].xxxx))).xyz;
    // 77: mov_sat r9.xyz, r6.xyzx
    r9.xyz = (saturate(r6.xyzx)).xyz;
    // 78: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 79: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 81: add r10.xyz, -r4.xyzx, r0.yyyy
    r10.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 82: mad r4.xyz, cb0[11].wwww, r10.xyzx, r4.xyzx
    r4.xyz = ((source[11].wwww)*(r10.xyzx)+(r4.xyzx)).xyz;
    // 83: mul r10.xyz, cb0[5].xyzx, cb0[12].xxxx
    r10.xyz = ((source[5].xyzx)*(source[12].xxxx)).xyz;
    // 84: mul r4.xyz, r4.xyzx, r10.xyzx
    r4.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 85: mad r0.yzw, r0.wwww, r9.xxyz, r4.xxyz
    r0.yzw = ((r0.wwww)*(r9.xxyz)+(r4.xxyz)).yzw;
    // 86: mul r0.yzw, r6.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)*(r0.yyzw)).yzw;
    // 87: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 88: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 89: mul r4.xyz, cb0[6].xyzx, cb0[12].yyyy
    r4.xyz = ((source[6].xyzx)*(source[12].yyyy)).xyz;
    // 90: mul r6.xyz, r7.xyzx, r4.xyzx
    r6.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 91: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 92: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 93: mad r4.xyz, cb0[12].wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((source[12].wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 94: add r4.xyz, -r0.yzwy, r4.xyzx
    r4.xyz = ((-(r0.yzwy))+(r4.xyzx)).xyz;
    // 95: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 96: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 97: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 98: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 99: mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 100: dp3 r1.w, r4.xyzx, r2.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 101: mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 103: mul r4.yzw, r4.yyyy, cb0[15].xxyz
    r4.yzw = ((r4.yyyy)*(source[15].xxyz)).yzw;
    // 104: mad r4.xyz, r4.xxxx, cb0[14].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[14].xyzx)+(r4.yzwy)).xyz;
    // 105: mul r4.xyz, r4.xyzx, cb0[16].wwww
    r4.xyz = ((r4.xyzx)*(source[16].wwww)).xyz;
    // 106: mul r6.xyz, r0.yzwy, r4.xyzx
    r6.xyz = ((r0.yzwy)*(r4.xyzx)).xyz;
    // 107: dp2_sat r9.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 108: dp3_sat r9.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 109: dp3_sat r9.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 110: dp3 r5.z, r3.xyzx, r2.xyzx
    r5.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 111: mul r2.xyz, r9.xyzx, r9.xyzx
    r2.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 112: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t7.xyzw, s6
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 113: mul r3.xyz, r3.xyzx, cb0[18].xyzx
    r3.xyz = ((r3.xyzx)*(source[18].xyzx)).xyz;
    // 114: dp3 r1.w, r3.xyzx, r2.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 115: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s6
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 116: mul r2.xyz, r2.xyzx, cb0[17].xyzx
    r2.xyz = ((r2.xyzx)*(source[17].xyzx)).xyz;
    // 117: mul r9.xyz, r1.wwww, r2.xyzx
    r9.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 118: mad r4.xyz, r2.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 119: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 120: div r4.xyz, r9.xyzx, r4.xyzx
    r4.xyz = ((r9.xyzx)/(r4.xyzx)).xyz;
    // 121: mad r6.xyz, r0.yzwy, r9.xyzx, r6.xyzx
    r6.xyz = ((r0.yzwy)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 122: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 124: add r4.xyz, -r7.xyzx, r2.wwww
    r4.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 125: mad r4.xyz, cb0[12].wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((source[12].wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 126: mul r7.xyz, cb0[7].xyzx, cb0[13].xxxx
    r7.xyz = ((source[7].xyzx)*(source[13].xxxx)).xyz;
    // 127: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 128: mad r4.xyz, cb0[13].yyyy, r4.xyzx, -r1.xyzx
    r4.xyz = ((source[13].yyyy)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 129: mad r1.xyz, r0.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 130: mad r1.xyz, r1.xyzx, cb2[4].wwww, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 131: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 132: dp2_sat r2.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 133: dp3_sat r2.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 134: dp3_sat r2.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 135: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 136: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 137: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 138: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 139: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 140: mad r2.xyz, r1.xyzx, r0.xxxx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r0.xxxx)+(r6.xyzx)).xyz;
    // 141: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 142: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 143: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 144: mad o0.xyz, r0.yzwy, cb0[16].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[16].xyzx)+(r1.xyzx)).xyz;
    // 145: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 146: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 147: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 148: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 149: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 150: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 151: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 152: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 153: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 154: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 155: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 156: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 157: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 158: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 159: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 160: mul o4.z, r1.w, r2.x
    output.targets[4].z = ((r1.wwww)*(r2.xxxx)).z;
    // 161: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 162: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 163: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 164: ret
    return output;
}

// source.character.static-map-native-227.v1 / source program 454b0701566b1043b4b4ae633f81b11a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase227(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
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
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
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
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: mul r0.w, r2.z, cb0[10].y
    r0.w = ((r2.zzzz)*(source[10].yyyy)).w;
    // 12: mad r1.zw, cb0[7].wwww, r1.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r1.xxxy)+(r1.zzzw)).zw;
    // 13: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 14: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 15: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 16: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 17: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 19: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 21: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 26: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 29: mul r6.xyz, r2.zxyz, r5.yzxy
    r6.xyz = ((r2.zxyz)*(r5.yzxy)).xyz;
    // 30: mad r6.xyz, r2.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r2.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 31: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r4.x, r5.xyzx, r1.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r4.xy, v4.xyxx, cb0[9].zzzz
    r4.xy = ((v4.xyxx)*(source[9].zzzz)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r4.w, r4.w
    r0.z = ((r4.wwww)*(r4.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 48: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 51: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 55: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 57: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 60: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 61: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 62: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 63: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 64: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 65: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 66: dp3 r8.x, r5.xyzx, r7.xyzx
    r8.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 67: dp3 r8.y, r6.xyzx, r7.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.y, r6.xyzx, r1.xyzx
    r6.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 70: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 71: mad r0.yz, cb0[9].wwww, r8.xxyx, r0.yyzy
    r0.yz = ((source[9].wwww)*(r8.xxyx)+(r0.yyzy)).yz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 74: mad r5.xyz, cb0[10].xxxx, r5.xyzx, r5.xyzx
    r5.xyz = ((source[10].xxxx)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 75: add r5.xyz, r5.xyzx, -cb0[10].xxxx
    r5.xyz = ((r5.xyzx)+(-(source[10].xxxx))).xyz;
    // 76: mov_sat r7.xyz, r5.xyzx
    r7.xyz = (saturate(r5.xyzx)).xyz;
    // 77: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 78: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 80: add r8.xyz, -r3.xyzx, r0.yyyy
    r8.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 81: mad r3.xyz, cb0[10].wwww, r8.xyzx, r3.xyzx
    r3.xyz = ((source[10].wwww)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r8.xyz, cb0[5].xyzx, cb0[11].xxxx
    r8.xyz = ((source[5].xyzx)*(source[11].xxxx)).xyz;
    // 83: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 84: mad r0.yzw, r0.wwww, r7.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r7.xxyz)+(r3.xxyz)).yzw;
    // 85: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 86: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 87: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 88: mul r3.xyz, cb0[6].xyzx, cb0[11].yyyy
    r3.xyz = ((source[6].xyzx)*(source[11].yyyy)).xyz;
    // 89: mul r5.xyz, r4.xyzx, r3.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 90: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: mad r3.xyz, -r3.xyzx, r4.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 92: mad r3.xyz, cb0[11].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[11].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 93: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 94: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 95: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 97: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 98: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 99: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 100: dp3 r6.z, r2.xyzx, r1.xyzx
    r6.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 101: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 103: mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // 104: mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // 105: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 106: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 107: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 108: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 109: mad o0.xyz, r0.xyzx, cb0[14].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)+(r2.xyzx)).xyz;
    // 110: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 111: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 113: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 114: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 115: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 116: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 117: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 118: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 119: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 120: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 121: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 122: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 123: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 124: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 125: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 126: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 127: ret
    return output;
}

// source.character.static-map-native-228.v1 / source program e80dfa510fd2934d8335ac9d5fff5e8a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked228(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=1.f; source[16]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: mul r0.xyzw, v4.xyxy, cb0[7].yyww
    r0.xyzw = ((v4.xyxy)*(source[7].yyww)).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r0.zwzz, t2.xyzw, s2, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 5: mul r2.xy, r1.xyxx, cb0[8].xxxx
    r2.xy = ((r1.xyxx)*(source[8].xxxx)).xy;
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
    // 13: mul r0.xy, r0.xyxx, cb0[7].zzzz
    r0.xy = ((r0.xyxx)*(source[7].zzzz)).xy;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 15: mad r0.zw, r3.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r3.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 16: mul r2.w, r3.z, cb0[9].w
    r2.w = ((r3.zzzz)*(source[9].wwww)).w;
    // 17: mad r0.xy, cb0[7].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[7].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
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
    // 28: dp3 r0.w, v1.xyzx, v1.xyzx
    r0.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 29: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 30: mul r3.xyz, r0.wwww, v1.xyzx
    r3.xyz = ((r0.wwww)*(v1.xyzx)).xyz;
    // 31: dp3 r4.z, r3.xyzx, r0.xyzx
    r4.z = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 32: max r5.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r5.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 33: min r5.xyz, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = (min(r5.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 34: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 35: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 36: mul r6.xyz, r0.wwww, v0.xyzx
    r6.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 37: dp3 r4.x, r6.xyzx, r0.xyzx
    r4.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 38: mul r7.xyz, r3.zxyz, r6.yzxy
    r7.xyz = ((r3.zxyz)*(r6.yzxy)).xyz;
    // 39: mad r7.xyz, r3.yzxy, r6.zxyz, -r7.xyzx
    r7.xyz = ((r3.yzxy)*(r6.zxyz)+(-(r7.xyzx))).xyz;
    // 40: mul r7.xyz, r7.xyzx, v1.wwww
    r7.xyz = ((r7.xyzx)*(v1.wwww)).xyz;
    // 41: dp3 r4.y, r7.xyzx, r0.xyzx
    r4.y = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 42: dp3 r0.w, r4.xyzx, r5.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 43: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: mad r0.w, r0.w, l(0.500000), cb0[9].x
    r0.w = ((r0.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].xxxx)).w;
    // 45: mul r3.w, r0.z, r0.z
    r3.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul_sat r3.w, r3.w, r4.w
    r3.w = (saturate((r3.wwww)*(r4.wwww))).w;
    // 48: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 50: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 51: max r3.w, cb0[8].y, l(0.000000)
    r3.w = (max(source[8].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: min r3.w, r3.w, l(0.990000)
    r3.w = (min(r3.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 53: mul r4.w, r1.w, r3.w
    r4.w = ((r1.wwww)*(r3.wwww)).w;
    // 54: mad r0.w, r0.w, r4.w, r0.w
    r0.w = ((r0.wwww)*(r4.wwww)+(r0.wwww)).w;
    // 55: add r4.w, -r3.w, r0.w
    r4.w = ((-(r3.wwww))+(r0.wwww)).w;
    // 56: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: div r3.w, l(1.000000, 1.000000, 1.000000, 1.000000), r3.w
    r3.w = (((r3.wwww) != 0.f ? 1.f / (r3.wwww) : 0.f)).w;
    // 58: mad r0.w, -r3.w, r4.w, r0.w
    r0.w = ((-(r3.wwww))*(r4.wwww)+(r0.wwww)).w;
    // 59: mul r3.w, r4.w, r3.w
    r3.w = ((r4.wwww)*(r3.wwww)).w;
    // 60: mad_sat r0.w, r1.w, r0.w, r3.w
    r0.w = (saturate((r1.wwww)*(r0.wwww)+(r3.wwww))).w;
    // 61: mul r1.w, r0.w, l(0.650000)
    r1.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 62: mad r0.xyz, r1.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 63: dp3 r1.w, r0.xyzx, r0.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 64: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 65: mul r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 66: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 67: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 68: mul r2.xyz, r1.wwww, v5.xyzx
    r2.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 69: dp3 r1.w, r0.xyzx, r2.xyzx
    r1.w = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 70: mul r5.xyz, r0.xyzx, r1.wwww
    r5.xyz = ((r0.xyzx)*(r1.wwww)).xyz;
    // 71: mad r2.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r2.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 72: dp3 r5.x, r6.xyzx, r2.xyzx
    r5.x = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 73: dp3 r6.x, r6.xyzx, r0.xyzx
    r6.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 74: dp3 r5.y, r7.xyzx, r2.xyzx
    r5.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 75: dp3 r6.y, r7.xyzx, r0.xyzx
    r6.y = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 76: mul r5.zw, cb0[0].xxxy, l(0.000000, 0.000000, 0.000300, 0.000300)
    r5.zw = ((source[0].xxxy)*(float4(0.000000,0.000000,0.000300,0.000300))).zw;
    // 77: mad r5.xy, cb0[9].yyyy, r5.xyxx, r5.zwzz
    r5.xy = ((source[9].yyyy)*(r5.xyxx)+(r5.zwzz)).xy;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: mul r5.xyz, r5.xyzx, cb0[3].xyzx
    r5.xyz = ((r5.xyzx)*(source[3].xyzx)).xyz;
    // 80: mad r5.xyz, cb0[9].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[9].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 81: add r5.xyz, r5.xyzx, -cb0[9].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[9].zzzz))).xyz;
    // 82: mov_sat r7.xyz, r5.xyzx
    r7.xyz = (saturate(r5.xyzx)).xyz;
    // 83: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 84: mad r5.xyz, -r2.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r2.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r8.xyz, cb0[4].xyzx, cb0[10].xxxx
    r8.xyz = ((source[4].xyzx)*(source[10].xxxx)).xyz;
    // 86: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 87: mad r4.xyz, r2.wwww, r7.xyzx, r4.xyzx
    r4.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xyzx)).xyz;
    // 88: mul r4.xyz, r5.xyzx, r4.xyzx
    r4.xyz = ((r5.xyzx)*(r4.xyzx)).xyz;
    // 89: max r4.xyz, r4.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r4.xyz = (max(r4.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: min r4.xyz, r4.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r4.xyz = (min(r4.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 91: mul r5.xyz, cb0[5].xyzx, cb0[10].yyyy
    r5.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 92: mul r7.xyz, r1.xyzx, r5.xyzx
    r7.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 93: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: mad r5.xyz, -r5.xyzx, r1.xyzx, r1.wwww
    r5.xyz = ((-(r5.xyzx))*(r1.xyzx)+(r1.wwww)).xyz;
    // 95: mad r5.xyz, cb0[10].wwww, r5.xyzx, r7.xyzx
    r5.xyz = ((source[10].wwww)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 96: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 97: mad r4.xyz, r0.wwww, r5.xyzx, r4.xyzx
    r4.xyz = ((r0.wwww)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 98: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 99: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 100: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 101: mul r5.xyz, r1.wwww, v6.xyzx
    r5.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 102: dp3 r1.w, r5.xyzx, r0.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 103: mad r5.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 104: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 105: mul r5.yzw, r5.yyyy, cb0[13].xxyz
    r5.yzw = ((r5.yyyy)*(source[13].xxyz)).yzw;
    // 106: mad r5.xyz, r5.xxxx, cb0[12].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[12].xyzx)+(r5.yzwy)).xyz;
    // 107: mul r5.xyz, r5.xyzx, cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(source[14].wwww)).xyz;
    // 108: mul r7.xyz, r4.xyzx, r5.xyzx
    r7.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 109: dp2_sat r8.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 110: dp3_sat r8.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 111: dp3_sat r8.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 112: dp3 r6.z, r3.xyzx, r0.xyzx
    r6.z = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 113: mul r0.xyz, r8.xyzx, r8.xyzx
    r0.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 114: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t8.xyzw, s7
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 115: mul r3.xyz, r3.xyzx, cb0[16].xyzx
    r3.xyz = ((r3.xyzx)*(source[16].xyzx)).xyz;
    // 116: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 117: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s7
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 118: mul r8.xyz, r8.xyzx, cb0[15].xyzx
    r8.xyz = ((r8.xyzx)*(source[15].xyzx)).xyz;
    // 119: mul r9.xyz, r0.xxxx, r8.xyzx
    r9.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 120: mad r0.xyz, r8.xyzx, r0.xxxx, r5.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xxxx)+(r5.xyzx)).xyz;
    // 121: add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 122: div r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)/(r0.xyzx)).xyz;
    // 123: mad r5.xyz, r4.xyzx, r9.xyzx, r7.xyzx
    r5.xyz = ((r4.xyzx)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 124: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 125: dp3 r0.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 126: add r7.xyz, -r1.xyzx, r0.yyyy
    r7.xyz = ((-(r1.xyzx))+(r0.yyyy)).xyz;
    // 127: mad r1.xyz, cb0[10].wwww, r7.xyzx, r1.xyzx
    r1.xyz = ((source[10].wwww)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 128: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, v4.xyxx, t6.xyzw, s6, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 129: mul r9.xyz, cb0[6].xyzx, cb0[11].xxxx
    r9.xyz = ((source[6].xyzx)*(source[11].xxxx)).xyz;
    // 130: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 131: mad r1.xyz, cb0[11].yyyy, r1.xyzx, -r7.xyzx
    r1.xyz = ((source[11].yyyy)*(r1.xyzx)+(-(r7.xyzx))).xyz;
    // 132: mad r0.yzw, r0.wwww, r1.xxyz, r7.xxyz
    r0.yzw = ((r0.wwww)*(r1.xxyz)+(r7.xxyz)).yzw;
    // 133: mad r0.yzw, r0.yyzw, cb2[4].wwww, cb2[4].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[4].wwww)+(passValues[4].xxyz)).yzw;
    // 134: mul r0.yzw, r8.xxyz, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r0.yyzw)).yzw;
    // 135: dp2_sat r1.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r1.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 136: dp3_sat r1.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r1.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 137: dp3_sat r1.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r1.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 138: log r1.xyz, r1.xyzx
    r1.xyz = (log2(r1.xyzx)).xyz;
    // 139: add r1.w, cb0[11].z, l(1.000000)
    r1.w = ((source[11].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 141: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 142: dp3 r1.x, r3.xyzx, r1.xyzx
    r1.x = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 143: mad r1.yzw, r0.yyzw, r1.xxxx, r5.xxyz
    r1.yzw = ((r0.yyzw)*(r1.xxxx)+(r5.xxyz)).yzw;
    // 144: mul r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)*(r1.xxxx)).yzw;
    // 145: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 146: add r0.yzw, r1.yyzw, cb0[1].xxyz
    r0.yzw = ((r1.yyzw)+(source[1].xxyz)).yzw;
    // 147: mad o0.xyz, r4.xyzx, cb0[14].xyzx, r0.yzwy
    output.targets[0].xyz = ((r4.xyzx)*(source[14].xyzx)+(r0.yzwy)).xyz;
    // 148: mov o3.xyz, r4.xyzx
    output.targets[3].xyz = (r4.xyzx).xyz;
    // 149: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 150: dp3 r0.y, r6.xyzx, r6.xyzx
    r0.y = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 151: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 152: mul r0.yzw, r0.yyyy, r6.xxyz
    r0.yzw = ((r0.yyyy)*(r6.xxyz)).yzw;
    // 153: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 154: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 155: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 156: ge r2.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 157: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 158: mad r2.xy, -|r0.zyzz|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.zyzz)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 159: movc r0.yz, r1.xxxx, r2.xxyx, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r2.xxyx) : (r0.yyzy)).yz;
    // 160: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 161: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 162: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 163: mul o4.z, r0.x, r1.y
    output.targets[4].z = ((r0.xxxx)*(r1.yyyy)).z;
    // 164: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 165: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 166: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 167: ret
    return output;
}

// source.character.static-map-native-228.v1 / source program 61c8edc8012afa44bed8c7ff347e89f7
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase228(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
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
    source[9].x=g_SourceCharacterBaseConstants[10].x;
    source[9].y=g_SourceCharacterBaseConstants[10].y;
    source[9].z=g_SourceCharacterBaseConstants[10].z;
    source[9].w=g_SourceCharacterBaseConstants[10].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: mul r0.w, r1.z, cb0[8].w
    r0.w = ((r1.zzzz)*(source[8].wwww)).w;
    // 6: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 8: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 9: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 10: add r2.z, r1.z, l(0.000010)
    r2.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 11: mul r3.xyzw, v4.xyxy, cb0[6].yyww
    r3.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r3.xyxx, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 13: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 14: mul r1.zw, r1.zzzw, cb0[6].zzzz
    r1.zw = ((r1.zzzw)*(source[6].zzzz)).zw;
    // 15: mad r1.xy, cb0[6].xxxx, r1.xyxx, r1.zwzz
    r1.xy = ((source[6].xxxx)*(r1.xyxx)+(r1.zwzz)).xy;
    // 16: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 17: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 18: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 19: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 20: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 23: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 24: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 25: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 26: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 27: mul r6.xyz, r2.zxyz, r5.yzxy
    r6.xyz = ((r2.zxyz)*(r5.yzxy)).xyz;
    // 28: mad r6.xyz, r2.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r2.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 29: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 30: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 31: dp3 r4.x, r5.xyzx, r1.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 32: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 33: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 34: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 35: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 38: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r3.zwzz, t4.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, r3.zwzz, t2.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r3.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 41: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 42: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[7].y, l(0.000000)
    r0.z = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 48: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 51: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 55: dp2 r0.z, r3.xyxx, r3.xyxx
    r0.z = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 56: mul r3.xy, r3.xyxx, cb0[7].xxxx
    r3.xy = ((r3.xyxx)*(source[7].xxxx)).xy;
    // 57: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 58: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 59: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 60: add r3.z, r0.z, l(0.000010)
    r3.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 61: add r3.xyz, -r1.xyzx, r3.xyzx
    r3.xyz = ((-(r1.xyzx))+(r3.xyzx)).xyz;
    // 62: mad r1.xyz, r0.yyyy, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 63: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 64: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 65: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 66: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 67: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 68: mul r3.xyz, r0.yyyy, v5.xyzx
    r3.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 69: dp3 r0.y, r1.xyzx, r3.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 70: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 71: mad r3.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r3.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 72: dp3 r8.x, r5.xyzx, r3.xyzx
    r8.x = (dot((r5.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 73: dp3 r8.y, r6.xyzx, r3.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 74: dp3 r3.y, r6.xyzx, r1.xyzx
    r3.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 75: dp3 r3.x, r5.xyzx, r1.xyzx
    r3.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 76: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 77: mad r0.yz, cb0[8].yyyy, r8.xxyx, r0.yyzy
    r0.yz = ((source[8].yyyy)*(r8.xxyx)+(r0.yyzy)).yz;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t5.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: mul r5.xyz, r5.xyzx, cb0[3].xyzx
    r5.xyz = ((r5.xyzx)*(source[3].xyzx)).xyz;
    // 80: mad r5.xyz, cb0[8].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 81: add r5.xyz, r5.xyzx, -cb0[8].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[8].zzzz))).xyz;
    // 82: mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // 83: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 84: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: mul r8.xyz, cb0[4].xyzx, cb0[9].xxxx
    r8.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 86: mul r4.xyz, r4.xyzx, r8.xyzx
    r4.xyz = ((r4.xyzx)*(r8.xyzx)).xyz;
    // 87: mad r0.yzw, r0.wwww, r6.xxyz, r4.xxyz
    r0.yzw = ((r0.wwww)*(r6.xxyz)+(r4.xxyz)).yzw;
    // 88: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 89: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 90: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 91: mul r4.xyz, cb0[5].xyzx, cb0[9].yyyy
    r4.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 92: mul r5.xyz, r7.xyzx, r4.xyzx
    r5.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 93: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 94: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 95: mad r4.xyz, cb0[9].wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((source[9].wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 96: add r4.xyz, -r0.yzwy, r4.xyzx
    r4.xyz = ((-(r0.yzwy))+(r4.xyzx)).xyz;
    // 97: mad r0.xyz, r0.xxxx, r4.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r4.xyzx)+(r0.yzwy)).xyz;
    // 98: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 99: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 100: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 101: mul r4.xyz, r0.wwww, v6.xyzx
    r4.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 102: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 103: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 104: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 105: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 106: mul r1.yzw, r1.yyyy, cb0[11].xxyz
    r1.yzw = ((r1.yyyy)*(source[11].xxyz)).yzw;
    // 107: mad r1.xyz, r1.xxxx, cb0[10].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[10].xyzx)+(r1.yzwy)).xyz;
    // 108: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 109: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 110: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 111: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 112: mad o0.xyz, r0.xyzx, cb0[12].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 113: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 114: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 115: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 116: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 117: mul r0.xyz, r0.xxxx, r3.xyzx
    r0.xyz = ((r0.xxxx)*(r3.xyzx)).xyz;
    // 118: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 119: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 120: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 121: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 122: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 123: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 124: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 125: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 126: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 127: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 128: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 129: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 130: ret
    return output;
}

// source.character.static-map-native-229.v1 / source program 4f45da3cc17c784c9dfbd45486658ab2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked229(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=1.f; source[16]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: mul r0.xy, cb0[1].xyxx, cb0[8].xxxx
    r0.xy = ((source[1].xyxx)*(source[8].xxxx)).xy;
    // 2: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 3: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, v4.xyxx, cb0[7].yyyy
    r1.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t1.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 6: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 7: mul r1.xy, r1.xyxx, cb0[7].zzzz
    r1.xy = ((r1.xyxx)*(source[7].zzzz)).xy;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: mul r0.w, r2.z, cb0[9].w
    r0.w = ((r2.zzzz)*(source[9].wwww)).w;
    // 11: mad r1.xy, cb0[7].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[7].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 12: dp2 r1.z, r1.zwzz, r1.zwzz
    r1.z = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).z;
    // 13: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 14: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 15: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 16: add r2.z, r1.z, l(0.000010)
    r2.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 18: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 19: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 20: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 21: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 22: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 23: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 24: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 25: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 26: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 29: mul r5.xyz, r2.zxyz, r4.yzxy
    r5.xyz = ((r2.zxyz)*(r4.yzxy)).xyz;
    // 30: mad r5.xyz, r2.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r2.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 31: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r3.x, r4.xyzx, r1.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[8].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].wwww)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: mul r6.xy, v4.xyxx, cb0[9].xxxx
    r6.xy = ((v4.xyxx)*(source[9].xxxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 44: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 45: max r0.z, cb0[7].w, l(0.000000)
    r0.z = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 46: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 47: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 48: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 49: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 50: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 52: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 53: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 54: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 55: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 56: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 57: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 58: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 59: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 60: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 61: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 64: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 65: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 66: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 67: dp3 r8.x, r4.xyzx, r7.xyzx
    r8.x = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 68: dp3 r4.x, r4.xyzx, r1.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 69: dp3 r8.y, r5.xyzx, r7.xyzx
    r8.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 70: dp3 r4.y, r5.xyzx, r1.xyzx
    r4.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 71: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 72: mad r0.yz, cb0[9].yyyy, r8.xxyx, r0.yyzy
    r0.yz = ((source[9].yyyy)*(r8.xxyx)+(r0.yyzy)).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mul r5.xyz, r5.xyzx, cb0[3].xyzx
    r5.xyz = ((r5.xyzx)*(source[3].xyzx)).xyz;
    // 75: mad r5.xyz, cb0[9].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[9].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 76: add r5.xyz, r5.xyzx, -cb0[9].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[9].zzzz))).xyz;
    // 77: mov_sat r8.xyz, r5.xyzx
    r8.xyz = (saturate(r5.xyzx)).xyz;
    // 78: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 79: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r9.xyz, cb0[4].xyzx, cb0[10].xxxx
    r9.xyz = ((source[4].xyzx)*(source[10].xxxx)).xyz;
    // 81: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 82: mad r0.yzw, r0.wwww, r8.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r8.xxyz)+(r3.xxyz)).yzw;
    // 83: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 84: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 85: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 86: mul r3.xyz, cb0[5].xyzx, cb0[10].yyyy
    r3.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 87: mul r5.xyz, r6.xyzx, r3.xyzx
    r5.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 88: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 90: mad r3.xyz, cb0[10].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 91: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 92: mad r0.yzw, r0.xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 93: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 94: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 95: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 96: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 97: dp3 r1.w, r3.xyzx, r1.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 98: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 100: mul r3.yzw, r3.yyyy, cb0[13].xxyz
    r3.yzw = ((r3.yyyy)*(source[13].xxyz)).yzw;
    // 101: mad r3.xyz, r3.xxxx, cb0[12].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[12].xyzx)+(r3.yzwy)).xyz;
    // 102: mul r3.xyz, r3.xyzx, cb0[14].wwww
    r3.xyz = ((r3.xyzx)*(source[14].wwww)).xyz;
    // 103: mul r5.xyz, r0.yzwy, r3.xyzx
    r5.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 104: dp2_sat r8.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 105: dp3_sat r8.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 106: dp3_sat r8.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 107: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 108: mul r1.xyz, r8.xyzx, r8.xyzx
    r1.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 109: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t7.xyzw, s6
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 110: mul r2.xyz, r2.xyzx, cb0[16].xyzx
    r2.xyz = ((r2.xyzx)*(source[16].xyzx)).xyz;
    // 111: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 112: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t6.wxyz, s6
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 113: mul r1.yzw, r1.yyzw, cb0[15].xxyz
    r1.yzw = ((r1.yyzw)*(source[15].xxyz)).yzw;
    // 114: mul r8.xyz, r1.xxxx, r1.yzwy
    r8.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 115: mad r3.xyz, r1.yzwy, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.yzwy)*(r1.xxxx)+(r3.xyzx)).xyz;
    // 116: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 117: div r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)/(r3.xyzx)).xyz;
    // 118: mad r5.xyz, r0.yzwy, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.yzwy)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 119: dp3 r1.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 120: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 121: add r3.xyz, -r6.xyzx, r2.wwww
    r3.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 122: mad r3.xyz, cb0[10].wwww, r3.xyzx, r6.xyzx
    r3.xyz = ((source[10].wwww)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 123: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 124: mul r8.xyz, cb0[6].xyzx, cb0[11].xxxx
    r8.xyz = ((source[6].xyzx)*(source[11].xxxx)).xyz;
    // 125: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 126: mad r3.xyz, cb0[11].yyyy, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[11].yyyy)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 127: mad r3.xyz, r0.xxxx, r3.xyzx, r6.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 128: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 129: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 130: dp2_sat r3.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 131: dp3_sat r3.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 132: dp3_sat r3.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 133: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 134: add r0.x, cb0[11].z, l(1.000000)
    r0.x = ((source[11].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mul r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 136: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 137: dp3 r0.x, r2.xyzx, r3.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 138: mad r2.xyz, r1.yzwy, r0.xxxx, r5.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xxxx)+(r5.xyzx)).xyz;
    // 139: mul r1.yzw, r0.xxxx, r1.yyzw
    r1.yzw = ((r0.xxxx)*(r1.yyzw)).yzw;
    // 140: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 141: add r1.yzw, r2.xxyz, cb0[2].xxyz
    r1.yzw = ((r2.xxyz)+(source[2].xxyz)).yzw;
    // 142: mad o0.xyz, r0.yzwy, cb0[14].xyzx, r1.yzwy
    output.targets[0].xyz = ((r0.yzwy)*(source[14].xyzx)+(r1.yzwy)).xyz;
    // 143: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 144: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 145: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 146: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 147: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 148: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 149: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 150: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 151: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 152: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 153: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 154: movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 155: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 156: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 157: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 158: mul o4.z, r1.x, r2.x
    output.targets[4].z = ((r1.xxxx)*(r2.xxxx)).z;
    // 159: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 160: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 161: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 162: ret
    return output;
}

// source.character.static-map-native-229.v1 / source program 3a3496fe0eb7614ca5fb220419a10200
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase229(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
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
    source[9].x=g_SourceCharacterBaseConstants[10].x;
    source[9].y=g_SourceCharacterBaseConstants[10].y;
    source[9].z=g_SourceCharacterBaseConstants[10].z;
    source[9].w=g_SourceCharacterBaseConstants[10].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: mul r0.xy, cb0[1].xyxx, cb0[7].xxxx
    r0.xy = ((source[1].xyxx)*(source[7].xxxx)).xy;
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
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: mad r1.zw, r2.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r2.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 10: mul r0.w, r2.z, cb0[8].w
    r0.w = ((r2.zzzz)*(source[8].wwww)).w;
    // 11: mad r1.xy, cb0[6].xxxx, r1.zwzz, r1.xyxx
    r1.xy = ((source[6].xxxx)*(r1.zwzz)+(r1.xyxx)).xy;
    // 12: dp2 r1.z, r1.zwzz, r1.zwzz
    r1.z = (dot((r1.zwzz).xy,(r1.zwzz).xy).xxxx).z;
    // 13: add r1.z, -r1.z, l(1.000000)
    r1.z = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 14: max r1.z, r1.z, l(0.000000)
    r1.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 15: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 16: add r2.z, r1.z, l(0.000010)
    r2.z = ((r1.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 18: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 19: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 20: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 21: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 22: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 23: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 24: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 25: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 26: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 29: mul r5.xyz, r2.zxyz, r4.yzxy
    r5.xyz = ((r2.zxyz)*(r4.yzxy)).xyz;
    // 30: mad r5.xyz, r2.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r2.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 31: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r3.x, r4.xyzx, r1.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 40: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 41: mul r6.xy, v4.xyxx, cb0[8].xxxx
    r6.xy = ((v4.xyxx)*(source[8].xxxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 44: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 45: max r0.z, cb0[6].w, l(0.000000)
    r0.z = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 46: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 47: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 48: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 49: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 50: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 51: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 52: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 53: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 54: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 55: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 56: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 57: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 58: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 59: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 60: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 61: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 62: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 63: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 64: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 65: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 66: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 67: dp3 r8.x, r4.xyzx, r7.xyzx
    r8.x = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 68: dp3 r8.y, r5.xyzx, r7.xyzx
    r8.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 69: dp3 r5.y, r5.xyzx, r1.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 70: dp3 r5.x, r4.xyzx, r1.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 71: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 72: mad r0.yz, cb0[8].yyyy, r8.xxyx, r0.yyzy
    r0.yz = ((source[8].yyyy)*(r8.xxyx)+(r0.yyzy)).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mul r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // 75: mad r4.xyz, cb0[8].zzzz, r4.xyzx, r4.xyzx
    r4.xyz = ((source[8].zzzz)*(r4.xyzx)+(r4.xyzx)).xyz;
    // 76: add r4.xyz, r4.xyzx, -cb0[8].zzzz
    r4.xyz = ((r4.xyzx)+(-(source[8].zzzz))).xyz;
    // 77: mov_sat r7.xyz, r4.xyzx
    r7.xyz = (saturate(r4.xyzx)).xyz;
    // 78: mov_sat r4.xyz, -r4.xyzx
    r4.xyz = (saturate(-(r4.xyzx))).xyz;
    // 79: mad r4.xyz, -r0.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 80: mul r8.xyz, cb0[4].xyzx, cb0[9].xxxx
    r8.xyz = ((source[4].xyzx)*(source[9].xxxx)).xyz;
    // 81: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 82: mad r0.yzw, r0.wwww, r7.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r7.xxyz)+(r3.xxyz)).yzw;
    // 83: mul r0.yzw, r4.xxyz, r0.yyzw
    r0.yzw = ((r4.xxyz)*(r0.yyzw)).yzw;
    // 84: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 85: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 86: mul r3.xyz, cb0[5].xyzx, cb0[9].yyyy
    r3.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 87: mul r4.xyz, r6.xyzx, r3.xyzx
    r4.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 88: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 89: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 90: mad r3.xyz, cb0[9].wwww, r3.xyzx, r4.xyzx
    r3.xyz = ((source[9].wwww)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 91: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 92: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 93: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 94: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 97: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 98: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 99: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 100: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 101: mul r1.yzw, r1.yyyy, cb0[11].xxyz
    r1.yzw = ((r1.yyyy)*(source[11].xxyz)).yzw;
    // 102: mad r1.xyz, r1.xxxx, cb0[10].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[10].xyzx)+(r1.yzwy)).xyz;
    // 103: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 104: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[2].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[2].xyzx)).xyz;
    // 105: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 106: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 107: mad o0.xyz, r0.xyzx, cb0[12].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 108: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 109: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 110: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 111: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 112: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 113: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 114: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 115: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 116: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 117: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 118: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 119: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 120: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 121: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 122: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 123: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 124: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 125: ret
    return output;
}

// source.character.static-map-native-230.v1 / source program 062c7c8607299e4cac585abcd5663db5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked230(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=1.f; source[18]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: mul r1.xy, v4.xyxx, cb0[2].xyxx
    r1.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 4: mul r1.zw, r1.xxxy, cb0[9].xxxx
    r1.zw = ((r1.xxxy)*(source[9].xxxx)).zw;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 6: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.zw, r1.zzzw, cb0[9].yyyy
    r1.zw = ((r1.zzzw)*(source[9].yyyy)).zw;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: mul r0.w, r2.z, cb0[11].y
    r0.w = ((r2.zzzz)*(source[11].yyyy)).w;
    // 12: mad r1.zw, cb0[8].wwww, r1.xxxy, r1.zzzw
    r1.zw = ((source[8].wwww)*(r1.xxxy)+(r1.zzzw)).zw;
    // 13: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 14: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 15: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 16: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 17: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 19: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 21: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 26: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 29: mul r6.xyz, r2.zxyz, r5.yzxy
    r6.xyz = ((r2.zxyz)*(r5.yzxy)).xyz;
    // 30: mad r6.xyz, r2.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r2.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 31: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r4.x, r5.xyzx, r1.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r4.xy, v4.xyxx, cb0[10].zzzz
    r4.xy = ((v4.xyxx)*(source[10].zzzz)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r4.w, r4.w
    r0.z = ((r4.wwww)*(r4.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[9].z, l(0.000000)
    r0.z = (max(source[9].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 48: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 51: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 55: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 57: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 60: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 61: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 62: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 63: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 64: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 65: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 66: dp3 r8.x, r5.xyzx, r7.xyzx
    r8.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 67: dp3 r5.x, r5.xyzx, r1.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 68: dp3 r8.y, r6.xyzx, r7.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 69: dp3 r5.y, r6.xyzx, r1.xyzx
    r5.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 70: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 71: mad r0.yz, cb0[10].wwww, r8.xxyx, r0.yyzy
    r0.yz = ((source[10].wwww)*(r8.xxyx)+(r0.yyzy)).yz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r6.xyz, r6.xyzx, cb0[4].xyzx
    r6.xyz = ((r6.xyzx)*(source[4].xyzx)).xyz;
    // 74: mad r6.xyz, cb0[11].xxxx, r6.xyzx, r6.xyzx
    r6.xyz = ((source[11].xxxx)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 75: add r6.xyz, r6.xyzx, -cb0[11].xxxx
    r6.xyz = ((r6.xyzx)+(-(source[11].xxxx))).xyz;
    // 76: mov_sat r8.xyz, r6.xyzx
    r8.xyz = (saturate(r6.xyzx)).xyz;
    // 77: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 78: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 80: add r9.xyz, -r3.xyzx, r0.yyyy
    r9.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 81: mad r9.xyz, cb0[11].wwww, r9.xyzx, r3.xyzx
    r9.xyz = ((source[11].wwww)*(r9.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r10.xyz, cb0[5].xyzx, cb0[12].xxxx
    r10.xyz = ((source[5].xyzx)*(source[12].xxxx)).xyz;
    // 83: mul r9.xyz, r9.xyzx, r10.xyzx
    r9.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 84: mad r0.yzw, r0.wwww, r8.xxyz, r9.xxyz
    r0.yzw = ((r0.wwww)*(r8.xxyz)+(r9.xxyz)).yzw;
    // 85: mul r0.yzw, r6.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)*(r0.yyzw)).yzw;
    // 86: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 87: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 88: mul r6.xyz, cb0[6].xyzx, cb0[12].yyyy
    r6.xyz = ((source[6].xyzx)*(source[12].yyyy)).xyz;
    // 89: mul r8.xyz, r4.xyzx, r6.xyzx
    r8.xyz = ((r4.xyzx)*(r6.xyzx)).xyz;
    // 90: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: mad r6.xyz, -r6.xyzx, r4.xyzx, r1.wwww
    r6.xyz = ((-(r6.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 92: mad r6.xyz, cb0[12].wwww, r6.xyzx, r8.xyzx
    r6.xyz = ((source[12].wwww)*(r6.xyzx)+(r8.xyzx)).xyz;
    // 93: add r6.xyz, -r0.yzwy, r6.xyzx
    r6.xyz = ((-(r0.yzwy))+(r6.xyzx)).xyz;
    // 94: mad r0.yzw, r0.xxxx, r6.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r6.xxyz)+(r0.yyzw)).yzw;
    // 95: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 96: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 97: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 98: mul r6.xyz, r1.wwww, v6.xyzx
    r6.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 99: dp3 r1.w, r6.xyzx, r1.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 100: mad r6.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r6.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 101: mul r6.xy, r6.xyxx, r6.xyxx
    r6.xy = ((r6.xyxx)*(r6.xyxx)).xy;
    // 102: mul r6.yzw, r6.yyyy, cb0[15].xxyz
    r6.yzw = ((r6.yyyy)*(source[15].xxyz)).yzw;
    // 103: mad r6.xyz, r6.xxxx, cb0[14].xyzx, r6.yzwy
    r6.xyz = ((r6.xxxx)*(source[14].xyzx)+(r6.yzwy)).xyz;
    // 104: mul r6.xyz, r6.xyzx, cb0[16].wwww
    r6.xyz = ((r6.xyzx)*(source[16].wwww)).xyz;
    // 105: mul r8.xyz, r0.yzwy, r6.xyzx
    r8.xyz = ((r0.yzwy)*(r6.xyzx)).xyz;
    // 106: dp2_sat r9.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 107: dp3_sat r9.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 108: dp3_sat r9.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 109: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 110: mul r1.xyz, r9.xyzx, r9.xyzx
    r1.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 111: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s5
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 112: mul r2.xyz, r2.xyzx, cb0[18].xyzx
    r2.xyz = ((r2.xyzx)*(source[18].xyzx)).xyz;
    // 113: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 114: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t5.wxyz, s5
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 115: mul r1.yzw, r1.yyzw, cb0[17].xxyz
    r1.yzw = ((r1.yyzw)*(source[17].xxyz)).yzw;
    // 116: mul r9.xyz, r1.xxxx, r1.yzwy
    r9.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 117: mad r6.xyz, r1.yzwy, r1.xxxx, r6.xyzx
    r6.xyz = ((r1.yzwy)*(r1.xxxx)+(r6.xyzx)).xyz;
    // 118: add r6.xyz, r6.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r6.xyz = ((r6.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 119: div r6.xyz, r9.xyzx, r6.xyzx
    r6.xyz = ((r9.xyzx)/(r6.xyzx)).xyz;
    // 120: mad r8.xyz, r0.yzwy, r9.xyzx, r8.xyzx
    r8.xyz = ((r0.yzwy)*(r9.xyzx)+(r8.xyzx)).xyz;
    // 121: dp3 r1.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 122: dp3 r2.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 123: add r6.xyz, -r4.xyzx, r2.wwww
    r6.xyz = ((-(r4.xyzx))+(r2.wwww)).xyz;
    // 124: mad r4.xyz, cb0[12].wwww, r6.xyzx, r4.xyzx
    r4.xyz = ((source[12].wwww)*(r6.xyzx)+(r4.xyzx)).xyz;
    // 125: mul r6.xyz, cb0[7].xyzx, cb0[13].xxxx
    r6.xyz = ((source[7].xyzx)*(source[13].xxxx)).xyz;
    // 126: mul r3.xyz, r3.xyzx, r6.xyzx
    r3.xyz = ((r3.xyzx)*(r6.xyzx)).xyz;
    // 127: mad r4.xyz, cb0[13].yyyy, r4.xyzx, -r3.xyzx
    r4.xyz = ((source[13].yyyy)*(r4.xyzx)+(-(r3.xyzx))).xyz;
    // 128: mad r3.xyz, r0.xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((r0.xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 129: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 130: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 131: dp2_sat r3.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 132: dp3_sat r3.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 133: dp3_sat r3.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 134: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 135: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: mul r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 137: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 138: dp3 r0.x, r2.xyzx, r3.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 139: mad r2.xyz, r1.yzwy, r0.xxxx, r8.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xxxx)+(r8.xyzx)).xyz;
    // 140: mul r1.yzw, r0.xxxx, r1.yyzw
    r1.yzw = ((r0.xxxx)*(r1.yyzw)).yzw;
    // 141: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 142: add r1.yzw, r2.xxyz, cb0[1].xxyz
    r1.yzw = ((r2.xxyz)+(source[1].xxyz)).yzw;
    // 143: mad o0.xyz, r0.yzwy, cb0[16].xyzx, r1.yzwy
    output.targets[0].xyz = ((r0.yzwy)*(source[16].xyzx)+(r1.yzwy)).xyz;
    // 144: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 145: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 146: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 147: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 148: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
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

// source.character.static-map-native-230.v1 / source program 454b0701566b1043b4b4ae633f81b11a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase230(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
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
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
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
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 9: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 10: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 11: mul r0.w, r2.z, cb0[10].y
    r0.w = ((r2.zzzz)*(source[10].yyyy)).w;
    // 12: mad r1.zw, cb0[7].wwww, r1.xxxy, r1.zzzw
    r1.zw = ((source[7].wwww)*(r1.xxxy)+(r1.zzzw)).zw;
    // 13: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 14: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 15: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 16: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 17: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 19: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 20: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 21: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 22: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 23: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 24: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 25: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 26: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 27: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 28: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 29: mul r6.xyz, r2.zxyz, r5.yzxy
    r6.xyz = ((r2.zxyz)*(r5.yzxy)).xyz;
    // 30: mad r6.xyz, r2.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r2.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 31: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 32: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 33: dp3 r4.x, r5.xyzx, r1.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 34: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 35: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 36: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 37: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 38: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 39: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 40: mul r4.xy, v4.xyxx, cb0[9].zzzz
    r4.xy = ((v4.xyxx)*(source[9].zzzz)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 42: mul r0.z, r4.w, r4.w
    r0.z = ((r4.wwww)*(r4.wwww)).z;
    // 43: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 44: max r0.z, cb0[8].z, l(0.000000)
    r0.z = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 45: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 46: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 47: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 48: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 49: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 50: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 51: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 52: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 53: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 54: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 55: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 57: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 60: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 61: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 62: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 63: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 64: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 65: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 66: dp3 r8.x, r5.xyzx, r7.xyzx
    r8.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 67: dp3 r8.y, r6.xyzx, r7.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 68: dp3 r6.y, r6.xyzx, r1.xyzx
    r6.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 69: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 70: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 71: mad r0.yz, cb0[9].wwww, r8.xxyx, r0.yyzy
    r0.yz = ((source[9].wwww)*(r8.xxyx)+(r0.yyzy)).yz;
    // 72: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 73: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 74: mad r5.xyz, cb0[10].xxxx, r5.xyzx, r5.xyzx
    r5.xyz = ((source[10].xxxx)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 75: add r5.xyz, r5.xyzx, -cb0[10].xxxx
    r5.xyz = ((r5.xyzx)+(-(source[10].xxxx))).xyz;
    // 76: mov_sat r7.xyz, r5.xyzx
    r7.xyz = (saturate(r5.xyzx)).xyz;
    // 77: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 78: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 80: add r8.xyz, -r3.xyzx, r0.yyyy
    r8.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 81: mad r3.xyz, cb0[10].wwww, r8.xyzx, r3.xyzx
    r3.xyz = ((source[10].wwww)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 82: mul r8.xyz, cb0[5].xyzx, cb0[11].xxxx
    r8.xyz = ((source[5].xyzx)*(source[11].xxxx)).xyz;
    // 83: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 84: mad r0.yzw, r0.wwww, r7.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r7.xxyz)+(r3.xxyz)).yzw;
    // 85: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 86: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 87: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 88: mul r3.xyz, cb0[6].xyzx, cb0[11].yyyy
    r3.xyz = ((source[6].xyzx)*(source[11].yyyy)).xyz;
    // 89: mul r5.xyz, r4.xyzx, r3.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 90: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 91: mad r3.xyz, -r3.xyzx, r4.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 92: mad r3.xyz, cb0[11].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[11].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 93: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 94: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 95: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 96: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 97: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 98: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 99: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 100: dp3 r6.z, r2.xyzx, r1.xyzx
    r6.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 101: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 102: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 103: mul r1.yzw, r1.yyyy, cb0[13].xxyz
    r1.yzw = ((r1.yyyy)*(source[13].xxyz)).yzw;
    // 104: mad r1.xyz, r1.xxxx, cb0[12].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[12].xyzx)+(r1.yzwy)).xyz;
    // 105: mul r1.xyz, r1.xyzx, cb0[14].wwww
    r1.xyz = ((r1.xyzx)*(source[14].wwww)).xyz;
    // 106: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 107: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 108: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 109: mad o0.xyz, r0.xyzx, cb0[14].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)+(r2.xyzx)).xyz;
    // 110: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 111: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 112: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 113: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 114: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 115: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 116: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 117: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 118: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 119: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 120: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 121: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 122: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 123: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 124: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 125: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 126: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 127: ret
    return output;
}

// source.character.static-map-native-231.v1 / source program dcfc4fa15835f142a067ea1f948bd88b
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked231(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[18]=1.f; source[19]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, r1.xyxx, cb0[8].wwww
    r1.xy = ((r1.xyxx)*(source[8].wwww)).xy;
    // 5: mad r1.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, 1.000000), l(0.000000, 0.000000, 0.500000, -0.500000)
    r1.zw = ((v4.yyyx)*(float4(0.000000,0.000000,-1.000000,1.000000))+(float4(0.000000,0.000000,0.500000,-0.500000))).zw;
    // 6: mad r1.xy, cb0[8].zzzz, r1.zwzz, r1.xyxx
    r1.xy = ((source[8].zzzz)*(r1.zwzz)+(r1.xyxx)).xy;
    // 7: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 9: mul r1.zw, r1.xxxy, cb0[10].xxxx
    r1.zw = ((r1.xxxy)*(source[10].xxxx)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 11: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 12: mul r1.zw, r1.zzzw, cb0[10].yyyy
    r1.zw = ((r1.zzzw)*(source[10].yyyy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 14: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: mul r0.w, r2.z, cb0[12].y
    r0.w = ((r2.zzzz)*(source[12].yyyy)).w;
    // 16: mad r1.zw, cb0[9].wwww, r2.xxxy, r1.zzzw
    r1.zw = ((source[9].wwww)*(r2.xxxy)+(r1.zzzw)).zw;
    // 17: dp2 r2.x, r2.xyxx, r2.xyxx
    r2.x = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).x;
    // 18: add r2.x, -r2.x, l(1.000000)
    r2.x = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 19: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 20: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 21: add r2.z, r2.x, l(0.000010)
    r2.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 22: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 23: dp3 r1.z, r2.xyzx, r2.xyzx
    r1.z = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 24: sqrt r1.z, r1.z
    r1.z = (sqrt(r1.zzzz)).z;
    // 25: div r2.xyz, r2.xyzx, r1.zzzz
    r2.xyz = ((r2.xyzx)/(r1.zzzz)).xyz;
    // 26: dp3 r1.z, v1.xyzx, v1.xyzx
    r1.z = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).z;
    // 27: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 28: mul r3.xyz, r1.zzzz, v1.xyzx
    r3.xyz = ((r1.zzzz)*(v1.xyzx)).xyz;
    // 29: dp3 r4.z, r3.xyzx, r2.xyzx
    r4.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 30: dp3 r1.z, v0.xyzx, v0.xyzx
    r1.z = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).z;
    // 31: rsq r1.z, r1.z
    r1.z = (rsqrt(r1.zzzz)).z;
    // 32: mul r5.xyz, r1.zzzz, v0.xyzx
    r5.xyz = ((r1.zzzz)*(v0.xyzx)).xyz;
    // 33: dp3 r4.x, r5.xyzx, r2.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 34: mul r6.xyz, r3.zxyz, r5.yzxy
    r6.xyz = ((r3.zxyz)*(r5.yzxy)).xyz;
    // 35: mad r6.xyz, r3.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r3.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 36: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 37: dp3 r4.y, r6.xyzx, r2.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 38: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 39: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 40: mad r0.x, r0.x, l(0.500000), cb0[11].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[11].yyyy)).x;
    // 41: mul r0.y, r2.z, r2.z
    r0.y = ((r2.zzzz)*(r2.zzzz)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 44: mul_sat r0.y, r0.y, r4.w
    r0.y = (saturate((r0.yyyy)*(r4.wwww))).y;
    // 45: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 46: mul r7.xy, v4.xyxx, cb0[11].zzzz
    r7.xy = ((v4.xyxx)*(source[11].zzzz)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 49: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 50: max r0.z, cb0[10].z, l(0.000000)
    r0.z = (max(source[10].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 51: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 52: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 53: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 54: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 55: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 56: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 57: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 58: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 59: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 60: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 61: add r8.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r2.xyz, r0.yyyy, r8.xyzx, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r8.xyzx)+(r2.xyzx)).xyz;
    // 63: dp3 r0.y, r2.xyzx, r2.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 64: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 65: mul r2.xyz, r0.yyyy, r2.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 66: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 67: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 68: mul r8.xyz, r0.yyyy, v5.xyzx
    r8.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 69: dp3 r0.y, r2.xyzx, r8.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 70: mul r9.xyz, r0.yyyy, r2.xyzx
    r9.xyz = ((r0.yyyy)*(r2.xyzx)).xyz;
    // 71: mad r8.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r8.xyzx
    r8.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r8.xyzx))).xyz;
    // 72: dp3 r9.x, r5.xyzx, r8.xyzx
    r9.x = (dot((r5.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 73: dp3 r5.x, r5.xyzx, r2.xyzx
    r5.x = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 74: dp3 r9.y, r6.xyzx, r8.xyzx
    r9.y = (dot((r6.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 75: dp3 r5.y, r6.xyzx, r2.xyzx
    r5.y = (dot((r6.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 76: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 77: mad r0.yz, cb0[11].wwww, r9.xxyx, r0.yyzy
    r0.yz = ((source[11].wwww)*(r9.xxyx)+(r0.yyzy)).yz;
    // 78: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 79: mul r6.xyz, r6.xyzx, cb0[4].xyzx
    r6.xyz = ((r6.xyzx)*(source[4].xyzx)).xyz;
    // 80: mad r6.xyz, cb0[12].xxxx, r6.xyzx, r6.xyzx
    r6.xyz = ((source[12].xxxx)*(r6.xyzx)+(r6.xyzx)).xyz;
    // 81: add r6.xyz, r6.xyzx, -cb0[12].xxxx
    r6.xyz = ((r6.xyzx)+(-(source[12].xxxx))).xyz;
    // 82: mov_sat r9.xyz, r6.xyzx
    r9.xyz = (saturate(r6.xyzx)).xyz;
    // 83: mov_sat r6.xyz, -r6.xyzx
    r6.xyz = (saturate(-(r6.xyzx))).xyz;
    // 84: mad r6.xyz, -r0.wwww, r6.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r0.wwww))*(r6.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 85: dp3 r0.y, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 86: add r10.xyz, -r4.xyzx, r0.yyyy
    r10.xyz = ((-(r4.xyzx))+(r0.yyyy)).xyz;
    // 87: mad r4.xyz, cb0[12].wwww, r10.xyzx, r4.xyzx
    r4.xyz = ((source[12].wwww)*(r10.xyzx)+(r4.xyzx)).xyz;
    // 88: mul r10.xyz, cb0[5].xyzx, cb0[13].xxxx
    r10.xyz = ((source[5].xyzx)*(source[13].xxxx)).xyz;
    // 89: mul r4.xyz, r4.xyzx, r10.xyzx
    r4.xyz = ((r4.xyzx)*(r10.xyzx)).xyz;
    // 90: mad r0.yzw, r0.wwww, r9.xxyz, r4.xxyz
    r0.yzw = ((r0.wwww)*(r9.xxyz)+(r4.xxyz)).yzw;
    // 91: mul r0.yzw, r6.xxyz, r0.yyzw
    r0.yzw = ((r6.xxyz)*(r0.yyzw)).yzw;
    // 92: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 93: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 94: mul r4.xyz, cb0[6].xyzx, cb0[13].yyyy
    r4.xyz = ((source[6].xyzx)*(source[13].yyyy)).xyz;
    // 95: mul r6.xyz, r7.xyzx, r4.xyzx
    r6.xyz = ((r7.xyzx)*(r4.xyzx)).xyz;
    // 96: dp3 r1.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 97: mad r4.xyz, -r4.xyzx, r7.xyzx, r1.wwww
    r4.xyz = ((-(r4.xyzx))*(r7.xyzx)+(r1.wwww)).xyz;
    // 98: mad r4.xyz, cb0[13].wwww, r4.xyzx, r6.xyzx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(r6.xyzx)).xyz;
    // 99: add r4.xyz, -r0.yzwy, r4.xyzx
    r4.xyz = ((-(r0.yzwy))+(r4.xyzx)).xyz;
    // 100: mad r0.yzw, r0.xxxx, r4.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r4.xxyz)+(r0.yyzw)).yzw;
    // 101: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 102: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 103: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 104: mul r4.xyz, r1.wwww, v6.xyzx
    r4.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 105: dp3 r1.w, r4.xyzx, r2.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 106: mad r4.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mul r4.xy, r4.xyxx, r4.xyxx
    r4.xy = ((r4.xyxx)*(r4.xyxx)).xy;
    // 108: mul r4.yzw, r4.yyyy, cb0[16].xxyz
    r4.yzw = ((r4.yyyy)*(source[16].xxyz)).yzw;
    // 109: mad r4.xyz, r4.xxxx, cb0[15].xyzx, r4.yzwy
    r4.xyz = ((r4.xxxx)*(source[15].xyzx)+(r4.yzwy)).xyz;
    // 110: mul r4.xyz, r4.xyzx, cb0[17].wwww
    r4.xyz = ((r4.xyzx)*(source[17].wwww)).xyz;
    // 111: mul r6.xyz, r0.yzwy, r4.xyzx
    r6.xyz = ((r0.yzwy)*(r4.xyzx)).xyz;
    // 112: dp2_sat r9.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r9.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 113: dp3_sat r9.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r9.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 114: dp3_sat r9.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r9.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 115: dp3 r5.z, r3.xyzx, r2.xyzx
    r5.z = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 116: mul r2.xyz, r9.xyzx, r9.xyzx
    r2.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 117: sample_indexable(texture2d)(float,float,float,float) r3.xyz, v3.zwzz, t7.xyzw, s6
    r3.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 118: mul r3.xyz, r3.xyzx, cb0[19].xyzx
    r3.xyz = ((r3.xyzx)*(source[19].xyzx)).xyz;
    // 119: dp3 r1.w, r3.xyzx, r2.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 120: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s6
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 121: mul r2.xyz, r2.xyzx, cb0[18].xyzx
    r2.xyz = ((r2.xyzx)*(source[18].xyzx)).xyz;
    // 122: mul r9.xyz, r1.wwww, r2.xyzx
    r9.xyz = ((r1.wwww)*(r2.xyzx)).xyz;
    // 123: mad r4.xyz, r2.xyzx, r1.wwww, r4.xyzx
    r4.xyz = ((r2.xyzx)*(r1.wwww)+(r4.xyzx)).xyz;
    // 124: add r4.xyz, r4.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r4.xyz = ((r4.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 125: div r4.xyz, r9.xyzx, r4.xyzx
    r4.xyz = ((r9.xyzx)/(r4.xyzx)).xyz;
    // 126: mad r6.xyz, r0.yzwy, r9.xyzx, r6.xyzx
    r6.xyz = ((r0.yzwy)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 127: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 128: dp3 r2.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 129: add r4.xyz, -r7.xyzx, r2.wwww
    r4.xyz = ((-(r7.xyzx))+(r2.wwww)).xyz;
    // 130: mad r4.xyz, cb0[13].wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((source[13].wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 131: mul r7.xyz, cb0[7].xyzx, cb0[14].xxxx
    r7.xyz = ((source[7].xyzx)*(source[14].xxxx)).xyz;
    // 132: mul r1.xyz, r1.xyzx, r7.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)).xyz;
    // 133: mad r4.xyz, cb0[14].yyyy, r4.xyzx, -r1.xyzx
    r4.xyz = ((source[14].yyyy)*(r4.xyzx)+(-(r1.xyzx))).xyz;
    // 134: mad r1.xyz, r0.xxxx, r4.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 135: mad r1.xyz, r1.xyzx, cb2[4].wwww, cb2[4].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 136: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 137: dp2_sat r2.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r2.x = (saturate(dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 138: dp3_sat r2.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r2.y = (saturate(dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 139: dp3_sat r2.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r2.z = (saturate(dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 140: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 141: add r0.x, cb0[14].z, l(1.000000)
    r0.x = ((source[14].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 142: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 143: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 144: dp3 r0.x, r3.xyzx, r2.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 145: mad r2.xyz, r1.xyzx, r0.xxxx, r6.xyzx
    r2.xyz = ((r1.xyzx)*(r0.xxxx)+(r6.xyzx)).xyz;
    // 146: mul r1.xyz, r0.xxxx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)).xyz;
    // 147: dp3 o4.x, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 148: add r1.xyz, r2.xyzx, cb0[1].xyzx
    r1.xyz = ((r2.xyzx)+(source[1].xyzx)).xyz;
    // 149: mad o0.xyz, r0.yzwy, cb0[17].xyzx, r1.xyzx
    output.targets[0].xyz = ((r0.yzwy)*(source[17].xyzx)+(r1.xyzx)).xyz;
    // 150: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 151: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 152: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 153: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 154: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 155: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 156: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 157: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 158: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 159: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 160: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 161: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 162: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 163: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 164: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 165: mul o4.z, r1.w, r2.x
    output.targets[4].z = ((r1.wwww)*(r2.xxxx)).z;
    // 166: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 167: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 168: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 169: ret
    return output;
}

// source.character.static-map-native-231.v1 / source program a044e1a448e4864bbd7538035fee6547
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase231(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
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
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: add r1.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 4: mul r1.xy, r1.xyxx, cb0[7].wwww
    r1.xy = ((r1.xyxx)*(source[7].wwww)).xy;
    // 5: mad r1.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, 1.000000), l(0.000000, 0.000000, 0.500000, -0.500000)
    r1.zw = ((v4.yyyx)*(float4(0.000000,0.000000,-1.000000,1.000000))+(float4(0.000000,0.000000,0.500000,-0.500000))).zw;
    // 6: mad r1.xy, cb0[7].zzzz, r1.zwzz, r1.xyxx
    r1.xy = ((source[7].zzzz)*(r1.zwzz)+(r1.xyxx)).xy;
    // 7: add r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 8: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 9: mul r1.zw, r1.xxxy, cb0[9].xxxx
    r1.zw = ((r1.xxxy)*(source[9].xxxx)).zw;
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r1.zw, r1.zwzz, t1.zwxy, s1, l(0.000000)
    r1.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 11: mad r1.zw, r1.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((r1.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 12: mul r1.zw, r1.zzzw, cb0[9].yyyy
    r1.zw = ((r1.zzzw)*(source[9].yyyy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r1.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 14: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.xyxx, t2.xyzw, s2, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 15: mad r1.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mul r0.w, r2.z, cb0[11].y
    r0.w = ((r2.zzzz)*(source[11].yyyy)).w;
    // 17: mad r1.zw, cb0[8].wwww, r1.xxxy, r1.zzzw
    r1.zw = ((source[8].wwww)*(r1.xxxy)+(r1.zzzw)).zw;
    // 18: dp2 r1.x, r1.xyxx, r1.xyxx
    r1.x = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 19: add r1.x, -r1.x, l(1.000000)
    r1.x = ((-(r1.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 20: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 21: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 22: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r2.xy, r1.zwzz, v2.wwww
    r2.xy = ((r1.zwzz)*(v2.wwww)).xy;
    // 24: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 25: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 26: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 27: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 28: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 29: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 30: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 31: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 32: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 33: mul r5.xyz, r1.wwww, v0.xyzx
    r5.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 34: dp3 r4.x, r5.xyzx, r1.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 35: mul r6.xyz, r2.zxyz, r5.yzxy
    r6.xyz = ((r2.zxyz)*(r5.yzxy)).xyz;
    // 36: mad r6.xyz, r2.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r2.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 37: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 38: dp3 r4.y, r6.xyzx, r1.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 39: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 40: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 41: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 42: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 43: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 44: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 45: mul r4.xy, v4.xyxx, cb0[10].zzzz
    r4.xy = ((v4.xyxx)*(source[10].zzzz)).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r4.xyxx, t3.xyzw, s3, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r0.z, r4.w, r4.w
    r0.z = ((r4.wwww)*(r4.wwww)).z;
    // 48: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 49: max r0.z, cb0[9].z, l(0.000000)
    r0.z = (max(source[9].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 50: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 51: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 52: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 53: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 54: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 55: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 56: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 57: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 58: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 59: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 60: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 62: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 63: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 64: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 65: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 68: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 69: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 70: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 71: dp3 r8.x, r5.xyzx, r7.xyzx
    r8.x = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 72: dp3 r8.y, r6.xyzx, r7.xyzx
    r8.y = (dot((r6.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 73: dp3 r6.y, r6.xyzx, r1.xyzx
    r6.y = (dot((r6.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 74: dp3 r6.x, r5.xyzx, r1.xyzx
    r6.x = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 75: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 76: mad r0.yz, cb0[10].wwww, r8.xxyx, r0.yyzy
    r0.yz = ((source[10].wwww)*(r8.xxyx)+(r0.yyzy)).yz;
    // 77: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 78: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 79: mad r5.xyz, cb0[11].xxxx, r5.xyzx, r5.xyzx
    r5.xyz = ((source[11].xxxx)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 80: add r5.xyz, r5.xyzx, -cb0[11].xxxx
    r5.xyz = ((r5.xyzx)+(-(source[11].xxxx))).xyz;
    // 81: mov_sat r7.xyz, r5.xyzx
    r7.xyz = (saturate(r5.xyzx)).xyz;
    // 82: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 83: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 85: add r8.xyz, -r3.xyzx, r0.yyyy
    r8.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 86: mad r3.xyz, cb0[11].wwww, r8.xyzx, r3.xyzx
    r3.xyz = ((source[11].wwww)*(r8.xyzx)+(r3.xyzx)).xyz;
    // 87: mul r8.xyz, cb0[5].xyzx, cb0[12].xxxx
    r8.xyz = ((source[5].xyzx)*(source[12].xxxx)).xyz;
    // 88: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 89: mad r0.yzw, r0.wwww, r7.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r7.xxyz)+(r3.xxyz)).yzw;
    // 90: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 91: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 92: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 93: mul r3.xyz, cb0[6].xyzx, cb0[12].yyyy
    r3.xyz = ((source[6].xyzx)*(source[12].yyyy)).xyz;
    // 94: mul r5.xyz, r4.xyzx, r3.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)).xyz;
    // 95: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 96: mad r3.xyz, -r3.xyzx, r4.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r4.xyzx)+(r1.wwww)).xyz;
    // 97: mad r3.xyz, cb0[12].wwww, r3.xyzx, r5.xyzx
    r3.xyz = ((source[12].wwww)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 98: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 99: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 100: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 101: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 102: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 103: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 104: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 105: dp3 r6.z, r2.xyzx, r1.xyzx
    r6.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 106: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 107: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 108: mul r1.yzw, r1.yyyy, cb0[14].xxyz
    r1.yzw = ((r1.yyyy)*(source[14].xxyz)).yzw;
    // 109: mad r1.xyz, r1.xxxx, cb0[13].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[13].xyzx)+(r1.yzwy)).xyz;
    // 110: mul r1.xyz, r1.xyzx, cb0[15].wwww
    r1.xyz = ((r1.xyzx)*(source[15].wwww)).xyz;
    // 111: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 112: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 113: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 114: mad o0.xyz, r0.xyzx, cb0[15].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[15].xyzx)+(r2.xyzx)).xyz;
    // 115: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 116: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 117: dp3 r0.x, r6.xyzx, r6.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 118: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 119: mul r0.xyz, r0.xxxx, r6.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)).xyz;
    // 120: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 121: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 122: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 123: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 124: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 125: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 126: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 127: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 128: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 129: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 130: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 131: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 132: ret
    return output;
}

// source.character.static-map-native-232.v1 / source program 4198b8e9b1772c41af960e9de0068cfd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked232(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[15]=1.f; source[16]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[8].yyyy
    r0.xy = ((v4.xyxx)*(source[8].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mul r0.xy, r0.xyxx, cb0[8].zzzz
    r0.xy = ((r0.xyxx)*(source[8].zzzz)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 6: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.x, r1.z, cb0[10].x
    r1.x = ((r1.zzzz)*(source[10].xxxx)).x;
    // 8: mad r0.xy, cb0[8].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[8].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 9: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 10: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 12: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 13: add r2.z, r0.z, l(0.000010)
    r2.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r0.xyxx, v2.wwww
    r2.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 15: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 16: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 17: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 18: add r1.yzw, -r0.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r1.yzw = ((-(r0.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 19: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 21: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 22: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: mul r3.xy, v4.xyxx, cb0[9].xxxx
    r3.xy = ((v4.xyxx)*(source[9].xxxx)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 25: mul r2.w, r3.w, r3.w
    r2.w = ((r3.wwww)*(r3.wwww)).w;
    // 26: mul r0.w, r0.w, r2.w
    r0.w = ((r0.wwww)*(r2.wwww)).w;
    // 27: max r2.w, cb0[8].w, l(0.000000)
    r2.w = (max(source[8].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 28: min r2.w, r2.w, l(0.990000)
    r2.w = (min(r2.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 29: mul r3.w, r0.w, r2.w
    r3.w = ((r0.wwww)*(r2.wwww)).w;
    // 30: add r4.x, -v2.x, l(1.000000)
    r4.x = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 31: mad r3.w, r4.x, r3.w, r4.x
    r3.w = ((r4.xxxx)*(r3.wwww)+(r4.xxxx)).w;
    // 32: add r4.x, -r2.w, r3.w
    r4.x = ((-(r2.wwww))+(r3.wwww)).x;
    // 33: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 35: mad r3.w, -r2.w, r4.x, r3.w
    r3.w = ((-(r2.wwww))*(r4.xxxx)+(r3.wwww)).w;
    // 36: mul r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)*(r2.wwww)).w;
    // 37: mad_sat r0.w, r0.w, r3.w, r2.w
    r0.w = (saturate((r0.wwww)*(r3.wwww)+(r2.wwww))).w;
    // 38: mul r2.w, r0.w, l(0.650000)
    r2.w = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 39: mad r0.xyz, r2.wwww, r1.yzwy, r0.xyzx
    r0.xyz = ((r2.wwww)*(r1.yzwy)+(r0.xyzx)).xyz;
    // 40: dp3 r1.y, r0.xyzx, r0.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 41: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 42: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 43: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 44: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 45: mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // 46: dp3 r2.w, r0.xyzx, r1.yzwy
    r2.w = (dot((r0.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 47: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 48: mad r1.yzw, r4.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), -r1.yyzw
    r1.yzw = ((r4.xxyz)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r1.yyzw))).yzw;
    // 49: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 52: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 55: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 56: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 57: dp3 r4.z, r4.xyzx, r0.xyzx
    r4.z = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 58: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 59: dp3 r7.y, r6.xyzx, r1.yzwy
    r7.y = (dot((r6.xyzx).xyz,(r1.yzwy).xyz).xxxx).y;
    // 60: dp3 r4.y, r6.xyzx, r0.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 61: dp3 r7.x, r5.xyzx, r1.yzwy
    r7.x = (dot((r5.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 62: dp3 r4.x, r5.xyzx, r0.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 63: mul r5.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r5.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 64: mad r5.xy, cb0[9].zzzz, r7.xyxx, r5.xyxx
    r5.xy = ((source[9].zzzz)*(r7.xyxx)+(r5.xyxx)).xy;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 66: mul r5.xyz, r5.xyzx, cb0[4].xyzx
    r5.xyz = ((r5.xyzx)*(source[4].xyzx)).xyz;
    // 67: mad r5.xyz, cb0[9].wwww, r5.xyzx, r5.xyzx
    r5.xyz = ((source[9].wwww)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 68: add r5.xyz, r5.xyzx, -cb0[9].wwww
    r5.xyz = ((r5.xyzx)+(-(source[9].wwww))).xyz;
    // 69: mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // 70: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 71: mad r5.xyz, -r1.xxxx, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r1.xxxx))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 72: mul r7.xyz, cb0[5].xyzx, cb0[10].yyyy
    r7.xyz = ((source[5].xyzx)*(source[10].yyyy)).xyz;
    // 73: mul r2.xyz, r2.xyzx, r7.xyzx
    r2.xyz = ((r2.xyzx)*(r7.xyzx)).xyz;
    // 74: mad r2.xyz, r1.xxxx, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 75: mul r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)*(r2.xyzx)).xyz;
    // 76: max r2.xyz, r2.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xyz = (max(r2.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 77: min r2.xyz, r2.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r2.xyz = (min(r2.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 78: mul r5.xyz, cb0[6].xyzx, cb0[10].zzzz
    r5.xyz = ((source[6].xyzx)*(source[10].zzzz)).xyz;
    // 79: mul r6.xyz, r3.xyzx, r5.xyzx
    r6.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 80: dp3 r1.x, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 81: mad r5.xyz, -r5.xyzx, r3.xyzx, r1.xxxx
    r5.xyz = ((-(r5.xyzx))*(r3.xyzx)+(r1.xxxx)).xyz;
    // 82: mad r5.xyz, cb0[11].xxxx, r5.xyzx, r6.xyzx
    r5.xyz = ((source[11].xxxx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 83: add r5.xyz, -r2.xyzx, r5.xyzx
    r5.xyz = ((-(r2.xyzx))+(r5.xyzx)).xyz;
    // 84: mad r2.xyz, r0.wwww, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.wwww)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 85: mad r2.xyz, r2.xyzx, cb2[3].wwww, cb2[3].xyzx
    r2.xyz = ((r2.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 86: dp3 r1.x, v6.xyzx, v6.xyzx
    r1.x = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).x;
    // 87: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 88: mul r5.xyz, r1.xxxx, v6.xyzx
    r5.xyz = ((r1.xxxx)*(v6.xyzx)).xyz;
    // 89: dp3 r1.x, r5.xyzx, r0.xyzx
    r1.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 90: mad r5.xy, r1.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r1.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 91: mul r5.xy, r5.xyxx, r5.xyxx
    r5.xy = ((r5.xyxx)*(r5.xyxx)).xy;
    // 92: mul r5.yzw, r5.yyyy, cb0[13].xxyz
    r5.yzw = ((r5.yyyy)*(source[13].xxyz)).yzw;
    // 93: mad r5.xyz, r5.xxxx, cb0[12].xyzx, r5.yzwy
    r5.xyz = ((r5.xxxx)*(source[12].xyzx)+(r5.yzwy)).xyz;
    // 94: mul r5.xyz, r5.xyzx, cb0[14].wwww
    r5.xyz = ((r5.xyzx)*(source[14].wwww)).xyz;
    // 95: mul r6.xyz, r2.xyzx, r5.xyzx
    r6.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 96: dp2_sat r7.x, r0.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r7.x = (saturate(dot((r0.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 97: dp3_sat r7.y, r0.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r7.y = (saturate(dot((r0.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 98: dp3_sat r7.z, r0.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r7.z = (saturate(dot((r0.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 99: mul r0.xyz, r7.xyzx, r7.xyzx
    r0.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 100: sample_indexable(texture2d)(float,float,float,float) r7.xyz, v3.zwzz, t8.xyzw, s7
    r7.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 101: mul r7.xyz, r7.xyzx, cb0[16].xyzx
    r7.xyz = ((r7.xyzx)*(source[16].xyzx)).xyz;
    // 102: dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 103: sample_indexable(texture2d)(float,float,float,float) r8.xyz, v3.zwzz, t7.xyzw, s7
    r8.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 104: mul r8.xyz, r8.xyzx, cb0[15].xyzx
    r8.xyz = ((r8.xyzx)*(source[15].xyzx)).xyz;
    // 105: mul r9.xyz, r0.xxxx, r8.xyzx
    r9.xyz = ((r0.xxxx)*(r8.xyzx)).xyz;
    // 106: mad r0.xyz, r8.xyzx, r0.xxxx, r5.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xxxx)+(r5.xyzx)).xyz;
    // 107: add r0.xyz, r0.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r0.xyz = ((r0.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 108: div r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)/(r0.xyzx)).xyz;
    // 109: mad r5.xyz, r2.xyzx, r9.xyzx, r6.xyzx
    r5.xyz = ((r2.xyzx)*(r9.xyzx)+(r6.xyzx)).xyz;
    // 110: dp3 r0.x, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 111: dp3 r0.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 112: add r6.xyz, -r3.xyzx, r0.yyyy
    r6.xyz = ((-(r3.xyzx))+(r0.yyyy)).xyz;
    // 113: mad r3.xyz, cb0[11].xxxx, r6.xyzx, r3.xyzx
    r3.xyz = ((source[11].xxxx)*(r6.xyzx)+(r3.xyzx)).xyz;
    // 114: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t5.xyzw, s6, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 115: mul r9.xyz, cb0[7].xyzx, cb0[11].yyyy
    r9.xyz = ((source[7].xyzx)*(source[11].yyyy)).xyz;
    // 116: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 117: mad r3.xyz, cb0[11].zzzz, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[11].zzzz)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 118: mad r0.yzw, r0.wwww, r3.xxyz, r6.xxyz
    r0.yzw = ((r0.wwww)*(r3.xxyz)+(r6.xxyz)).yzw;
    // 119: mad r0.yzw, r0.yyzw, cb2[4].wwww, cb2[4].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[4].wwww)+(passValues[4].xxyz)).yzw;
    // 120: mul r0.yzw, r8.xxyz, r0.yyzw
    r0.yzw = ((r8.xxyz)*(r0.yyzw)).yzw;
    // 121: dp2_sat r3.x, r1.zwzz, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r1.zwzz).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 122: dp3_sat r3.y, r1.yzwy, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r1.yzwy).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 123: dp3_sat r3.z, r1.yzwy, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r1.yzwy).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 124: log r1.xyz, r3.xyzx
    r1.xyz = (log2(r3.xyzx)).xyz;
    // 125: add r1.w, cb0[11].w, l(1.000000)
    r1.w = ((source[11].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 127: exp r1.xyz, r1.xyzx
    r1.xyz = (exp2(r1.xyzx)).xyz;
    // 128: dp3 r1.x, r7.xyzx, r1.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 129: mad r1.yzw, r0.yyzw, r1.xxxx, r5.xxyz
    r1.yzw = ((r0.yyzw)*(r1.xxxx)+(r5.xxyz)).yzw;
    // 130: mul r0.yzw, r0.yyzw, r1.xxxx
    r0.yzw = ((r0.yyzw)*(r1.xxxx)).yzw;
    // 131: dp3 o4.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 132: mul r0.yz, v4.xxyx, cb0[2].xxyx
    r0.yz = ((v4.xxyx)*(source[2].xxyx)).yz;
    // 133: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t6.wxyz, s4, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 134: mul r3.xyz, cb0[3].xyzx, cb0[9].yyyy
    r3.xyz = ((source[3].xyzx)*(source[9].yyyy)).xyz;
    // 135: mad r0.yzw, r0.yyzw, r3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(r3.xxyz)+(source[1].xxyz)).yzw;
    // 136: add r0.yzw, r1.yyzw, r0.yyzw
    r0.yzw = ((r1.yyzw)+(r0.yyzw)).yzw;
    // 137: mad o0.xyz, r2.xyzx, cb0[14].xyzx, r0.yzwy
    output.targets[0].xyz = ((r2.xyzx)*(source[14].xyzx)+(r0.yzwy)).xyz;
    // 138: mov o3.xyz, r2.xyzx
    output.targets[3].xyz = (r2.xyzx).xyz;
    // 139: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 140: dp3 r0.y, r4.xyzx, r4.xyzx
    r0.y = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 141: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 142: mul r0.yzw, r0.yyyy, r4.xxyz
    r0.yzw = ((r0.yyyy)*(r4.xxyz)).yzw;
    // 143: ge r1.x, l(0.000000), r0.w
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.wwww)) * 0xffffffffu)).x;
    // 144: dp3 r0.w, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.yzwy|
    r0.w = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.yzwy)).xyz).xxxx).w;
    // 145: div r0.yz, r0.yyzy, r0.wwww
    r0.yz = ((r0.yyzy)/(r0.wwww)).yz;
    // 146: ge r2.xy, r0.yzyy, l(0.000000, 0.000000, 0.000000, 0.000000)
    r2.xy = (asfloat((uint4)((r0.yzyy)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 147: movc r2.xy, r2.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 148: mad r2.xy, -|r0.zyzz|, r2.xyxx, r2.xyxx
    r2.xy = ((-(abs(r0.zyzz)))*(r2.xyxx)+(r2.xyxx)).xy;
    // 149: movc r0.yz, r1.xxxx, r2.xxyx, r0.yyzy
    r0.yz = ((asuint(r1.xxxx) != 0u) ? (r2.xxyx) : (r0.yyzy)).yz;
    // 150: mad o2.xy, r0.yzyy, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.yzyy)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 151: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 152: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 153: mul o4.z, r0.x, r1.y
    output.targets[4].z = ((r0.xxxx)*(r1.yyyy)).z;
    // 154: dp3 o4.y, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 155: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 156: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 157: ret
    return output;
}

// source.character.static-map-native-232.v1 / source program 7aaead06c66ede469e1e32c8c75562ca
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase232(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[7].x=g_SourceCharacterBaseConstants[8].x;
    source[7].y=g_SourceCharacterBaseConstants[8].y;
    source[7].z=g_SourceCharacterBaseConstants[8].z;
    source[7].w=g_SourceCharacterBaseConstants[8].w;
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
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[7].yyyy
    r0.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t1.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 3: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 4: mul r0.xy, r0.xyxx, cb0[7].zzzz
    r0.xy = ((r0.xyxx)*(source[7].zzzz)).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 6: mad r0.zw, r1.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r1.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 7: mul r1.x, r1.z, cb0[9].x
    r1.x = ((r1.zzzz)*(source[9].xxxx)).x;
    // 8: mad r0.xy, cb0[7].xxxx, r0.zwzz, r0.xyxx
    r0.xy = ((source[7].xxxx)*(r0.zwzz)+(r0.xyxx)).xy;
    // 9: dp2 r0.z, r0.zwzz, r0.zwzz
    r0.z = (dot((r0.zwzz).xy,(r0.zwzz).xy).xxxx).z;
    // 10: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: max r0.z, r0.z, l(0.000000)
    r0.z = (max(r0.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 12: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 13: add r2.z, r0.z, l(0.000010)
    r2.z = ((r0.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 14: mul r2.xy, r0.xyxx, v2.wwww
    r2.xy = ((r0.xyxx)*(v2.wwww)).xy;
    // 15: dp3 r0.x, r2.xyzx, r2.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 16: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 17: div r0.xyz, r2.xyzx, r0.xxxx
    r0.xyz = ((r2.xyzx)/(r0.xxxx)).xyz;
    // 18: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 20: mul_sat r0.w, r0.w, r2.w
    r0.w = (saturate((r0.wwww)*(r2.wwww))).w;
    // 21: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: mul r1.yz, v4.xxyx, cb0[8].xxxx
    r1.yz = ((v4.xxyx)*(source[8].xxxx)).yz;
    // 23: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r1.yzyy, t3.xyzw, s3, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 24: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 25: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 26: max r1.y, cb0[7].w, l(0.000000)
    r1.y = (max(source[7].wwww,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 27: min r1.y, r1.y, l(0.990000)
    r1.y = (min(r1.yyyy,float4(0.990000,0.990000,0.990000,0.990000))).y;
    // 28: mul r1.z, r0.w, r1.y
    r1.z = ((r0.wwww)*(r1.yyyy)).z;
    // 29: add r1.w, -v2.x, l(1.000000)
    r1.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: mad r1.z, r1.w, r1.z, r1.w
    r1.z = ((r1.wwww)*(r1.zzzz)+(r1.wwww)).z;
    // 31: add r1.w, -r1.y, r1.z
    r1.w = ((-(r1.yyyy))+(r1.zzzz)).w;
    // 32: add r1.y, -r1.y, l(1.000000)
    r1.y = ((-(r1.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 33: div r1.y, l(1.000000, 1.000000, 1.000000, 1.000000), r1.y
    r1.y = (((r1.yyyy) != 0.f ? 1.f / (r1.yyyy) : 0.f)).y;
    // 34: mad r1.z, -r1.y, r1.w, r1.z
    r1.z = ((-(r1.yyyy))*(r1.wwww)+(r1.zzzz)).z;
    // 35: mul r1.y, r1.w, r1.y
    r1.y = ((r1.wwww)*(r1.yyyy)).y;
    // 36: mad_sat r0.w, r0.w, r1.z, r1.y
    r0.w = (saturate((r0.wwww)*(r1.zzzz)+(r1.yyyy))).w;
    // 37: mul r1.y, r0.w, l(0.650000)
    r1.y = ((r0.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 38: add r4.xyz, -r0.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 39: mad r0.xyz, r1.yyyy, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.yyyy)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 40: dp3 r1.y, r0.xyzx, r0.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 41: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 42: mul r0.xyz, r0.xyzx, r1.yyyy
    r0.xyz = ((r0.xyzx)*(r1.yyyy)).xyz;
    // 43: dp3 r1.y, v5.xyzx, v5.xyzx
    r1.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 44: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 45: mul r1.yzw, r1.yyyy, v5.xxyz
    r1.yzw = ((r1.yyyy)*(v5.xxyz)).yzw;
    // 46: dp3 r2.w, r0.xyzx, r1.yzwy
    r2.w = (dot((r0.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 47: mul r4.xyz, r0.xyzx, r2.wwww
    r4.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 48: mad r1.yzw, r4.xxyz, l(0.000000, 2.000000, 2.000000, 2.000000), -r1.yyzw
    r1.yzw = ((r4.xxyz)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r1.yyzw))).yzw;
    // 49: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 50: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 51: mul r4.xyz, r2.wwww, v1.xyzx
    r4.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 52: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 53: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 54: mul r5.xyz, r2.wwww, v0.xyzx
    r5.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 55: mul r6.xyz, r4.zxyz, r5.yzxy
    r6.xyz = ((r4.zxyz)*(r5.yzxy)).xyz;
    // 56: mad r6.xyz, r4.yzxy, r5.zxyz, -r6.xyzx
    r6.xyz = ((r4.yzxy)*(r5.zxyz)+(-(r6.xyzx))).xyz;
    // 57: dp3 r4.z, r4.xyzx, r0.xyzx
    r4.z = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).z;
    // 58: mul r6.xyz, r6.xyzx, v1.wwww
    r6.xyz = ((r6.xyzx)*(v1.wwww)).xyz;
    // 59: dp3 r7.y, r6.xyzx, r1.yzwy
    r7.y = (dot((r6.xyzx).xyz,(r1.yzwy).xyz).xxxx).y;
    // 60: dp3 r7.x, r5.xyzx, r1.yzwy
    r7.x = (dot((r5.xyzx).xyz,(r1.yzwy).xyz).xxxx).x;
    // 61: dp3 r4.x, r5.xyzx, r0.xyzx
    r4.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 62: dp3 r4.y, r6.xyzx, r0.xyzx
    r4.y = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).y;
    // 63: mul r1.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r1.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 64: mad r1.yz, cb0[8].zzzz, r7.xxyx, r1.yyzy
    r1.yz = ((source[8].zzzz)*(r7.xxyx)+(r1.yyzy)).yz;
    // 65: sample_b_indexable(texture2d)(float,float,float,float) r1.yzw, r1.yzyy, t4.wxyz, s5, l(0.000000)
    r1.yzw = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r1.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 66: mul r1.yzw, r1.yyzw, cb0[4].xxyz
    r1.yzw = ((r1.yyzw)*(source[4].xxyz)).yzw;
    // 67: mad r1.yzw, cb0[8].wwww, r1.yyzw, r1.yyzw
    r1.yzw = ((source[8].wwww)*(r1.yyzw)+(r1.yyzw)).yzw;
    // 68: add r1.yzw, r1.yyzw, -cb0[8].wwww
    r1.yzw = ((r1.yyzw)+(-(source[8].wwww))).yzw;
    // 69: mov_sat r5.xyz, r1.yzwy
    r5.xyz = (saturate(r1.yzwy)).xyz;
    // 70: mov_sat r1.yzw, -r1.yyzw
    r1.yzw = (saturate(-(r1.yyzw))).yzw;
    // 71: mad r1.yzw, -r1.xxxx, r1.yyzw, l(0.000000, 1.000000, 1.000000, 1.000000)
    r1.yzw = ((-(r1.xxxx))*(r1.yyzw)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 72: mul r6.xyz, cb0[5].xyzx, cb0[9].yyyy
    r6.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 73: mul r2.xyz, r2.xyzx, r6.xyzx
    r2.xyz = ((r2.xyzx)*(r6.xyzx)).xyz;
    // 74: mad r2.xyz, r1.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r1.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 75: mul r1.xyz, r1.yzwy, r2.xyzx
    r1.xyz = ((r1.yzwy)*(r2.xyzx)).xyz;
    // 76: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 77: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 78: mul r2.xyz, cb0[6].xyzx, cb0[9].zzzz
    r2.xyz = ((source[6].xyzx)*(source[9].zzzz)).xyz;
    // 79: mul r5.xyz, r3.xyzx, r2.xyzx
    r5.xyz = ((r3.xyzx)*(r2.xyzx)).xyz;
    // 80: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 81: mad r2.xyz, -r2.xyzx, r3.xyzx, r1.wwww
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r1.wwww)).xyz;
    // 82: mad r2.xyz, cb0[10].xxxx, r2.xyzx, r5.xyzx
    r2.xyz = ((source[10].xxxx)*(r2.xyzx)+(r5.xyzx)).xyz;
    // 83: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 84: mad r1.xyz, r0.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 85: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 86: mul r2.xy, v4.xyxx, cb0[2].xyxx
    r2.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s4, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 88: mul r3.xyz, cb0[3].xyzx, cb0[8].yyyy
    r3.xyz = ((source[3].xyzx)*(source[8].yyyy)).xyz;
    // 89: mad r2.xyz, r2.xyzx, r3.xyzx, cb0[1].xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)+(source[1].xyzx)).xyz;
    // 90: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 91: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 92: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 93: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 94: mad r0.xy, r0.xxxx, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xxxx)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 95: mul r0.xy, r0.xyxx, r0.xyxx
    r0.xy = ((r0.xyxx)*(r0.xyxx)).xy;
    // 96: mul r0.yzw, r0.yyyy, cb0[12].xxyz
    r0.yzw = ((r0.yyyy)*(source[12].xxyz)).yzw;
    // 97: mad r0.xyz, r0.xxxx, cb0[11].xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(source[11].xyzx)+(r0.yzwy)).xyz;
    // 98: mul r0.xyz, r0.xyzx, cb0[13].wwww
    r0.xyz = ((r0.xyzx)*(source[13].wwww)).xyz;
    // 99: mad r2.xyz, r0.xyzx, r1.xyzx, r2.xyzx
    r2.xyz = ((r0.xyzx)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 100: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 101: dp3 o4.y, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 102: mad o0.xyz, r1.xyzx, cb0[13].xyzx, r2.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[13].xyzx)+(r2.xyzx)).xyz;
    // 103: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 104: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 105: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 106: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 107: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 108: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 109: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 110: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 111: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 112: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 113: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 114: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 115: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 116: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 117: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 118: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 119: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 120: ret
    return output;
}

// source.character.static-map-native-233.v1 / source program 614cf164a4863a44a2b979301cdd9b63
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked233(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[17]=1.f; source[18]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 3: add r0.z, r1.w, l(-0.333300)
    r0.z = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 4: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 5: discard_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mul r0.w, r2.z, cb0[12].w
    r0.w = ((r2.zzzz)*(source[12].wwww)).w;
    // 11: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 12: mul r3.xy, r2.xyxx, cb0[9].wwww
    r3.xy = ((r2.xyxx)*(source[9].wwww)).xy;
    // 13: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 15: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 16: add r3.z, r2.x, l(0.000010)
    r3.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r2.x, r3.xyzx, r3.xyzx
    r2.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 18: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 19: div r2.xyz, r3.xyzx, r2.xxxx
    r2.xyz = ((r3.xyzx)/(r2.xxxx)).xyz;
    // 20: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 21: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: add r3.x, r2.w, -cb0[10].z
    r3.x = ((r2.wwww)+(-(source[10].zzzz))).x;
    // 23: mul_sat r3.x, r3.x, cb0[11].x
    r3.x = (saturate((r3.xxxx)*(source[11].xxxx))).x;
    // 24: mad r2.w, -r3.x, r3.x, r2.w
    r2.w = ((-(r3.xxxx))*(r3.xxxx)+(r2.wwww)).w;
    // 25: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 26: mad_sat r1.w, r1.w, r2.w, r3.x
    r1.w = (saturate((r1.wwww)*(r2.wwww)+(r3.xxxx))).w;
    // 27: mul r3.xy, v4.xyxx, cb0[10].xxxx
    r3.xy = ((v4.xyxx)*(source[10].xxxx)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 29: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 30: mul r4.xyz, cb0[3].xyzx, cb0[11].yyyy
    r4.xyz = ((source[3].xyzx)*(source[11].yyyy)).xyz;
    // 31: mad r5.xyz, r4.xyzx, r3.xyzx, -r0.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 32: mad r0.xyz, r1.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 34: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 35: mul r5.xy, v4.xyxx, cb0[5].xyxx
    r5.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: mul r6.xyz, cb0[6].xyzx, cb0[12].xxxx
    r6.xyz = ((source[6].xyzx)*(source[12].xxxx)).xyz;
    // 38: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 39: add r6.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 40: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 41: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 42: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 43: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 44: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 45: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 46: mul r6.xyz, r2.wwww, v5.xyzx
    r6.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 47: dp3 r2.w, r2.xyzx, r6.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 48: mul r7.xyz, r2.wwww, r2.xyzx
    r7.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 49: mad r6.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 50: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 51: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 52: mul r7.xyz, r2.wwww, v0.xyzx
    r7.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 53: dp3 r8.x, r7.xyzx, r6.xyzx
    r8.x = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 54: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r9.xyz, r2.wwww, v1.xyzx
    r9.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 57: dp3 r8.z, r9.xyzx, r6.xyzx
    r8.z = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: mul r10.xyz, r7.yzxy, r9.zxyz
    r10.xyz = ((r7.yzxy)*(r9.zxyz)).xyz;
    // 59: mad r10.xyz, r9.yzxy, r7.zxyz, -r10.xyzx
    r10.xyz = ((r9.yzxy)*(r7.zxyz)+(-(r10.xyzx))).xyz;
    // 60: dp3 r9.z, r9.xyzx, r2.xyzx
    r9.z = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 61: dp3 r9.x, r7.xyzx, r2.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 62: mul r7.xyz, r10.xyzx, v1.wwww
    r7.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r8.y, r7.xyzx, r6.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 64: dp3 r9.y, r7.xyzx, r2.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 65: dp3 r6.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx).y;
    // 66: dp3 r6.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx).z;
    // 67: dp2 r6.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx).x;
    // 68: min r6.xyz, |r6.xyzx|, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (min(abs(r6.xyzx),float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 70: mul r6.xyz, r6.xyzx, cb0[11].wwww
    r6.xyz = ((r6.xyzx)*(source[11].wwww)).xyz;
    // 71: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 72: add r2.w, r6.y, r6.x
    r2.w = ((r6.yyyy)+(r6.xxxx)).w;
    // 73: add r2.w, r6.z, r2.w
    r2.w = ((r6.zzzz)+(r2.wwww)).w;
    // 74: mad r0.xyz, r0.xyzx, r2.wwww, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r2.wwww)+(r5.xyzx)).xyz;
    // 75: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 76: mul r5.xyz, cb0[8].xyzx, cb0[13].xxxx
    r5.xyz = ((source[8].xyzx)*(source[13].xxxx)).xyz;
    // 77: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 78: mul r5.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r5.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 79: mad r5.xy, cb0[12].yyyy, r8.xyxx, r5.xyxx
    r5.xy = ((source[12].yyyy)*(r8.xyxx)+(r5.xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 81: mul r5.xyz, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r5.xyzx)*(source[7].xyzx)).xyz;
    // 82: mad r5.xyz, cb0[12].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[12].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 83: add r5.xyz, r5.xyzx, -cb0[12].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[12].zzzz))).xyz;
    // 84: mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // 85: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 86: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 87: mad r1.xyz, r0.wwww, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 88: mul r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)).xyz;
    // 89: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 91: mad r3.xyz, r4.xyzx, r3.xyzx, -r1.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 92: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 93: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 94: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 97: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 98: mad r3.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 100: mul r3.yzw, r3.yyyy, cb0[15].xxyz
    r3.yzw = ((r3.yyyy)*(source[15].xxyz)).yzw;
    // 101: mad r3.xyz, r3.xxxx, cb0[14].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[14].xyzx)+(r3.yzwy)).xyz;
    // 102: mul r3.xyz, r3.xyzx, cb0[16].wwww
    r3.xyz = ((r3.xyzx)*(source[16].wwww)).xyz;
    // 103: mul r4.xyz, r1.xyzx, r3.xyzx
    r4.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 104: dp2_sat r5.x, r2.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r5.x = (saturate(dot((r2.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 105: dp3_sat r5.y, r2.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r5.y = (saturate(dot((r2.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 106: dp3_sat r5.z, r2.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r5.z = (saturate(dot((r2.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 107: mul r2.xyz, r5.xyzx, r5.xyzx
    r2.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 108: sample_indexable(texture2d)(float,float,float,float) r5.xyz, v3.zwzz, t7.xyzw, s6
    r5.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 109: mul r5.xyz, r5.xyzx, cb0[18].xyzx
    r5.xyz = ((r5.xyzx)*(source[18].xyzx)).xyz;
    // 110: dp3 r0.w, r5.xyzx, r2.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 111: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s6
    r2.xyz = ((float4(input.bakedAverage,1.f)).xyzw).xyz;
    // 112: mul r2.xyz, r2.xyzx, cb0[17].xyzx
    r2.xyz = ((r2.xyzx)*(source[17].xyzx)).xyz;
    // 113: mul r5.xyz, r0.wwww, r2.xyzx
    r5.xyz = ((r0.wwww)*(r2.xyzx)).xyz;
    // 114: mad r2.xyz, r2.xyzx, r0.wwww, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 115: add r2.xyz, r2.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r2.xyz = ((r2.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 116: div r2.xyz, r5.xyzx, r2.xyzx
    r2.xyz = ((r5.xyzx)/(r2.xyzx)).xyz;
    // 117: mad r3.xyz, r1.xyzx, r5.xyzx, r4.xyzx
    r3.xyz = ((r1.xyzx)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 118: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 119: mul o4.z, r0.w, r3.x
    output.targets[4].z = ((r0.wwww)*(r3.xxxx)).z;
    // 120: add r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)+(r3.xyzx)).xyz;
    // 121: dp3 o4.y, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 122: mad o0.xyz, r1.xyzx, cb0[16].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 123: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 124: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 125: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 126: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 127: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
    // 128: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 129: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 130: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 131: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 132: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 133: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 134: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 135: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 136: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 137: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 138: mov o4.xw, l(0,0,0,0)
    output.targets[4].xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 139: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 140: ret
    return output;
}

// source.character.static-map-native-233.v1 / source program 95695570f692614c867f93d77db9244c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase233(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[8]=g_SourceCharacterBaseConstants[8];
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
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
    // 1: mul r0.xy, v4.xyxx, cb0[2].xyxx
    r0.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r1.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 3: add r0.z, r1.w, l(-0.333300)
    r0.z = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 4: lt r0.z, r0.z, l(0.000000)
    r0.z = (asfloat((uint4)((r0.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 5: discard_nz r0.z
    if ((asuint(r0.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 6: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r0.xyxx, t0.xywz, s0, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s3, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 9: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 10: mul r0.w, r2.z, cb0[12].w
    r0.w = ((r2.zzzz)*(source[12].wwww)).w;
    // 11: dp2 r2.z, r2.xyxx, r2.xyxx
    r2.z = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).z;
    // 12: mul r3.xy, r2.xyxx, cb0[9].wwww
    r3.xy = ((r2.xyxx)*(source[9].wwww)).xy;
    // 13: add r2.x, -r2.z, l(1.000000)
    r2.x = ((-(r2.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 14: max r2.x, r2.x, l(0.000000)
    r2.x = (max(r2.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 15: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 16: add r3.z, r2.x, l(0.000010)
    r3.z = ((r2.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 17: dp3 r2.x, r3.xyzx, r3.xyzx
    r2.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 18: sqrt r2.x, r2.x
    r2.x = (sqrt(r2.xxxx)).x;
    // 19: div r2.xyz, r3.xyzx, r2.xxxx
    r2.xyz = ((r3.xyzx)/(r2.xxxx)).xyz;
    // 20: mul r1.w, r1.w, r2.z
    r1.w = ((r1.wwww)*(r2.zzzz)).w;
    // 21: add r2.w, -v2.x, l(1.000000)
    r2.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 22: add r3.x, r2.w, -cb0[10].z
    r3.x = ((r2.wwww)+(-(source[10].zzzz))).x;
    // 23: mul_sat r3.x, r3.x, cb0[11].x
    r3.x = (saturate((r3.xxxx)*(source[11].xxxx))).x;
    // 24: mad r2.w, -r3.x, r3.x, r2.w
    r2.w = ((-(r3.xxxx))*(r3.xxxx)+(r2.wwww)).w;
    // 25: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 26: mad_sat r1.w, r1.w, r2.w, r3.x
    r1.w = (saturate((r1.wwww)*(r2.wwww)+(r3.xxxx))).w;
    // 27: mul r3.xy, v4.xyxx, cb0[10].xxxx
    r3.xy = ((v4.xyxx)*(source[10].xxxx)).xy;
    // 28: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 29: mul r1.w, r1.w, r3.w
    r1.w = ((r1.wwww)*(r3.wwww)).w;
    // 30: mul r4.xyz, cb0[3].xyzx, cb0[11].yyyy
    r4.xyz = ((source[3].xyzx)*(source[11].yyyy)).xyz;
    // 31: mad r5.xyz, r4.xyzx, r3.xyzx, -r0.xyzx
    r5.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r0.xyzx))).xyz;
    // 32: mad r0.xyz, r1.wwww, r5.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r5.xyzx)+(r0.xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 34: mul r0.xyz, r0.xyzx, cb0[11].zzzz
    r0.xyz = ((r0.xyzx)*(source[11].zzzz)).xyz;
    // 35: mul r5.xy, v4.xyxx, cb0[5].xyxx
    r5.xy = ((v4.xyxx)*(source[5].xyxx)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t5.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: mul r6.xyz, cb0[6].xyzx, cb0[12].xxxx
    r6.xyz = ((source[6].xyzx)*(source[12].xxxx)).xyz;
    // 38: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 39: add r6.xyz, -r2.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r6.xyz = ((-(r2.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 40: mad r2.xyz, r1.wwww, r6.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r6.xyzx)+(r2.xyzx)).xyz;
    // 41: dp3 r2.w, r2.xyzx, r2.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 42: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 43: mul r2.xyz, r2.wwww, r2.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 44: dp3 r2.w, v5.xyzx, v5.xyzx
    r2.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 45: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 46: mul r6.xyz, r2.wwww, v5.xyzx
    r6.xyz = ((r2.wwww)*(v5.xyzx)).xyz;
    // 47: dp3 r2.w, r2.xyzx, r6.xyzx
    r2.w = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 48: mul r7.xyz, r2.wwww, r2.xyzx
    r7.xyz = ((r2.wwww)*(r2.xyzx)).xyz;
    // 49: mad r6.xyz, r7.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r6.xyzx
    r6.xyz = ((r7.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r6.xyzx))).xyz;
    // 50: dp3 r2.w, v0.xyzx, v0.xyzx
    r2.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 51: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 52: mul r7.xyz, r2.wwww, v0.xyzx
    r7.xyz = ((r2.wwww)*(v0.xyzx)).xyz;
    // 53: dp3 r8.x, r7.xyzx, r6.xyzx
    r8.x = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 54: dp3 r2.w, v1.xyzx, v1.xyzx
    r2.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r9.xyz, r2.wwww, v1.xyzx
    r9.xyz = ((r2.wwww)*(v1.xyzx)).xyz;
    // 57: dp3 r8.z, r9.xyzx, r6.xyzx
    r8.z = (dot((r9.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: mul r10.xyz, r7.yzxy, r9.zxyz
    r10.xyz = ((r7.yzxy)*(r9.zxyz)).xyz;
    // 59: mad r10.xyz, r9.yzxy, r7.zxyz, -r10.xyzx
    r10.xyz = ((r9.yzxy)*(r7.zxyz)+(-(r10.xyzx))).xyz;
    // 60: dp3 r9.z, r9.xyzx, r2.xyzx
    r9.z = (dot((r9.xyzx).xyz,(r2.xyzx).xyz).xxxx).z;
    // 61: dp3 r9.x, r7.xyzx, r2.xyzx
    r9.x = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 62: mul r7.xyz, r10.xyzx, v1.wwww
    r7.xyz = ((r10.xyzx)*(v1.wwww)).xyz;
    // 63: dp3 r8.y, r7.xyzx, r6.xyzx
    r8.y = (dot((r7.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 64: dp3 r9.y, r7.xyzx, r2.xyzx
    r9.y = (dot((r7.xyzx).xyz,(r2.xyzx).xyz).xxxx).y;
    // 65: dp3 r6.y, r8.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r6.y = (dot((r8.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx).y;
    // 66: dp3 r6.z, r8.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r6.z = (dot((r8.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx).z;
    // 67: dp2 r6.x, r8.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r6.x = (dot((r8.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx).x;
    // 68: min r6.xyz, |r6.xyzx|, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = (min(abs(r6.xyzx),float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 69: log r6.xyz, r6.xyzx
    r6.xyz = (log2(r6.xyzx)).xyz;
    // 70: mul r6.xyz, r6.xyzx, cb0[11].wwww
    r6.xyz = ((r6.xyzx)*(source[11].wwww)).xyz;
    // 71: exp r6.xyz, r6.xyzx
    r6.xyz = (exp2(r6.xyzx)).xyz;
    // 72: add r2.w, r6.y, r6.x
    r2.w = ((r6.yyyy)+(r6.xxxx)).w;
    // 73: add r2.w, r6.z, r2.w
    r2.w = ((r6.zzzz)+(r2.wwww)).w;
    // 74: mad r0.xyz, r0.xyzx, r2.wwww, r5.xyzx
    r0.xyz = ((r0.xyzx)*(r2.wwww)+(r5.xyzx)).xyz;
    // 75: add r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)+(source[1].xyzx)).xyz;
    // 76: mul r5.xyz, cb0[8].xyzx, cb0[13].xxxx
    r5.xyz = ((source[8].xyzx)*(source[13].xxxx)).xyz;
    // 77: mul r1.xyz, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.xyzx)*(r5.xyzx)).xyz;
    // 78: mul r5.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r5.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 79: mad r5.xy, cb0[12].yyyy, r8.xyxx, r5.xyxx
    r5.xy = ((source[12].yyyy)*(r8.xyxx)+(r5.xyxx)).xy;
    // 80: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t3.xyzw, s5, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 81: mul r5.xyz, r5.xyzx, cb0[7].xyzx
    r5.xyz = ((r5.xyzx)*(source[7].xyzx)).xyz;
    // 82: mad r5.xyz, cb0[12].zzzz, r5.xyzx, r5.xyzx
    r5.xyz = ((source[12].zzzz)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 83: add r5.xyz, r5.xyzx, -cb0[12].zzzz
    r5.xyz = ((r5.xyzx)+(-(source[12].zzzz))).xyz;
    // 84: mov_sat r6.xyz, r5.xyzx
    r6.xyz = (saturate(r5.xyzx)).xyz;
    // 85: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 86: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 87: mad r1.xyz, r0.wwww, r6.xyzx, r1.xyzx
    r1.xyz = ((r0.wwww)*(r6.xyzx)+(r1.xyzx)).xyz;
    // 88: mul r1.xyz, r5.xyzx, r1.xyzx
    r1.xyz = ((r5.xyzx)*(r1.xyzx)).xyz;
    // 89: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 90: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 91: mad r3.xyz, r4.xyzx, r3.xyzx, -r1.xyzx
    r3.xyz = ((r4.xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 92: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 93: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 94: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 95: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 96: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 97: dp3 r0.w, r3.xyzx, r2.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 98: mad r2.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r2.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 99: mul r2.xy, r2.xyxx, r2.xyxx
    r2.xy = ((r2.xyxx)*(r2.xyxx)).xy;
    // 100: mul r2.yzw, r2.yyyy, cb0[15].xxyz
    r2.yzw = ((r2.yyyy)*(source[15].xxyz)).yzw;
    // 101: mad r2.xyz, r2.xxxx, cb0[14].xyzx, r2.yzwy
    r2.xyz = ((r2.xxxx)*(source[14].xyzx)+(r2.yzwy)).xyz;
    // 102: mul r2.xyz, r2.xyzx, cb0[16].wwww
    r2.xyz = ((r2.xyzx)*(source[16].wwww)).xyz;
    // 103: mad r0.xyz, r2.xyzx, r1.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 104: mul r2.xyz, r1.xyzx, r2.xyzx
    r2.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 105: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 106: mad o0.xyz, r1.xyzx, cb0[16].xyzx, r0.xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[16].xyzx)+(r0.xyzx)).xyz;
    // 107: mov o3.xyz, r1.xyzx
    output.targets[3].xyz = (r1.xyzx).xyz;
    // 108: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: dp3 r0.x, r9.xyzx, r9.xyzx
    r0.x = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 110: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 111: mul r0.xyz, r0.xxxx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r9.xyzx)).xyz;
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

// source.character.static-map-native-234.v1 / source program 18f0a36e4f9a414b898c48dd788bf1e3
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapMonsterBaked234(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterBaseConstants[i];
    source[14]=1.f; source[15]=1.f; // RNM samples already contain their instance scales.
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: mul r0.w, r1.z, cb0[9].x
    r0.w = ((r1.zzzz)*(source[9].xxxx)).w;
    // 6: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: mul r1.xy, r1.xyxx, cb0[7].xxxx
    r1.xy = ((r1.xyxx)*(source[7].xxxx)).xy;
    // 8: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 9: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 11: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 12: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 14: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 15: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 16: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 19: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 20: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 23: mul r5.xyz, r2.zxyz, r4.yzxy
    r5.xyz = ((r2.zxyz)*(r4.yzxy)).xyz;
    // 24: mad r5.xyz, r2.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r2.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 25: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 27: dp3 r3.x, r4.xyzx, r1.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 29: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 31: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 34: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: mul r6.xy, v4.xyxx, cb0[8].yyyy
    r6.xy = ((v4.xyxx)*(source[8].yyyy)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 38: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 39: max r0.z, cb0[7].y, l(0.000000)
    r0.z = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 40: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 41: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 42: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 43: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 44: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 45: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 46: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 47: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 48: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 49: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 50: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 51: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 52: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 53: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 54: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 55: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 56: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 57: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 58: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 59: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 60: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 61: dp3 r8.x, r4.xyzx, r7.xyzx
    r8.x = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 62: dp3 r4.x, r4.xyzx, r1.xyzx
    r4.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 63: dp3 r8.y, r5.xyzx, r7.xyzx
    r8.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 64: dp3 r4.y, r5.xyzx, r1.xyzx
    r4.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 65: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 66: mad r0.yz, cb0[8].zzzz, r8.xxyx, r0.yyzy
    r0.yz = ((source[8].zzzz)*(r8.xxyx)+(r0.yyzy)).yz;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r0.yzyy, t3.xyzw, s3, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r5.xyz, r5.xyzx, cb0[3].xyzx
    r5.xyz = ((r5.xyzx)*(source[3].xyzx)).xyz;
    // 69: mad r5.xyz, cb0[8].wwww, r5.xyzx, r5.xyzx
    r5.xyz = ((source[8].wwww)*(r5.xyzx)+(r5.xyzx)).xyz;
    // 70: add r5.xyz, r5.xyzx, -cb0[8].wwww
    r5.xyz = ((r5.xyzx)+(-(source[8].wwww))).xyz;
    // 71: mov_sat r8.xyz, r5.xyzx
    r8.xyz = (saturate(r5.xyzx)).xyz;
    // 72: mov_sat r5.xyz, -r5.xyzx
    r5.xyz = (saturate(-(r5.xyzx))).xyz;
    // 73: mad r5.xyz, -r0.wwww, r5.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(r0.wwww))*(r5.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mul r9.xyz, cb0[4].xyzx, cb0[9].yyyy
    r9.xyz = ((source[4].xyzx)*(source[9].yyyy)).xyz;
    // 75: mul r3.xyz, r3.xyzx, r9.xyzx
    r3.xyz = ((r3.xyzx)*(r9.xyzx)).xyz;
    // 76: mad r0.yzw, r0.wwww, r8.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r8.xxyz)+(r3.xxyz)).yzw;
    // 77: mul r0.yzw, r5.xxyz, r0.yyzw
    r0.yzw = ((r5.xxyz)*(r0.yyzw)).yzw;
    // 78: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 79: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 80: mul r3.xyz, cb0[5].xyzx, cb0[9].zzzz
    r3.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 81: mul r5.xyz, r6.xyzx, r3.xyzx
    r5.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 82: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 84: mad r3.xyz, cb0[10].xxxx, r3.xyzx, r5.xyzx
    r3.xyz = ((source[10].xxxx)*(r3.xyzx)+(r5.xyzx)).xyz;
    // 85: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 86: mad r0.yzw, r0.xxxx, r3.xxyz, r0.yyzw
    r0.yzw = ((r0.xxxx)*(r3.xxyz)+(r0.yyzw)).yzw;
    // 87: mad r0.yzw, r0.yyzw, cb2[3].wwww, cb2[3].xxyz
    r0.yzw = ((r0.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz)).yzw;
    // 88: dp3 r1.w, v6.xyzx, v6.xyzx
    r1.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 89: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 90: mul r3.xyz, r1.wwww, v6.xyzx
    r3.xyz = ((r1.wwww)*(v6.xyzx)).xyz;
    // 91: dp3 r1.w, r3.xyzx, r1.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 92: mad r3.xy, r1.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r1.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 93: mul r3.xy, r3.xyxx, r3.xyxx
    r3.xy = ((r3.xyxx)*(r3.xyxx)).xy;
    // 94: mul r3.yzw, r3.yyyy, cb0[12].xxyz
    r3.yzw = ((r3.yyyy)*(source[12].xxyz)).yzw;
    // 95: mad r3.xyz, r3.xxxx, cb0[11].xyzx, r3.yzwy
    r3.xyz = ((r3.xxxx)*(source[11].xyzx)+(r3.yzwy)).xyz;
    // 96: mul r3.xyz, r3.xyzx, cb0[13].wwww
    r3.xyz = ((r3.xyzx)*(source[13].wwww)).xyz;
    // 97: mul r5.xyz, r0.yzwy, r3.xyzx
    r5.xyz = ((r0.yzwy)*(r3.xyzx)).xyz;
    // 98: dp2_sat r8.x, r1.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r8.x = (saturate(dot((r1.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 99: dp3_sat r8.y, r1.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r8.y = (saturate(dot((r1.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 100: dp3_sat r8.z, r1.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r8.z = (saturate(dot((r1.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 101: dp3 r4.z, r2.xyzx, r1.xyzx
    r4.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 102: mul r1.xyz, r8.xyzx, r8.xyzx
    r1.xyz = ((r8.xyzx)*(r8.xyzx)).xyz;
    // 103: sample_indexable(texture2d)(float,float,float,float) r2.xyz, v3.zwzz, t6.xyzw, s5
    r2.xyz = ((float4(input.bakedCoefficients,1.f)).xyzw).xyz;
    // 104: mul r2.xyz, r2.xyzx, cb0[15].xyzx
    r2.xyz = ((r2.xyzx)*(source[15].xyzx)).xyz;
    // 105: dp3 r1.x, r2.xyzx, r1.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 106: sample_indexable(texture2d)(float,float,float,float) r1.yzw, v3.zwzz, t5.wxyz, s5
    r1.yzw = ((float4(input.bakedAverage,1.f)).wxyz).yzw;
    // 107: mul r1.yzw, r1.yyzw, cb0[14].xxyz
    r1.yzw = ((r1.yyzw)*(source[14].xxyz)).yzw;
    // 108: mul r8.xyz, r1.xxxx, r1.yzwy
    r8.xyz = ((r1.xxxx)*(r1.yzwy)).xyz;
    // 109: mad r3.xyz, r1.yzwy, r1.xxxx, r3.xyzx
    r3.xyz = ((r1.yzwy)*(r1.xxxx)+(r3.xyzx)).xyz;
    // 110: add r3.xyz, r3.xyzx, l(0.000010, 0.000010, 0.000010, 0.000000)
    r3.xyz = ((r3.xyzx)+(float4(0.000010,0.000010,0.000010,0.000000))).xyz;
    // 111: div r3.xyz, r8.xyzx, r3.xyzx
    r3.xyz = ((r8.xyzx)/(r3.xyzx)).xyz;
    // 112: mad r5.xyz, r0.yzwy, r8.xyzx, r5.xyzx
    r5.xyz = ((r0.yzwy)*(r8.xyzx)+(r5.xyzx)).xyz;
    // 113: dp3 r1.x, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: dp3 r2.w, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 115: add r3.xyz, -r6.xyzx, r2.wwww
    r3.xyz = ((-(r6.xyzx))+(r2.wwww)).xyz;
    // 116: mad r3.xyz, cb0[10].xxxx, r3.xyzx, r6.xyzx
    r3.xyz = ((source[10].xxxx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 117: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, v4.xyxx, t4.xyzw, s4, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 118: mul r8.xyz, cb0[6].xyzx, cb0[10].yyyy
    r8.xyz = ((source[6].xyzx)*(source[10].yyyy)).xyz;
    // 119: mul r6.xyz, r6.xyzx, r8.xyzx
    r6.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 120: mad r3.xyz, cb0[10].zzzz, r3.xyzx, -r6.xyzx
    r3.xyz = ((source[10].zzzz)*(r3.xyzx)+(-(r6.xyzx))).xyz;
    // 121: mad r3.xyz, r0.xxxx, r3.xyzx, r6.xyzx
    r3.xyz = ((r0.xxxx)*(r3.xyzx)+(r6.xyzx)).xyz;
    // 122: mad r3.xyz, r3.xyzx, cb2[4].wwww, cb2[4].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 123: mul r1.yzw, r1.yyzw, r3.xxyz
    r1.yzw = ((r1.yyzw)*(r3.xxyz)).yzw;
    // 124: dp2_sat r3.x, r7.yzyy, l(0.816497, 0.577350, 0.000000, 0.000000)
    r3.x = (saturate(dot((r7.yzyy).xy,(float4(0.816497,0.577350,0.000000,0.000000)).xy).xxxx)).x;
    // 125: dp3_sat r3.y, r7.xyzx, l(-0.707107, -0.408248, 0.577350, 0.000000)
    r3.y = (saturate(dot((r7.xyzx).xyz,(float4(-0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).y;
    // 126: dp3_sat r3.z, r7.xyzx, l(0.707107, -0.408248, 0.577350, 0.000000)
    r3.z = (saturate(dot((r7.xyzx).xyz,(float4(0.707107,-0.408248,0.577350,0.000000)).xyz).xxxx)).z;
    // 127: log r3.xyz, r3.xyzx
    r3.xyz = (log2(r3.xyzx)).xyz;
    // 128: add r0.x, cb0[10].w, l(1.000000)
    r0.x = ((source[10].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 129: mul r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 130: exp r3.xyz, r3.xyzx
    r3.xyz = (exp2(r3.xyzx)).xyz;
    // 131: dp3 r0.x, r2.xyzx, r3.xyzx
    r0.x = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 132: mad r2.xyz, r1.yzwy, r0.xxxx, r5.xyzx
    r2.xyz = ((r1.yzwy)*(r0.xxxx)+(r5.xyzx)).xyz;
    // 133: mul r1.yzw, r0.xxxx, r1.yyzw
    r1.yzw = ((r0.xxxx)*(r1.yyzw)).yzw;
    // 134: dp3 o4.x, r1.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].x = (dot((r1.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 135: add r1.yzw, r2.xxyz, cb0[1].xxyz
    r1.yzw = ((r2.xxyz)+(source[1].xxyz)).yzw;
    // 136: mad o0.xyz, r0.yzwy, cb0[13].xyzx, r1.yzwy
    output.targets[0].xyz = ((r0.yzwy)*(source[13].xyzx)+(r1.yzwy)).xyz;
    // 137: mov o3.xyz, r0.yzwy
    output.targets[3].xyz = (r0.yzwy).xyz;
    // 138: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 139: dp3 r0.x, r4.xyzx, r4.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 140: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 141: mul r0.xyz, r0.xxxx, r4.xyzx
    r0.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 142: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 143: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 144: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 145: ge r1.yz, r0.xxyx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.yz = (asfloat((uint4)((r0.xxyx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).yz;
    // 146: movc r1.yz, r1.yyzy, l(0,1.000000,1.000000,0), l(0,-1.000000,-1.000000,0)
    r1.yz = ((asuint(r1.yyzy) != 0u) ? (float4(asfloat(0u),1.000000,1.000000,asfloat(0u))) : (float4(asfloat(0u),-1.000000,-1.000000,asfloat(0u)))).yz;
    // 147: mad r1.yz, -|r0.yyxy|, r1.yyzy, r1.yyzy
    r1.yz = ((-(abs(r0.yyxy)))*(r1.yyzy)+(r1.yyzy)).yz;
    // 148: movc r0.xy, r0.wwww, r1.yzyy, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.yzyy) : (r0.xyxx)).xy;
    // 149: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 150: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 151: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 152: mul o4.z, r1.x, r2.x
    output.targets[4].z = ((r1.xxxx)*(r2.xxxx)).z;
    // 153: dp3 o4.y, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 154: mov o4.w, l(0)
    output.targets[4].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 155: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 156: ret
    return output;
}

// source.character.static-map-native-234.v1 / source program 5e26b797c7cecb42a3ac851d5f72ed94
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterBase234(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterBaseConstants[1];
    source[2]=g_SourceCharacterBaseConstants[2];
    source[3]=g_SourceCharacterBaseConstants[3];
    source[4]=g_SourceCharacterBaseConstants[4];
    source[5]=g_SourceCharacterBaseConstants[5];
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
    source[9].x=g_SourceCharacterBaseConstants[10].x;
    source[9].y=g_SourceCharacterBaseConstants[10].y;
    source[9].z=g_SourceCharacterBaseConstants[10].z;
    source[9].w=g_SourceCharacterBaseConstants[10].w;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0;
    // 1: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 2: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, v4.xyxx, t0.xywz, s0, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 4: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: mul r0.w, r1.z, cb0[8].x
    r0.w = ((r1.zzzz)*(source[8].xxxx)).w;
    // 6: dp2 r1.z, r1.xyxx, r1.xyxx
    r1.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 8: mul r2.xy, r1.xyxx, v2.wwww
    r2.xy = ((r1.xyxx)*(v2.wwww)).xy;
    // 9: add r1.x, -r1.z, l(1.000000)
    r1.x = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 10: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 11: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 12: add r2.z, r1.x, l(0.000010)
    r2.z = ((r1.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 13: dp3 r1.x, r2.xyzx, r2.xyzx
    r1.x = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 14: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 15: div r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)/(r1.xxxx)).xyz;
    // 16: dp3 r1.w, v1.xyzx, v1.xyzx
    r1.w = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).w;
    // 17: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 18: mul r2.xyz, r1.wwww, v1.xyzx
    r2.xyz = ((r1.wwww)*(v1.xyzx)).xyz;
    // 19: dp3 r3.z, r2.xyzx, r1.xyzx
    r3.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 20: dp3 r1.w, v0.xyzx, v0.xyzx
    r1.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 21: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 22: mul r4.xyz, r1.wwww, v0.xyzx
    r4.xyz = ((r1.wwww)*(v0.xyzx)).xyz;
    // 23: mul r5.xyz, r2.zxyz, r4.yzxy
    r5.xyz = ((r2.zxyz)*(r4.yzxy)).xyz;
    // 24: mad r5.xyz, r2.yzxy, r4.zxyz, -r5.xyzx
    r5.xyz = ((r2.yzxy)*(r4.zxyz)+(-(r5.xyzx))).xyz;
    // 25: mul r5.xyz, r5.xyzx, v1.wwww
    r5.xyz = ((r5.xyzx)*(v1.wwww)).xyz;
    // 26: dp3 r3.y, r5.xyzx, r1.xyzx
    r3.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 27: dp3 r3.x, r4.xyzx, r1.xyzx
    r3.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 28: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 29: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 30: mad r0.x, r0.x, l(0.500000), cb0[7].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).x;
    // 31: mul r0.y, r1.z, r1.z
    r0.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, v4.xyxx, t1.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: mul_sat r0.y, r0.y, r3.w
    r0.y = (saturate((r0.yyyy)*(r3.wwww))).y;
    // 34: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 35: mul r6.xy, v4.xyxx, cb0[7].yyyy
    r6.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 37: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 38: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 39: max r0.z, cb0[6].y, l(0.000000)
    r0.z = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 40: min r0.z, r0.z, l(0.990000)
    r0.z = (min(r0.zzzz,float4(0.990000,0.990000,0.990000,0.990000))).z;
    // 41: mul r1.w, r0.y, r0.z
    r1.w = ((r0.yyyy)*(r0.zzzz)).w;
    // 42: mad r0.x, r0.x, r1.w, r0.x
    r0.x = ((r0.xxxx)*(r1.wwww)+(r0.xxxx)).x;
    // 43: add r1.w, -r0.z, r0.x
    r1.w = ((-(r0.zzzz))+(r0.xxxx)).w;
    // 44: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 45: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = (((r0.zzzz) != 0.f ? 1.f / (r0.zzzz) : 0.f)).z;
    // 46: mad r0.x, -r0.z, r1.w, r0.x
    r0.x = ((-(r0.zzzz))*(r1.wwww)+(r0.xxxx)).x;
    // 47: mul r0.z, r1.w, r0.z
    r0.z = ((r1.wwww)*(r0.zzzz)).z;
    // 48: mad_sat r0.x, r0.y, r0.x, r0.z
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r0.zzzz))).x;
    // 49: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 50: add r7.xyz, -r1.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r1.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 51: mad r1.xyz, r0.yyyy, r7.xyzx, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r7.xyzx)+(r1.xyzx)).xyz;
    // 52: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 53: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 54: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 55: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 56: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 57: mul r7.xyz, r0.yyyy, v5.xyzx
    r7.xyz = ((r0.yyyy)*(v5.xyzx)).xyz;
    // 58: dp3 r0.y, r1.xyzx, r7.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 59: mul r8.xyz, r0.yyyy, r1.xyzx
    r8.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 60: mad r7.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r7.xyzx
    r7.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r7.xyzx))).xyz;
    // 61: dp3 r8.x, r4.xyzx, r7.xyzx
    r8.x = (dot((r4.xyzx).xyz,(r7.xyzx).xyz).xxxx).x;
    // 62: dp3 r8.y, r5.xyzx, r7.xyzx
    r8.y = (dot((r5.xyzx).xyz,(r7.xyzx).xyz).xxxx).y;
    // 63: dp3 r5.y, r5.xyzx, r1.xyzx
    r5.y = (dot((r5.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 64: dp3 r5.x, r4.xyzx, r1.xyzx
    r5.x = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 65: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 66: mad r0.yz, cb0[7].zzzz, r8.xxyx, r0.yyzy
    r0.yz = ((source[7].zzzz)*(r8.xxyx)+(r0.yyzy)).yz;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r0.yzyy, t3.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 68: mul r4.xyz, r4.xyzx, cb0[3].xyzx
    r4.xyz = ((r4.xyzx)*(source[3].xyzx)).xyz;
    // 69: mad r4.xyz, cb0[7].wwww, r4.xyzx, r4.xyzx
    r4.xyz = ((source[7].wwww)*(r4.xyzx)+(r4.xyzx)).xyz;
    // 70: add r4.xyz, r4.xyzx, -cb0[7].wwww
    r4.xyz = ((r4.xyzx)+(-(source[7].wwww))).xyz;
    // 71: mov_sat r7.xyz, r4.xyzx
    r7.xyz = (saturate(r4.xyzx)).xyz;
    // 72: mov_sat r4.xyz, -r4.xyzx
    r4.xyz = (saturate(-(r4.xyzx))).xyz;
    // 73: mad r4.xyz, -r0.wwww, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((-(r0.wwww))*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mul r8.xyz, cb0[4].xyzx, cb0[8].yyyy
    r8.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 75: mul r3.xyz, r3.xyzx, r8.xyzx
    r3.xyz = ((r3.xyzx)*(r8.xyzx)).xyz;
    // 76: mad r0.yzw, r0.wwww, r7.xxyz, r3.xxyz
    r0.yzw = ((r0.wwww)*(r7.xxyz)+(r3.xxyz)).yzw;
    // 77: mul r0.yzw, r4.xxyz, r0.yyzw
    r0.yzw = ((r4.xxyz)*(r0.yyzw)).yzw;
    // 78: max r0.yzw, r0.yyzw, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.yzw = (max(r0.yyzw,float4(0.000000,0.000000,0.000000,0.000000))).yzw;
    // 79: min r0.yzw, r0.yyzw, l(0.000000, 999.000000, 999.000000, 999.000000)
    r0.yzw = (min(r0.yyzw,float4(0.000000,999.000000,999.000000,999.000000))).yzw;
    // 80: mul r3.xyz, cb0[5].xyzx, cb0[8].zzzz
    r3.xyz = ((source[5].xyzx)*(source[8].zzzz)).xyz;
    // 81: mul r4.xyz, r6.xyzx, r3.xyzx
    r4.xyz = ((r6.xyzx)*(r3.xyzx)).xyz;
    // 82: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 83: mad r3.xyz, -r3.xyzx, r6.xyzx, r1.wwww
    r3.xyz = ((-(r3.xyzx))*(r6.xyzx)+(r1.wwww)).xyz;
    // 84: mad r3.xyz, cb0[9].xxxx, r3.xyzx, r4.xyzx
    r3.xyz = ((source[9].xxxx)*(r3.xyzx)+(r4.xyzx)).xyz;
    // 85: add r3.xyz, -r0.yzwy, r3.xyzx
    r3.xyz = ((-(r0.yzwy))+(r3.xyzx)).xyz;
    // 86: mad r0.xyz, r0.xxxx, r3.xyzx, r0.yzwy
    r0.xyz = ((r0.xxxx)*(r3.xyzx)+(r0.yzwy)).xyz;
    // 87: mad r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 88: dp3 r0.w, v6.xyzx, v6.xyzx
    r0.w = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).w;
    // 89: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 90: mul r3.xyz, r0.wwww, v6.xyzx
    r3.xyz = ((r0.wwww)*(v6.xyzx)).xyz;
    // 91: dp3 r0.w, r3.xyzx, r1.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 92: dp3 r5.z, r2.xyzx, r1.xyzx
    r5.z = (dot((r2.xyzx).xyz,(r1.xyzx).xyz).xxxx).z;
    // 93: mad r1.xy, r0.wwww, l(0.500000, -0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r0.wwww)*(float4(0.500000,-0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 94: mul r1.xy, r1.xyxx, r1.xyxx
    r1.xy = ((r1.xyxx)*(r1.xyxx)).xy;
    // 95: mul r1.yzw, r1.yyyy, cb0[11].xxyz
    r1.yzw = ((r1.yyyy)*(source[11].xxyz)).yzw;
    // 96: mad r1.xyz, r1.xxxx, cb0[10].xyzx, r1.yzwy
    r1.xyz = ((r1.xxxx)*(source[10].xyzx)+(r1.yzwy)).xyz;
    // 97: mul r1.xyz, r1.xyzx, cb0[12].wwww
    r1.xyz = ((r1.xyzx)*(source[12].wwww)).xyz;
    // 98: mad r2.xyz, r1.xyzx, r0.xyzx, cb0[1].xyzx
    r2.xyz = ((r1.xyzx)*(r0.xyzx)+(source[1].xyzx)).xyz;
    // 99: mul r1.xyz, r0.xyzx, r1.xyzx
    r1.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 100: dp3 o4.y, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    output.targets[4].y = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 101: mad o0.xyz, r0.xyzx, cb0[12].xyzx, r2.xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)+(r2.xyzx)).xyz;
    // 102: mov o3.xyz, r0.xyzx
    output.targets[3].xyz = (r0.xyzx).xyz;
    // 103: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 104: dp3 r0.x, r5.xyzx, r5.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 105: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 106: mul r0.xyz, r0.xxxx, r5.xyzx
    r0.xyz = ((r0.xxxx)*(r5.xyzx)).xyz;
    // 107: ge r0.w, l(0.000000), r0.z
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))>=(r0.zzzz)) * 0xffffffffu)).w;
    // 108: dp3 r0.z, l(1.000000, 1.000000, 1.000000, 0.000000), |r0.xyzx|
    r0.z = (dot((float4(1.000000,1.000000,1.000000,0.000000)).xyz,(abs(r0.xyzx)).xyz).xxxx).z;
    // 109: div r0.xy, r0.xyxx, r0.zzzz
    r0.xy = ((r0.xyxx)/(r0.zzzz)).xy;
    // 110: ge r1.xy, r0.xyxx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xy = (asfloat((uint4)((r0.xyxx)>=(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).xy;
    // 111: movc r1.xy, r1.xyxx, l(1.000000,1.000000,0,0), l(-1.000000,-1.000000,0,0)
    r1.xy = ((asuint(r1.xyxx) != 0u) ? (float4(1.000000,1.000000,asfloat(0u),asfloat(0u))) : (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u)))).xy;
    // 112: mad r1.xy, -|r0.yxyy|, r1.xyxx, r1.xyxx
    r1.xy = ((-(abs(r0.yxyy)))*(r1.xyxx)+(r1.xyxx)).xy;
    // 113: movc r0.xy, r0.wwww, r1.xyxx, r0.xyxx
    r0.xy = ((asuint(r0.wwww) != 0u) ? (r1.xyxx) : (r0.xyxx)).xy;
    // 114: mad o2.xy, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000), l(0.500000, 0.500000, 0.000000, 0.000000)
    output.targets[2].xy = ((r0.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 115: mov o2.zw, l(0,0,1.000000,0)
    output.targets[2].zw = (float4(asfloat(0u),asfloat(0u),1.000000,asfloat(0u))).zw;
    // 116: mov o3.w, l(0)
    output.targets[3].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 117: mov o4.xzw, l(0,0,0,0)
    output.targets[4].xzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xzw;
    // 118: mov o5.xyzw, l(0,0,0,0)
    output.targets[5].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 119: ret
    return output;
}

// source.character.static-map-native-237.v1 / source program aa9cc8cfd384c84b8c71cbec7d57b93f
