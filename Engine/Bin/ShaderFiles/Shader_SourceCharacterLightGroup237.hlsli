// source.character.static-map-native-237.v1 / source program e73b0ff02ebe4748b14ec48757093949
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight237(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6].x=(g_SourceCharacterTime.xxxx).x;
    source[8]=float4(input.lightColor,1.0);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0;
    // 1: mad r0.xy, cb0[6].xxxx, cb0[2].zwzz, v2.xyxx
    r0.xy = ((source[6].xxxx)*(source[2].zwzz)+(v2.xyxx)).xy;
    // 2: mul r0.xy, r0.xyxx, cb0[2].xyxx
    r0.xy = ((r0.xyxx)*(source[2].xyxx)).xy;
    // 3: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r0.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 4: mad r0.xy, r0.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 5: dp2 r0.w, r0.xyxx, r0.xyxx
    r0.w = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).w;
    // 6: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 7: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 8: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 9: add r0.w, r0.w, l(0.000010)
    r0.w = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 10: mad r1.xy, -cb0[6].xxxx, cb0[2].zwzz, v2.xyxx
    r1.xy = ((-(source[6].xxxx))*(source[2].zwzz)+(v2.xyxx)).xy;
    // 11: mul r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)*(source[2].xyxx)).xy;
    // 12: mul r1.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r1.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r1.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 14: mad r1.xy, r1.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 15: dp2 r1.w, r1.xyxx, r1.xyxx
    r1.w = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).w;
    // 16: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 17: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 18: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 19: add r1.z, r0.w, r1.w
    r1.z = ((r0.wwww)+(r1.wwww)).z;
    // 20: mov r0.z, l(0.000010)
    r0.z = (float4(0.000010,0.000010,0.000010,0.000010)).z;
    // 21: add r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)+(r1.xyzx)).xyz;
    // 22: mul r1.xyzw, r0.xyxy, cb0[6].yyzz
    r1.xyzw = ((r0.xyxy)*(source[6].yyzz)).xyzw;
    // 23: mad r2.xy, cb0[6].xxxx, cb0[4].zwzz, v2.xyxx
    r2.xy = ((source[6].xxxx)*(source[4].zwzz)+(v2.xyxx)).xy;
    // 24: mad r1.zw, r2.xxxy, cb0[4].xxxy, r1.zzzw
    r1.zw = ((r2.xxxy)*(source[4].xxxy)+(r1.zzzw)).zw;
    // 25: mov r0.xy, r1.xyxx
    r0.xy = (r1.xyxx).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r1.zwzz, t1.xyzw, s2, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r1.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 27: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 28: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r0.w
    r1.w = (sqrt(r0.wwww)).w;
    // 30: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 31: mul r2.xyz, r0.wwww, r0.xyzx
    r2.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 32: div r0.xyz, r0.xyzx, r1.wwww
    r0.xyz = ((r0.xyzx)/(r1.wwww)).xyz;
    // 33: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 34: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 35: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 36: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 37: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 38: mad r0.x, -r0.x, l(0.500000), l(1.000000)
    r0.x = ((-(r0.xxxx))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul_sat r0.x, r0.x, cb0[7].x
    r0.x = (saturate((r0.xxxx)*(source[7].xxxx))).x;
    // 40: dp3 r0.y, r2.xyzx, r3.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 41: mul r0.yzw, r0.yyyy, r2.xxyz
    r0.yzw = ((r0.yyyy)*(r2.xxyz)).yzw;
    // 42: mad r0.yzw, r0.yyzw, l(0.000000, 2.000000, 2.000000, 2.000000), -r3.xxyz
    r0.yzw = ((r0.yyzw)*(float4(0.000000,2.000000,2.000000,2.000000))+(-(r3.xxyz))).yzw;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r0.yzyy, t2.xyzw, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 44: mad r3.xyz, cb0[5].xyzx, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[5].xyzx)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 45: mad r1.xyz, r0.xxxx, r3.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 46: add r3.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 47: mul r1.xyz, r1.xyzx, r3.xyzx
    r1.xyz = ((r1.xyzx)*(r3.xyzx)).xyz;
    // 48: mad r1.xyz, r1.xyzx, cb2[3].wwww, cb2[3].xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 49: dp3 r0.x, v3.xyzx, v3.xyzx
    r0.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 50: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 51: mul r3.xyz, r0.xxxx, v3.xyzx
    r3.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 52: dp3_sat r0.x, r0.yzwy, r3.xyzx
    r0.x = (saturate(dot((r0.yzwy).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 53: dp3_sat r0.y, r2.xyzx, r3.xyzx
    r0.y = (saturate(dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx)).y;
    // 54: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 55: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 56: mul r0.z, r0.z, l(15.000000)
    r0.z = ((r0.zzzz)*(float4(15.000000,15.000000,15.000000,15.000000))).z;
    // 57: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 58: mul r2.xyz, r0.zzzz, cb2[4].xyzx
    r2.xyz = ((r0.zzzz)*(passValues[4].xyzx)).xyz;
    // 59: movc r0.xzw, r0.xxxx, l(0,0,0,0), r2.xxyz
    r0.xzw = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxyz)).xzw;
    // 60: lt r1.w, r0.y, l(0.000001)
    r1.w = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 61: movc r0.y, r1.w, l(0), r0.y
    r0.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 62: mad r0.xyz, r1.xyzx, r0.yyyy, r0.xzwx
    r0.xyz = ((r1.xyzx)*(r0.yyyy)+(r0.xzwx)).xyz;
    // 63: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 64: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 65: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 66: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t4.xyzw, s0, l(0.000000)
    r0.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 67: min r0.x, r0.x, l(0.999000)
    r0.x = (min(r0.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 68: mad r0.y, r0.x, cb2[1].z, -cb2[1].w
    r0.y = ((r0.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).y;
    // 69: mad r0.x, r0.x, cb2[1].x, cb2[1].y
    r0.x = ((r0.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).x;
    // 70: div r0.y, l(1.000000, 1.000000, 1.000000, 1.000000), r0.y
    r0.y = (((r0.yyyy) != 0.f ? 1.f / (r0.yyyy) : 0.f)).y;
    // 71: add r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)+(r0.xxxx)).x;
    // 72: add r0.x, r0.x, -v6.w
    r0.x = ((r0.xxxx)+(-(v6.wwww))).x;
    // 73: add r0.y, -cb0[7].z, l(1.000000)
    r0.y = ((-(source[7].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 74: max r0.y, r0.y, l(0.001000)
    r0.y = (max(r0.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 75: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v2.xyxx, t3.yxzw, s4, l(0.000000)
    r0.y = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yxzw).y;
    // 77: mul_sat r0.y, r0.y, cb0[7].y
    r0.y = (saturate((r0.yyyy)*(source[7].yyyy))).y;
    // 78: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 79: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 80: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 81: ret
    return output;
}
