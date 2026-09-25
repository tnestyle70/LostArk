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

// source.character.kouku-bazooka-dead.v1 / source program da7621e9468b8d4d9d69090a334b5828
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight238(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[8]=SourceCharacterAppend(g_SourceCharacterTime.xxxx,g_SourceCharacterTime.xxxx,1u);
    source[10]=SourceCharacterAppend(cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),(float4(-1,0,0,0)*sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0)))),1u);
    source[11]=SourceCharacterAppend(sin((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),cos((g_SourceCharacterTime.xxxx*float4(0.100000001,0,0,0))),1u);
    source[16].z=(g_SourceCharacterTime.xxxx).x;
    source[20]=float4(input.lightColor,1.0);
    source[21].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0, r12=0.0, r13=0.0, r14=0.0;
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
    // 10: add r3.xyzw, v8.yzxy, cb0[0].yzxy
    r3.xyzw = ((v8.yzxy)+(source[0].yzxy)).xyzw;
    // 11: mul r4.xyz, v8.yyyy, cb1[1].xywx
    r4.xyz = ((v8.yyyy)*(projection[1].xywx)).xyz;
    // 12: mad r4.xyz, cb1[0].xywx, v8.xxxx, r4.xyzx
    r4.xyz = ((projection[0].xywx)*(v8.xxxx)+(r4.xyzx)).xyz;
    // 13: mad r4.xyz, cb1[2].xywx, v8.zzzz, r4.xyzx
    r4.xyz = ((projection[2].xywx)*(v8.zzzz)+(r4.xyzx)).xyz;
    // 14: mad r4.xyz, cb1[3].xywx, v8.wwww, r4.xyzx
    r4.xyz = ((projection[3].xywx)*(v8.wwww)+(r4.xyzx)).xyz;
    // 15: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[21].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[21].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 17: div r5.xy, r4.xyxx, r4.zzzz
    r5.xy = ((r4.xyxx)/(r4.zzzz)).xy;
    // 18: mad r5.xy, r5.xyxx, cb2[0].xyxx, cb2[0].wzww
    r5.xy = ((r5.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t8.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 25: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 26: mul r6.xyz, r0.wwww, v7.xyzx
    r6.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 27: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 28: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 29: mul r7.xyz, r0.wwww, v5.xyzx
    r7.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 31: mul r9.xy, v4.xyxx, cb0[17].xxxx
    r9.xy = ((v4.xyxx)*(source[17].xxxx)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r1.w, r9.xyxx, t2.yzwx, s7, l(0.000000)
    r1.w = ((g_SourceCharacterTexture7.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 33: add r1.w, r1.w, -cb0[17].y
    r1.w = ((r1.wwww)+(-(source[17].yyyy))).w;
    // 34: round_pi_sat r1.w, r1.w
    r1.w = (saturate(ceil(r1.wwww))).w;
    // 35: mul_sat r1.w, r1.w, r8.w
    r1.w = (saturate((r1.wwww)*(r8.wwww))).w;
    // 36: add r1.w, r1.w, l(-0.333300)
    r1.w = ((r1.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 37: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 38: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 39: sample_b_indexable(texture2d)(float,float,float,float) r9.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r9.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 40: mad r9.xy, r9.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r9.xy = ((r9.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: mul r10.xy, r9.xyxx, cb0[12].xxxx
    r10.xy = ((r9.xyxx)*(source[12].xxxx)).xy;
    // 42: dp2 r1.w, r9.xyxx, r9.xyxx
    r1.w = (dot((r9.xyxx).xy,(r9.xyxx).xy).xxxx).w;
    // 43: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 44: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 45: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 46: add r10.z, r1.w, l(0.000010)
    r10.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 47: add r9.xyz, -r10.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r10.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 48: mad r9.xyz, cb0[12].wwww, r9.xyzx, r10.xyzx
    r9.xyz = ((source[12].wwww)*(r9.xyzx)+(r10.xyzx)).xyz;
    // 49: dp3 r1.w, r9.xyzx, r9.xyzx
    r1.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 50: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 51: div r9.xyz, r9.xyzx, r1.wwww
    r9.xyz = ((r9.xyzx)/(r1.wwww)).xyz;
    // 52: dp3 r11.x, r1.xyzx, r9.xyzx
    r11.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 53: dp3 r11.y, r2.xyzx, r9.xyzx
    r11.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 54: dp3 r11.z, r0.xyzx, r9.xyzx
    r11.z = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).z;
    // 55: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 56: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 57: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 58: dp3 r0.x, r11.xyzx, r1.xyzx
    r0.x = (dot((r11.xyzx).xyz,(r1.xyzx).xyz).xxxx).x;
    // 59: mul r0.xyz, r11.xyzx, r0.xxxx
    r0.xyz = ((r11.xyzx)*(r0.xxxx)).xyz;
    // 60: mad r1.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r0.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 61: mov r1.w, -r1.x
    r1.w = (-(r1.xxxx)).w;
    // 62: dp2 r0.x, r1.ywyy, r1.ywyy
    r0.x = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).x;
    // 63: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 64: div r0.xy, r1.ywyy, r0.xxxx
    r0.xy = ((r1.ywyy)/(r0.xxxx)).xy;
    // 65: mad r0.z, -r1.z, l(0.250000), l(0.250000)
    r0.z = ((-(r1.zzzz))*(float4(0.250000,0.250000,0.250000,0.250000))+(float4(0.250000,0.250000,0.250000,0.250000))).z;
    // 66: mad r0.xy, r0.zzzz, r0.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.zzzz)*(r0.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t3.xyzw, s3, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 68: add r0.z, -r2.w, l(1.000000)
    r0.z = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: lt r1.x, |r0.z|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 70: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 71: mul r0.z, r0.z, cb0[13].x
    r0.z = ((r0.zzzz)*(source[13].xxxx)).z;
    // 72: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 73: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 74: movc r0.z, r1.x, l(0), r0.z
    r0.z = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 75: sqrt r1.x, r0.z
    r1.x = (sqrt(r0.zzzz)).x;
    // 76: mul r1.x, r1.x, cb0[13].y
    r1.x = ((r1.xxxx)*(source[13].yyyy)).x;
    // 77: div r1.yw, r4.xxxy, r4.zzzz
    r1.yw = ((r4.xxxy)/(r4.zzzz)).yw;
    // 78: mad r1.yw, r1.yyyw, cb2[0].xxxy, cb2[0].wwwz
    r1.yw = ((r1.yyyw)*(passValues[0].xxxy)+(passValues[0].wwwz)).yw;
    // 79: mul r1.yw, r1.yyyw, l(0.000000, 700.000000, 0.000000, 700.000000)
    r1.yw = ((r1.yyyw)*(float4(0.000000,700.000000,0.000000,700.000000))).yw;
    // 80: deriv_rtx_coarse r4.xy, r1.ywyy
    r4.xy = (ddx_coarse(r1.ywyy)).xy;
    // 81: deriv_rty_coarse r1.yw, r1.yyyw
    r1.yw = (ddy_coarse(r1.yyyw)).yw;
    // 82: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 83: dp2 r1.y, r1.ywyy, r1.ywyy
    r1.y = (dot((r1.ywyy).xy,(r1.ywyy).xy).xxxx).y;
    // 84: max r1.y, r1.y, r2.w
    r1.y = (max(r1.yyyy,r2.wwww)).y;
    // 85: sqrt r1.y, r1.y
    r1.y = (sqrt(r1.yyyy)).y;
    // 86: log r1.y, r1.y
    r1.y = (log2(r1.yyyy)).y;
    // 87: rcp r1.w, |r1.y|
    r1.w = (1.0/(abs(r1.yyyy))).w;
    // 88: mul r1.x, r1.w, r1.x
    r1.x = ((r1.wwww)*(r1.xxxx)).x;
    // 89: sqrt r1.x, r1.x
    r1.x = (sqrt(r1.xxxx)).x;
    // 90: add r1.x, r1.x, |r1.y|
    r1.x = ((r1.xxxx)+(abs(r1.yyyy))).x;
    // 91: round_ni r1.x, r1.x
    r1.x = (floor(r1.xxxx)).x;
    // 92: sample_l_indexable(texture2d)(float,float,float,float) r1.xyw, r0.xyxx, t4.xywz, s4, r1.x
    r1.xyw = ((g_SourceCharacterTexture3.SampleLevel(SourceCharacterLookupSampler, (r0.xyxx).xy, (r1.xxxx).x)).xywz).xyw;
    // 93: rcp r0.x, cb0[13].z
    r0.x = (1.0/(source[13].zzzz)).x;
    // 94: log r4.xyz, r1.xywx
    r4.xyz = (log2(r1.xywx)).xyz;
    // 95: mul r9.xyz, r4.xyzx, cb0[13].zzzz
    r9.xyz = ((r4.xyzx)*(source[13].zzzz)).xyz;
    // 96: exp r9.xyz, r9.xyzx
    r9.xyz = (exp2(r9.xyzx)).xyz;
    // 97: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 98: exp r4.xyz, r4.xyzx
    r4.xyz = (exp2(r4.xyzx)).xyz;
    // 99: mul r4.xyz, r0.xxxx, r4.xyzx
    r4.xyz = ((r0.xxxx)*(r4.xyzx)).xyz;
    // 100: mad r4.xyz, r9.xyzx, cb0[13].zzzz, r4.xyzx
    r4.xyz = ((r9.xyzx)*(source[13].zzzz)+(r4.xyzx)).xyz;
    // 101: add r1.xyw, r1.xyxw, r4.xyxz
    r1.xyw = ((r1.xyxw)+(r4.xyxz)).xyw;
    // 102: mul r1.xyw, r1.xyxw, l(0.333333, 0.333333, 0.000000, 0.333333)
    r1.xyw = ((r1.xyxw)*(float4(0.333333,0.333333,0.000000,0.333333))).xyw;
    // 103: add r0.x, cb0[13].z, l(1.000000)
    r0.x = ((source[13].zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 104: mul r1.xyw, r0.xxxx, r1.xyxw
    r1.xyw = ((r0.xxxx)*(r1.xyxw)).xyw;
    // 105: mad r0.xyw, v5.xyxz, r0.wwww, r6.xyxz
    r0.xyw = ((v5.xyxz)*(r0.wwww)+(r6.xyxz)).xyw;
    // 106: dp3 r2.w, r0.xywx, r0.xywx
    r2.w = (dot((r0.xywx).xyz,(r0.xywx).xyz).xxxx).w;
    // 107: sqrt r4.x, r2.w
    r4.x = (sqrt(r2.wwww)).x;
    // 108: div r0.xyw, r0.xyxw, r4.xxxx
    r0.xyw = ((r0.xyxw)/(r4.xxxx)).xyw;
    // 109: dp3 r4.x, r0.xywx, r6.xyzx
    r4.x = (dot((r0.xywx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 110: add r4.x, -r4.x, l(1.000000)
    r4.x = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 111: lt r4.y, |r4.x|, l(0.000001)
    r4.y = (asfloat((uint4)((abs(r4.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 112: mul r4.z, |r4.x|, |r4.x|
    r4.z = ((abs(r4.xxxx))*(abs(r4.xxxx))).z;
    // 113: mul r4.z, r4.z, r4.z
    r4.z = ((r4.zzzz)*(r4.zzzz)).z;
    // 114: mul r4.x, r4.z, |r4.x|
    r4.x = ((r4.zzzz)*(abs(r4.xxxx))).x;
    // 115: movc r4.x, r4.y, l(0), r4.x
    r4.x = ((asuint(r4.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r4.xxxx)).x;
    // 116: add r4.y, r4.x, l(-0.027778)
    r4.y = ((r4.xxxx)+(float4(-0.027778,-0.027778,-0.027778,-0.027778))).y;
    // 117: mad r4.x, r4.x, r4.y, l(0.027778)
    r4.x = ((r4.xxxx)*(r4.yyyy)+(float4(0.027778,0.027778,0.027778,0.027778))).x;
    // 118: div r2.w, r4.x, r2.w
    r2.w = ((r4.xxxx)/(r2.wwww)).w;
    // 119: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 120: min r4.xy, r2.wwww, l(1.000000, 3.000000, 0.000000, 0.000000)
    r4.xy = (min(r2.wwww,float4(1.000000,3.000000,0.000000,0.000000))).xy;
    // 121: add r2.w, -r4.x, l(1.000000)
    r2.w = ((-(r4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: mul r4.x, r0.z, r2.w
    r4.x = ((r0.zzzz)*(r2.wwww)).x;
    // 123: mul r9.xyz, r1.xywx, r4.xxxx
    r9.xyz = ((r1.xywx)*(r4.xxxx)).xyz;
    // 124: mul r11.xyz, r9.xyzx, cb0[17].wwww
    r11.xyz = ((r9.xyzx)*(source[17].wwww)).xyz;
    // 125: dp3 r4.z, r10.xyzx, r10.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r10.xyzx).xyz).xxxx).z;
    // 126: sqrt r4.z, r4.z
    r4.z = (sqrt(r4.zzzz)).z;
    // 127: div r10.xyz, r10.xyzx, r4.zzzz
    r10.xyz = ((r10.xyzx)/(r4.zzzz)).xyz;
    // 128: dp3 r4.z, r10.xyzx, r7.xyzx
    r4.z = (dot((r10.xyzx).xyz,(r7.xyzx).xyz).xxxx).z;
    // 129: max r4.w, r4.z, l(0.000000)
    r4.w = (max(r4.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 130: min r5.w, r4.w, l(1.000000)
    r5.w = (min(r4.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 131: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 132: mad r4.z, r4.z, l(0.500000), -r5.w
    r4.z = ((r4.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(-(r5.wwww))).z;
    // 133: mad r4.z, r0.z, r4.z, r5.w
    r4.z = ((r0.zzzz)*(r4.zzzz)+(r5.wwww)).z;
    // 134: mad r2.w, -r0.z, r2.w, l(1.000000)
    r2.w = ((-(r0.zzzz))*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 135: dp3 r6.x, r10.xyzx, r6.xyzx
    r6.x = (dot((r10.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 136: mul_sat r6.xy, r6.xzxx, cb0[14].yyyy
    r6.xy = (saturate((r6.xzxx)*(source[14].yyyy))).xy;
    // 137: add r6.xy, -r6.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r6.xy = ((-(r6.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 138: add_sat r6.y, r6.y, -cb0[14].z
    r6.y = (saturate((r6.yyyy)+(-(source[14].zzzz)))).y;
    // 139: lt r6.z, r6.y, l(0.000001)
    r6.z = (asfloat((uint4)((r6.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 140: log r6.y, r6.y
    r6.y = (log2(r6.yyyy)).y;
    // 141: mul r6.y, r6.y, cb0[14].w
    r6.y = ((r6.yyyy)*(source[14].wwww)).y;
    // 142: exp r6.y, r6.y
    r6.y = (exp2(r6.yyyy)).y;
    // 143: mul r6.x, r6.y, r6.x
    r6.x = ((r6.yyyy)*(r6.xxxx)).x;
    // 144: movc r6.x, r6.z, l(0), r6.x
    r6.x = ((asuint(r6.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r6.xxxx)).x;
    // 145: add r6.y, -r6.x, l(1.000000)
    r6.y = ((-(r6.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 146: mul r6.y, r4.z, r6.y
    r6.y = ((r4.zzzz)*(r6.yyyy)).y;
    // 147: mad r6.z, r2.w, l(2.000000), -r4.x
    r6.z = ((r2.wwww)*(float4(2.000000,2.000000,2.000000,2.000000))+(-(r4.xxxx))).z;
    // 148: mad r6.y, r6.y, r6.z, r4.x
    r6.y = ((r6.yyyy)*(r6.zzzz)+(r4.xxxx)).y;
    // 149: add r6.y, r6.y, l(1.000000)
    r6.y = ((r6.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 150: mul_sat r6.y, r4.z, r6.y
    r6.y = (saturate((r4.zzzz)*(r6.yyyy))).y;
    // 151: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 152: mul r0.z, r0.z, cb0[18].y
    r0.z = ((r0.zzzz)*(source[18].yyyy)).z;
    // 153: mad r6.y, cb0[18].x, r6.y, -r4.z
    r6.y = ((source[18].xxxx)*(r6.yyyy)+(-(r4.zzzz))).y;
    // 154: mad r0.z, r0.z, r6.y, r4.z
    r0.z = ((r0.zzzz)*(r6.yyyy)+(r4.zzzz)).z;
    // 155: sqrt r4.z, r5.w
    r4.z = (sqrt(r5.wwww)).z;
    // 156: mul r5.xyz, r5.xyzx, r4.zzzz
    r5.xyz = ((r5.xyzx)*(r4.zzzz)).xyz;
    // 157: mul r5.xyz, r0.zzzz, r5.xyzx
    r5.xyz = ((r0.zzzz)*(r5.xyzx)).xyz;
    // 158: mad r6.yzw, -cb0[17].wwww, r9.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r6.yzw = ((-(source[17].wwww))*(r9.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 159: mad r6.yzw, r5.xxyz, r6.yyzw, r11.xxyz
    r6.yzw = ((r5.xxyz)*(r6.yyzw)+(r11.xxyz)).yzw;
    // 160: mul r7.xyz, cb0[3].xyzx, cb0[3].wwww
    r7.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 161: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 162: mad r9.xyz, -cb0[3].wwww, cb0[3].xyzx, r0.zzzz
    r9.xyz = ((-(source[3].wwww))*(source[3].xyzx)+(r0.zzzz)).xyz;
    // 163: mad r7.xyz, cb0[12].yyyy, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].yyyy)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 164: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 165: add r9.xyz, -r7.xyzx, r0.zzzz
    r9.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 166: mad r7.xyz, cb0[12].zzzz, r9.xyzx, r7.xyzx
    r7.xyz = ((source[12].zzzz)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 167: mad r9.xyz, cb0[4].wwww, cb0[4].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r9.xyz = ((source[4].wwww)*(source[4].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 168: mad r11.xyz, cb0[5].wwww, cb0[5].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r11.xyz = ((source[5].wwww)*(source[5].xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 170: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 171: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 172: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 173: mad r8.xyz, cb0[12].yyyy, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].yyyy)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 174: dp3 r0.z, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 175: add r11.xyz, -r8.xyzx, r0.zzzz
    r11.xyz = ((-(r8.xyzx))+(r0.zzzz)).xyz;
    // 176: mad r8.xyz, cb0[12].zzzz, r11.xyzx, r8.xyzx
    r8.xyz = ((source[12].zzzz)*(r11.xyzx)+(r8.xyzx)).xyz;
    // 177: mul r11.xyz, r7.xyzx, r8.xyzx
    r11.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 178: dp3 r0.z, r1.xywx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r1.xywx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 179: add r1.x, r1.z, l(1.000000)
    r1.x = ((r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 180: mul r1.x, r1.x, l(0.500000)
    r1.x = ((r1.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 181: add r1.yzw, -cb0[6].xxyz, cb0[7].xxyz
    r1.yzw = ((-(source[6].xxyz))+(source[7].xxyz)).yzw;
    // 182: mad r1.xyz, r1.xxxx, r1.yzwy, cb0[6].xyzx
    r1.xyz = ((r1.xxxx)*(r1.yzwy)+(source[6].xyzx)).xyz;
    // 183: mul r1.xyz, r0.zzzz, r1.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)).xyz;
    // 184: mul r1.xyz, r1.xyzx, cb0[13].wwww
    r1.xyz = ((r1.xyzx)*(source[13].wwww)).xyz;
    // 185: mul r12.xyz, r1.xyzx, r11.xyzx
    r12.xyz = ((r1.xyzx)*(r11.xyzx)).xyz;
    // 186: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 187: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 188: mad r2.xyz, cb0[12].yyyy, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].yyyy)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 189: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 190: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 191: mad r2.xyz, cb0[12].zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((source[12].zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 192: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 193: add r13.xyz, -r2.xyzx, r0.zzzz
    r13.xyz = ((-(r2.xyzx))+(r0.zzzz)).xyz;
    // 194: mul r13.xyz, r13.xyzx, cb0[14].xxxx
    r13.xyz = ((r13.xyzx)*(source[14].xxxx)).xyz;
    // 195: sample_b_indexable(texture2d)(float,float,float,float) r14.xyzw, v4.xyxx, t5.xyzw, s5, l(0.000000)
    r14.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 196: add r0.z, r14.y, r14.x
    r0.z = ((r14.yyyy)+(r14.xxxx)).z;
    // 197: add r0.z, r14.z, r0.z
    r0.z = ((r14.zzzz)+(r0.zzzz)).z;
    // 198: add_sat r0.z, r14.w, r0.z
    r0.z = (saturate((r14.wwww)+(r0.zzzz))).z;
    // 199: mad r2.xyz, r0.zzzz, r13.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r13.xyzx)+(r2.xyzx)).xyz;
    // 200: max r13.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r13.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 201: log r13.xyz, r13.xyzx
    r13.xyz = (log2(r13.xyzx)).xyz;
    // 202: mul r13.xyz, r13.xyzx, l(0.454545, 0.454545, 0.454545, 0.000000)
    r13.xyz = ((r13.xyzx)*(float4(0.454545,0.454545,0.454545,0.000000))).xyz;
    // 203: exp r13.xyz, r13.xyzx
    r13.xyz = (exp2(r13.xyzx)).xyz;
    // 204: dp3 r0.z, r13.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r13.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 205: log r0.z, r0.z
    r0.z = (log2(r0.zzzz)).z;
    // 206: mul r0.z, r0.z, cb0[15].x
    r0.z = ((r0.zzzz)*(source[15].xxxx)).z;
    // 207: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 208: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 209: mad r1.w, -r0.z, r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))*(r0.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 210: max r1.w, r1.w, l(0.001000)
    r1.w = (max(r1.wwww,float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 211: div r1.w, cb0[15].y, r1.w
    r1.w = ((source[15].yyyy)/(r1.wwww)).w;
    // 212: mul r1.w, r1.w, r6.x
    r1.w = ((r1.wwww)*(r6.xxxx)).w;
    // 213: mul r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)*(r1.wwww)).xyz;
    // 214: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 215: mul r1.w, r1.w, cb0[15].z
    r1.w = ((r1.wwww)*(source[15].zzzz)).w;
    // 216: mad r1.xyz, r2.xyzx, r1.xyzx, -r12.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)+(-(r12.xyzx))).xyz;
    // 217: mad r1.xyz, r1.wwww, r1.xyzx, r12.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r12.xyzx)).xyz;
    // 218: mad r1.xyz, r4.xxxx, r1.xyzx, -r11.xyzx
    r1.xyz = ((r4.xxxx)*(r1.xyzx)+(-(r11.xyzx))).xyz;
    // 219: mad r1.xyz, r0.zzzz, r1.xyzx, r11.xyzx
    r1.xyz = ((r0.zzzz)*(r1.xyzx)+(r11.xyzx)).xyz;
    // 220: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 221: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 222: mad r1.xyz, cb0[12].yyyy, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].yyyy)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 223: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 224: add r11.xyz, -r1.xyzx, r1.wwww
    r11.xyz = ((-(r1.xyzx))+(r1.wwww)).xyz;
    // 225: mad r1.xyz, cb0[12].zzzz, r11.xyzx, r1.xyzx
    r1.xyz = ((source[12].zzzz)*(r11.xyzx)+(r1.xyzx)).xyz;
    // 226: mul r1.xyz, r9.xyzx, r1.xyzx
    r1.xyz = ((r9.xyzx)*(r1.xyzx)).xyz;
    // 227: mul r1.w, cb0[2].z, l(1.500000)
    r1.w = ((source[2].zzzz)*(float4(1.500000,1.500000,1.500000,1.500000))).w;
    // 228: add r4.z, -cb0[2].w, l(1.000000)
    r4.z = ((-(source[2].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 229: mul r4.z, r4.z, cb0[16].z
    r4.z = ((r4.zzzz)*(source[16].zzzz)).z;
    // 230: mul r4.z, r4.z, l(6.283185)
    r4.z = ((r4.zzzz)*(float4(6.283185,6.283185,6.283185,6.283185))).z;
    // 231: sincos r4.z, null, r4.z
    r4.z = (sin(r4.zzzz)).z;
    // 232: add r4.z, r4.z, l(1.000000)
    r4.z = ((r4.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 233: mul r1.w, r1.w, r4.z
    r1.w = ((r1.wwww)*(r4.zzzz)).w;
    // 234: mad r1.w, r1.w, l(0.500000), cb0[2].z
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[2].zzzz)).w;
    // 235: frc r4.z, cb0[2].x
    r4.z = (frac(source[2].xxxx)).z;
    // 236: add r6.x, -r4.z, cb0[2].x
    r6.x = ((-(r4.zzzz))+(source[2].xxxx)).x;
    // 237: mul r9.z, r6.x, l(0.125000)
    r9.z = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).z;
    // 238: mov r9.xw, l(0,0,0,0)
    r9.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 239: mul r9.y, cb0[2].y, cb0[8].y
    r9.y = ((source[2].yyyy)*(source[8].yyyy)).y;
    // 240: frc r6.x, v4.x
    r6.x = (frac(v4.xxxx)).x;
    // 241: mul r11.x, r6.x, l(0.125000)
    r11.x = ((r6.xxxx)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 242: mov r11.y, v4.y
    r11.y = (v4.yyyy).y;
    // 243: add r9.xy, r9.xyxx, r11.xyxx
    r9.xy = ((r9.xyxx)+(r11.xyxx)).xy;
    // 244: add r9.xy, r9.xyxx, r9.zwzz
    r9.xy = ((r9.xyxx)+(r9.zwzz)).xy;
    // 245: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r9.xyxx, t6.xyzw, s6, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r9.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 246: mul r9.xyz, r1.wwww, r9.xyzx
    r9.xyz = ((r1.wwww)*(r9.xyzx)).xyz;
    // 247: mul r1.w, r4.z, r9.w
    r1.w = ((r4.zzzz)*(r9.wwww)).w;
    // 248: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 249: mad r1.xyz, r1.wwww, r9.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r9.xyzx)+(r1.xyzx)).xyz;
    // 250: mul r1.w, cb0[9].y, cb0[16].z
    r1.w = ((source[9].yyyy)*(source[16].zzzz)).w;
    // 251: mul r1.w, r1.w, l(0.628319)
    r1.w = ((r1.wwww)*(float4(0.628319,0.628319,0.628319,0.628319))).w;
    // 252: sincos r1.w, null, r1.w
    r1.w = (sin(r1.wwww)).w;
    // 253: mul r9.y, r1.w, l(0.020000)
    r9.y = ((r1.wwww)*(float4(0.020000,0.020000,0.020000,0.020000))).y;
    // 254: add r3.xyzw, r3.xyzw, -cb0[1].yzxy
    r3.xyzw = ((r3.xyzw)+(-(source[1].yzxy))).xyzw;
    // 255: add r3.xy, -r3.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r3.xy = ((-(r3.xyxx))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 256: add r3.xy, -r3.zwzz, r3.xyxx
    r3.xy = ((-(r3.zwzz))+(r3.xyxx)).xy;
    // 257: mad r3.xy, cb0[9].wwww, r3.xyxx, r3.zwzz
    r3.xy = ((source[9].wwww)*(r3.xyxx)+(r3.zwzz)).xy;
    // 258: mul r3.z, cb0[9].x, l(0.001000)
    r3.z = ((source[9].xxxx)*(float4(0.001000,0.001000,0.001000,0.001000))).z;
    // 259: mov r9.x, l(0)
    r9.x = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).x;
    // 260: mad r3.xy, r3.zzzz, r3.xyxx, r9.xyxx
    r3.xy = ((r3.zzzz)*(r3.xyxx)+(r9.xyxx)).xy;
    // 261: dp2 r3.z, cb0[10].xyxx, r3.xyxx
    r3.z = (dot((source[10].xyxx).xy,(r3.xyxx).xy).xxxx).z;
    // 262: dp2 r3.y, cb0[11].xyxx, r3.xyxx
    r3.y = (dot((source[11].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 263: frc r3.z, r3.z
    r3.z = (frac(r3.zzzz)).z;
    // 264: mul r3.x, r3.z, l(0.125000)
    r3.x = ((r3.zzzz)*(float4(0.125000,0.125000,0.125000,0.125000))).x;
    // 265: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r3.xyxx, t6.xyzw, s6, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 266: mul r3.w, r3.w, l(0.900000)
    r3.w = ((r3.wwww)*(float4(0.900000,0.900000,0.900000,0.900000))).w;
    // 267: mad r3.xyz, r3.xyzx, l(3.500000, 3.500000, 3.500000, 0.000000), -r1.xyzx
    r3.xyz = ((r3.xyzx)*(float4(3.500000,3.500000,3.500000,0.000000))+(-(r1.xyzx))).xyz;
    // 268: mad r3.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r3.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 269: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 270: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 271: mul_sat r3.xyz, r3.xyzx, r1.wwww
    r3.xyz = (saturate((r3.xyzx)*(r1.wwww))).xyz;
    // 272: mul r9.xyz, r3.xyzx, cb0[9].zzzz
    r9.xyz = ((r3.xyzx)*(source[9].zzzz)).xyz;
    // 273: dp3 r1.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 274: mul r1.w, r1.w, l(3.000000)
    r1.w = ((r1.wwww)*(float4(3.000000,3.000000,3.000000,3.000000))).w;
    // 275: mad r3.xyz, cb0[9].zzzz, r3.xyzx, -r1.xyzx
    r3.xyz = ((source[9].zzzz)*(r3.xyzx)+(-(r1.xyzx))).xyz;
    // 276: mad r1.xyz, r1.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r1.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 277: max r3.xyz, r5.xyzx, l(0.500000, 0.500000, 0.500000, 0.000000)
    r3.xyz = (max(r5.xyzx,float4(0.500000,0.500000,0.500000,0.000000))).xyz;
    // 278: min r3.xyz, r3.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = (min(r3.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 279: mad r5.xyz, r7.xyzx, r8.xyzx, -r2.xyzx
    r5.xyz = ((r7.xyzx)*(r8.xyzx)+(-(r2.xyzx))).xyz;
    // 280: mad r2.xyz, r0.zzzz, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 281: dp3 r0.x, r0.xywx, r10.xyzx
    r0.x = (dot((r0.xywx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 282: mul r0.y, r2.w, cb0[18].z
    r0.y = ((r2.wwww)*(source[18].zzzz)).y;
    // 283: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t7.xyzw, s8, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture8.SampleBias(SourceCharacterLookupSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 284: add r0.w, -cb0[18].w, l(2.000000)
    r0.w = ((-(source[18].wwww))+(float4(2.000000,2.000000,2.000000,2.000000))).w;
    // 285: mad r0.w, r4.x, r0.w, cb0[18].w
    r0.w = ((r4.xxxx)*(r0.wwww)+(source[18].wwww)).w;
    // 286: mul r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 287: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 288: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 289: mul r0.xyz, r0.xyzx, cb0[19].xxxx
    r0.xyz = ((r0.xyzx)*(source[19].xxxx)).xyz;
    // 290: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 291: mul r0.xyz, r0.xyzx, r4.yyyy
    r0.xyz = ((r0.xyzx)*(r4.yyyy)).xyz;
    // 292: mul r0.xyz, r0.xyzx, r5.wwww
    r0.xyz = ((r0.xyzx)*(r5.wwww)).xyz;
    // 293: min r0.xyz, r0.xyzx, l(3.000000, 3.000000, 3.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(3.000000,3.000000,3.000000,0.000000))).xyz;
    // 294: mul r0.xyz, r0.xyzx, cb0[19].yyyy
    r0.xyz = ((r0.xyzx)*(source[19].yyyy)).xyz;
    // 295: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 296: mul r0.xyz, r0.xyzx, r3.xyzx
    r0.xyz = ((r0.xyzx)*(r3.xyzx)).xyz;
    // 297: mad r0.xyz, r6.yzwy, r1.xyzx, r0.xyzx
    r0.xyz = ((r6.yzwy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 298: mul r1.xyz, r4.wwww, cb2[3].xyzx
    r1.xyz = ((r4.wwww)*(passValues[3].xyzx)).xyz;
    // 299: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 300: mul o0.xyz, r0.xyzx, cb0[20].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[20].xyzx)).xyz;
    // 301: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 302: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 303: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 304: ret
    return output;
}

// source.character.selection-native-600.v1 / source program e36b84e020d38e408af80c720418b668
