// source.character.static-map-native-224.v1 / source program a54c39add512e849b1b65e7fc6b1ba55
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight224(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[4]=float4(input.lightColor,1.0);
    source[10].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v5.xyzx
    r0.xyz = ((r0.xxxx)*(v5.xyzx)).xyz;
    // 4: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v3.xyzx
    r1.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // 7: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].yyyy)) * 0xffffffffu)).w;
    // 8: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 9: mul r2.xyzw, v6.yyyy, cb0[6].xyzw
    r2.xyzw = ((v6.yyyy)*(source[6].xyzw)).xyzw;
    // 10: mad r2.xyzw, cb0[5].xyzw, v6.xxxx, r2.xyzw
    r2.xyzw = ((source[5].xyzw)*(v6.xxxx)+(r2.xyzw)).xyzw;
    // 11: mad r2.xyzw, cb0[7].xyzw, v6.zzzz, r2.xyzw
    r2.xyzw = ((source[7].xyzw)*(v6.zzzz)+(r2.xyzw)).xyzw;
    // 12: mad r2.xyzw, cb0[8].xyzw, v6.wwww, r2.xyzw
    r2.xyzw = ((source[8].xyzw)*(v6.wwww)+(r2.xyzw)).xyzw;
    // 13: div r2.xy, r2.xyxx, r2.wwww
    r2.xy = ((r2.xyxx)/(r2.wwww)).xy;
    // 14: sample_indexable(texture2d)(float,float,float,float) r3.x, r2.xyxx, t1.xyzw, s3
    r3.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 15: mov r4.xw, l(0,0,0,0)
    r4.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 16: mov r4.yz, cb0[9].wwzw
    r4.yz = (source[9].wwzw).yz;
    // 17: add r4.xyzw, r2.xyxy, r4.xyzw
    r4.xyzw = ((r2.xyxy)+(r4.xyzw)).xyzw;
    // 18: sample_indexable(texture2d)(float,float,float,float) r3.y, r4.xyxx, t1.yxzw, s3
    r3.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 19: sample_indexable(texture2d)(float,float,float,float) r3.z, r4.zwzz, t1.yzxw, s3
    r3.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 20: add r4.xy, r2.xyxx, cb0[9].zwzz
    r4.xy = ((r2.xyxx)+(source[9].zwzz)).xy;
    // 21: sample_indexable(texture2d)(float,float,float,float) r3.w, r4.xyxx, t1.yzwx, s3
    r3.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 22: lt r3.xyzw, r2.zzzz, r3.xyzw
    r3.xyzw = (asfloat((uint4)((r2.zzzz)<(r3.xyzw)) * 0xffffffffu)).xyzw;
    // 23: and r4.xyzw, r3.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r4.xyzw = (asfloat(asuint(r3.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 24: mul r2.xy, r2.xyxx, cb0[9].xyxx
    r2.xy = ((r2.xyxx)*(source[9].xyxx)).xy;
    // 25: frc r2.xy, r2.xyxx
    r2.xy = (frac(r2.xyxx)).xy;
    // 26: movc r2.zw, r3.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r2.zw = ((asuint(r3.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 27: add r2.zw, r2.zzzw, r4.zzzw
    r2.zw = ((r2.zzzw)+(r4.zzzw)).zw;
    // 28: mad r2.xz, r2.xxxx, r2.zzwz, r4.xxyx
    r2.xz = ((r2.xxxx)*(r2.zzwz)+(r4.xxyx)).xz;
    // 29: add r0.w, -r2.x, r2.z
    r0.w = ((-(r2.xxxx))+(r2.zzzz)).w;
    // 30: mad r0.w, r2.y, r0.w, r2.x
    r0.w = ((r2.yyyy)*(r0.wwww)+(r2.xxxx)).w;
    // 31: mul r2.xyz, r0.wwww, cb0[10].xxxx
    r2.xyz = ((r0.wwww)*(source[10].xxxx)).xyz;
    // 32: else
    } else {
    // 33: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 34: endif
    }
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 36: dp3 r0.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 37: add r4.xyz, -r3.xyzx, r0.wwww
    r4.xyz = ((-(r3.xyzx))+(r0.wwww)).xyz;
    // 38: mad r3.xyz, cb0[2].wwww, r4.xyzx, r3.xyzx
    r3.xyz = ((source[2].wwww)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 39: mul r4.xyz, cb0[1].xyzx, cb0[3].xxxx
    r4.xyz = ((source[1].xyzx)*(source[3].xxxx)).xyz;
    // 40: mul r3.xyz, r3.xyzx, r4.xyzx
    r3.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 42: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 43: dp2 r0.w, r4.xyxx, r4.xyxx
    r0.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 44: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 46: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 47: add r5.z, r0.w, l(0.000010)
    r5.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: mul r4.xy, r4.xyxx, cb0[2].xxxx
    r4.xy = ((r4.xyxx)*(source[2].xxxx)).xy;
    // 49: mul r5.xy, r4.xyxx, v0.wwww
    r5.xy = ((r4.xyxx)*(v0.wwww)).xy;
    // 50: dp3 r0.w, r5.xyzx, r5.xyzx
    r0.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 51: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 52: div r4.xyz, r5.xyzx, r0.wwww
    r4.xyz = ((r5.xyzx)/(r0.wwww)).xyz;
    // 53: dp3 r0.w, r4.xyzx, r1.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 54: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 55: min r1.x, r0.w, l(1.000000)
    r1.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 56: mul r1.xyz, r3.xyzx, r1.xxxx
    r1.xyz = ((r3.xyzx)*(r1.xxxx)).xyz;
    // 57: mul r3.xyz, r0.wwww, cb2[3].xyzx
    r3.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 58: mad r1.xyz, r1.xyzx, cb2[3].wwww, r3.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r3.xyzx)).xyz;
    // 59: mul r1.xyz, r2.xyzx, r1.xyzx
    r1.xyz = ((r2.xyzx)*(r1.xyzx)).xyz;
    // 60: mul o0.xyz, r1.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r1.xyzx)*(source[4].xyzx)).xyz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t3.yzwx, s2, l(0.000000)
    r0.w = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 62: mul r0.w, r0.w, cb0[3].y
    r0.w = ((r0.wwww)*(source[3].yyyy)).w;
    // 63: add r1.x, -|r0.z|, l(1.000000)
    r1.x = ((-(abs(r0.zzzz)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 64: dp3 r0.x, r4.xyzx, r0.xyzx
    r0.x = (dot((r4.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 65: add r0.x, -|r0.x|, l(1.000000)
    r0.x = ((-(abs(r0.xxxx)))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 66: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 67: mad r0.x, r0.x, r0.w, -r0.w
    r0.x = ((r0.xxxx)*(r0.wwww)+(-(r0.wwww))).x;
    // 68: mad_sat r0.x, cb0[3].z, r0.x, r0.w
    r0.x = (saturate((source[3].zzzz)*(r0.xxxx)+(r0.wwww))).x;
    // 69: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 70: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 71: ret
    return output;
}

// source.character.static-map-native-225.v1 / source program 91f8da7d3428ca4daf760ce068b82b25
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight225(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[7]=float4(input.lightColor,1.0);
    source[13].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0;
    // 1: dp3 r0.x, v5.xyzx, v5.xyzx
    r0.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: dp3 r0.y, v3.xyzx, v3.xyzx
    r0.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 4: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 5: mul r0.yzw, r0.yyyy, v3.xxyz
    r0.yzw = ((r0.yyyy)*(v3.xxyz)).yzw;
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].y
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].yyyy)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: mul r1.xyzw, v6.yyyy, cb0[9].xyzw
    r1.xyzw = ((v6.yyyy)*(source[9].xyzw)).xyzw;
    // 9: mad r1.xyzw, cb0[8].xyzw, v6.xxxx, r1.xyzw
    r1.xyzw = ((source[8].xyzw)*(v6.xxxx)+(r1.xyzw)).xyzw;
    // 10: mad r1.xyzw, cb0[10].xyzw, v6.zzzz, r1.xyzw
    r1.xyzw = ((source[10].xyzw)*(v6.zzzz)+(r1.xyzw)).xyzw;
    // 11: mad r1.xyzw, cb0[11].xyzw, v6.wwww, r1.xyzw
    r1.xyzw = ((source[11].xyzw)*(v6.wwww)+(r1.xyzw)).xyzw;
    // 12: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 13: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s2
    r2.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 14: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 15: mov r3.yz, cb0[12].wwzw
    r3.yz = (source[12].wwzw).yz;
    // 16: add r3.xyzw, r1.xyxy, r3.xyzw
    r3.xyzw = ((r1.xyxy)+(r3.xyzw)).xyzw;
    // 17: sample_indexable(texture2d)(float,float,float,float) r2.y, r3.xyxx, t1.yxzw, s2
    r2.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 18: sample_indexable(texture2d)(float,float,float,float) r2.z, r3.zwzz, t1.yzxw, s2
    r2.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 19: add r3.xy, r1.xyxx, cb0[12].zwzz
    r3.xy = ((r1.xyxx)+(source[12].zwzz)).xy;
    // 20: sample_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t1.yzwx, s2
    r2.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 21: lt r2.xyzw, r1.zzzz, r2.xyzw
    r2.xyzw = (asfloat((uint4)((r1.zzzz)<(r2.xyzw)) * 0xffffffffu)).xyzw;
    // 22: and r3.xyzw, r2.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r3.xyzw = (asfloat(asuint(r2.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 23: mul r1.xy, r1.xyxx, cb0[12].xyxx
    r1.xy = ((r1.xyxx)*(source[12].xyxx)).xy;
    // 24: frc r1.xy, r1.xyxx
    r1.xy = (frac(r1.xyxx)).xy;
    // 25: movc r1.zw, r2.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r1.zw = ((asuint(r2.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 26: add r1.zw, r1.zzzw, r3.zzzw
    r1.zw = ((r1.zzzw)+(r3.zzzw)).zw;
    // 27: mad r1.xz, r1.xxxx, r1.zzwz, r3.xxyx
    r1.xz = ((r1.xxxx)*(r1.zzwz)+(r3.xxyx)).xz;
    // 28: add r1.z, -r1.x, r1.z
    r1.z = ((-(r1.xxxx))+(r1.zzzz)).z;
    // 29: mad r1.x, r1.y, r1.z, r1.x
    r1.x = ((r1.yyyy)*(r1.zzzz)+(r1.xxxx)).x;
    // 30: mul r1.xyz, r1.xxxx, cb0[13].xxxx
    r1.xyz = ((r1.xxxx)*(source[13].xxxx)).xyz;
    // 31: else
    } else {
    // 32: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 33: endif
    }
    // 34: mul r2.xy, v2.xyxx, cb0[1].xyxx
    r2.xy = ((v2.xyxx)*(source[1].xyxx)).xy;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xyzw, r2.xyxx, t2.xyzw, s1, l(0.000000)
    r3.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 36: mul r4.xyz, cb0[2].xyzx, cb0[5].yyyy
    r4.xyz = ((source[2].xyzx)*(source[5].yyyy)).xyz;
    // 37: mul r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 39: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 40: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 41: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 44: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 45: mul r2.xy, r2.xyxx, cb0[4].wwww
    r2.xy = ((r2.xyxx)*(source[4].wwww)).xy;
    // 46: mul r5.xy, r2.xyxx, v0.wwww
    r5.xy = ((r2.xyxx)*(v0.wwww)).xy;
    // 47: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 48: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 49: div r2.xyz, r5.xyzx, r1.wwww
    r2.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 50: dp3 r1.w, r2.xyzx, r0.yzwy
    r1.w = (dot((r2.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 51: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 52: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 53: mul r5.xyz, cb0[3].xyzx, cb0[5].zzzz
    r5.xyz = ((source[3].xyzx)*(source[5].zzzz)).xyz;
    // 54: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 55: mad r0.xyz, v5.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v5.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 56: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 57: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 58: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 59: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 60: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 61: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 62: mul r0.x, r0.x, cb0[5].w
    r0.x = ((r0.xxxx)*(source[5].wwww)).x;
    // 63: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 64: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 65: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 66: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 67: mad r0.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 68: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 69: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 70: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 71: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 72: mul_sat r0.x, r3.w, cb0[6].x
    r0.x = (saturate((r3.wwww)*(source[6].xxxx))).x;
    // 73: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 74: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 75: ret
    return output;
}

// source.character.static-map-native-226.v1 / source program 147f1f40de82ce4c89f879d980756738
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight226(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[3]=float4(input.lightColor,1.0);
    source[9].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0;
    // 1: dp3 r0.x, v3.xyzx, v3.xyzx
    r0.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v3.xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)).xyz;
    // 4: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].y
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].yyyy)) * 0xffffffffu)).w;
    // 5: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 6: mul r1.xyzw, v6.yyyy, cb0[5].xyzw
    r1.xyzw = ((v6.yyyy)*(source[5].xyzw)).xyzw;
    // 7: mad r1.xyzw, cb0[4].xyzw, v6.xxxx, r1.xyzw
    r1.xyzw = ((source[4].xyzw)*(v6.xxxx)+(r1.xyzw)).xyzw;
    // 8: mad r1.xyzw, cb0[6].xyzw, v6.zzzz, r1.xyzw
    r1.xyzw = ((source[6].xyzw)*(v6.zzzz)+(r1.xyzw)).xyzw;
    // 9: mad r1.xyzw, cb0[7].xyzw, v6.wwww, r1.xyzw
    r1.xyzw = ((source[7].xyzw)*(v6.wwww)+(r1.xyzw)).xyzw;
    // 10: div r1.xy, r1.xyxx, r1.wwww
    r1.xy = ((r1.xyxx)/(r1.wwww)).xy;
    // 11: sample_indexable(texture2d)(float,float,float,float) r2.x, r1.xyxx, t1.xyzw, s2
    r2.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 12: mov r3.xw, l(0,0,0,0)
    r3.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 13: mov r3.yz, cb0[8].wwzw
    r3.yz = (source[8].wwzw).yz;
    // 14: add r3.xyzw, r1.xyxy, r3.xyzw
    r3.xyzw = ((r1.xyxy)+(r3.xyzw)).xyzw;
    // 15: sample_indexable(texture2d)(float,float,float,float) r2.y, r3.xyxx, t1.yxzw, s2
    r2.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 16: sample_indexable(texture2d)(float,float,float,float) r2.z, r3.zwzz, t1.yzxw, s2
    r2.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 17: add r3.xy, r1.xyxx, cb0[8].zwzz
    r3.xy = ((r1.xyxx)+(source[8].zwzz)).xy;
    // 18: sample_indexable(texture2d)(float,float,float,float) r2.w, r3.xyxx, t1.yzwx, s2
    r2.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 19: lt r2.xyzw, r1.zzzz, r2.xyzw
    r2.xyzw = (asfloat((uint4)((r1.zzzz)<(r2.xyzw)) * 0xffffffffu)).xyzw;
    // 20: and r3.xyzw, r2.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r3.xyzw = (asfloat(asuint(r2.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 21: mul r1.xy, r1.xyxx, cb0[8].xyxx
    r1.xy = ((r1.xyxx)*(source[8].xyxx)).xy;
    // 22: frc r1.xy, r1.xyxx
    r1.xy = (frac(r1.xyxx)).xy;
    // 23: movc r1.zw, r2.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r1.zw = ((asuint(r2.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 24: add r1.zw, r1.zzzw, r3.zzzw
    r1.zw = ((r1.zzzw)+(r3.zzzw)).zw;
    // 25: mad r1.xz, r1.xxxx, r1.zzwz, r3.xxyx
    r1.xz = ((r1.xxxx)*(r1.zzwz)+(r3.xxyx)).xz;
    // 26: add r0.w, -r1.x, r1.z
    r0.w = ((-(r1.xxxx))+(r1.zzzz)).w;
    // 27: mad r0.w, r1.y, r0.w, r1.x
    r0.w = ((r1.yyyy)*(r0.wwww)+(r1.xxxx)).w;
    // 28: mul r1.xyz, r0.wwww, cb0[9].xxxx
    r1.xyz = ((r0.wwww)*(source[9].xxxx)).xyz;
    // 29: else
    } else {
    // 30: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 31: endif
    }
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: mul r3.xyz, cb0[1].xyzx, cb0[2].zzzz
    r3.xyz = ((source[1].xyzx)*(source[2].zzzz)).xyz;
    // 34: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: dp2 r0.w, r3.xyxx, r3.xyxx
    r0.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 38: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 41: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r3.xy, r3.xyxx, cb0[2].xxxx
    r3.xy = ((r3.xyxx)*(source[2].xxxx)).xy;
    // 43: mul r4.xy, r3.xyxx, v0.wwww
    r4.xy = ((r3.xyxx)*(v0.wwww)).xy;
    // 44: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 45: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 46: div r3.xyz, r4.xyzx, r0.wwww
    r3.xyz = ((r4.xyzx)/(r0.wwww)).xyz;
    // 47: dp3 r0.x, r3.xyzx, r0.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 48: max r0.x, r0.x, l(0.000000)
    r0.x = (max(r0.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 49: min r0.y, r0.x, l(1.000000)
    r0.y = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: mul r0.yzw, r2.xxyz, r0.yyyy
    r0.yzw = ((r2.xxyz)*(r0.yyyy)).yzw;
    // 51: mul r2.xyz, r0.xxxx, cb2[3].xyzx
    r2.xyz = ((r0.xxxx)*(passValues[3].xyzx)).xyz;
    // 52: mad r0.xyz, r0.yzwy, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.yzwy)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 53: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 54: mul o0.xyz, r0.xyzx, cb0[3].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[3].xyzx)).xyz;
    // 55: mul_sat r0.x, r2.w, cb0[2].w
    r0.x = (saturate((r2.wwww)*(source[2].wwww))).x;
    // 56: mul o0.w, r0.x, cb0[0].x
    output.targets[0].w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 57: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 58: ret
    return output;
}

// source.character.static-map-native-227.v1 / source program 02b2a85f642a1440920ad6ab057d94d0
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight227(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[13]=float4(input.lightColor,1.0);
    source[14].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: mul r5.xy, v4.xyxx, cb0[1].xyxx
    r5.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r5.xyxx, t0.xywz, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 18: mad r5.zw, r6.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r6.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r6.xy, r5.xyxx, cb0[8].xxxx
    r6.xy = ((r5.xyxx)*(source[8].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r6.xy, r6.xyxx, cb0[8].yyyy
    r6.xy = ((r6.xyxx)*(source[8].yyyy)).xy;
    // 28: mad r5.zw, cb0[7].wwww, r5.zzzw, r6.xxxy
    r5.zw = ((source[7].wwww)*(r5.zzzw)+(r6.xxxy)).zw;
    // 29: mul r7.xy, r5.zwzz, v2.wwww
    r7.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r6.xyw, r7.xyxz, r1.wwww
    r6.xyw = ((r7.xyxz)/(r1.wwww)).xyw;
    // 33: max r1.w, cb0[8].z, l(0.000000)
    r1.w = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 35: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 37: dp3 r7.x, r1.xyzx, r6.xywx
    r7.x = (dot((r1.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 38: dp3 r7.y, r2.xyzx, r6.xywx
    r7.y = (dot((r2.xyzx).xyz,(r6.xywx).xyz).xxxx).y;
    // 39: dp3 r7.z, r0.xyzx, r6.xywx
    r7.z = (dot((r0.xyzx).xyz,(r6.xywx).xyz).xxxx).z;
    // 40: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 41: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 43: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 44: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r0.y, r6.w, r6.w
    r0.y = ((r6.wwww)*(r6.wwww)).y;
    // 47: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 48: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: mul r5.zw, v4.xxxy, cb0[9].zzzz
    r5.zw = ((v4.xxxy)*(source[9].zzzz)).zw;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r5.zwzz, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r0.z, r8.w, r8.w
    r0.z = ((r8.wwww)*(r8.wwww)).z;
    // 52: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 53: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 54: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 55: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 56: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 57: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 58: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 59: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 60: add r9.xyz, -r6.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r9.xyz, r0.yyyy, r9.xyzx, r6.xywx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)+(r6.xywx)).xyz;
    // 62: dp3 r0.y, r9.xyzx, r9.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 63: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 64: mul r10.xyz, r0.yyyy, r9.xyzx
    r10.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 65: dp3 r0.y, r10.xyzx, r3.xyzx
    r0.y = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 66: mul r10.xyz, r0.yyyy, r10.xyzx
    r10.xyz = ((r0.yyyy)*(r10.xyzx)).xyz;
    // 67: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 69: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 70: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 71: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 72: sample_indexable(texture2d)(float,float,float,float) r11.xyz, r0.yzyy, t6.xyzw, s0
    r11.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 73: mul r11.xyz, r11.xyzx, r11.xyzx
    r11.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 74: else
    } else {
    // 75: mov r11.xyz, l(1.000000,1.000000,1.000000,0)
    r11.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 76: endif
    }
    // 77: dp3 r1.x, r1.xyzx, r10.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 78: dp3 r1.y, r2.xyzx, r10.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 79: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 80: mad r0.yz, cb0[9].wwww, r1.xxyx, r0.yyzy
    r0.yz = ((source[9].wwww)*(r1.xxyx)+(r0.yyzy)).yz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 82: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 83: mad r1.xyz, cb0[10].xxxx, r1.xyzx, r1.xyzx
    r1.xyz = ((source[10].xxxx)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 84: add r1.xyz, r1.xyzx, -cb0[10].xxxx
    r1.xyz = ((r1.xyzx)+(-(source[10].xxxx))).xyz;
    // 85: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 86: mul r0.y, r6.z, cb0[10].y
    r0.y = ((r6.zzzz)*(source[10].yyyy)).y;
    // 87: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 88: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 89: mad r7.xyz, cb0[10].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[10].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 90: mul r10.xyz, cb0[4].xyzx, cb0[11].xxxx
    r10.xyz = ((source[4].xyzx)*(source[11].xxxx)).xyz;
    // 91: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 92: mad r2.xyz, r0.yyyy, r2.xyzx, r7.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 93: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 94: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 95: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 96: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 97: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 98: mul r2.xyz, cb0[5].xyzx, cb0[11].yyyy
    r2.xyz = ((source[5].xyzx)*(source[11].yyyy)).xyz;
    // 99: mul r7.xyz, r8.xyzx, r2.xyzx
    r7.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 100: dp3 r0.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 101: mad r2.xyz, -r2.xyzx, r8.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r8.xyzx)+(r0.yyyy)).xyz;
    // 102: mad r2.xyz, cb0[11].wwww, r2.xyzx, r7.xyzx
    r2.xyz = ((source[11].wwww)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 103: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 104: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 105: dp3 r0.y, r9.xyzx, r4.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 106: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 107: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r5.xyxx, t4.xyzw, s6, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 108: mul r4.xyz, cb0[6].xyzx, cb0[12].xxxx
    r4.xyz = ((source[6].xyzx)*(source[12].xxxx)).xyz;
    // 109: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 110: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 111: add r4.xyz, -r8.xyzx, r1.wwww
    r4.xyz = ((-(r8.xyzx))+(r1.wwww)).xyz;
    // 112: mad r4.xyz, cb0[11].wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 113: mad r4.xyz, cb0[12].yyyy, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[12].yyyy)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 114: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 115: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 116: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 117: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 118: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 119: dp3 r0.x, r3.xyzx, r6.xywx
    r0.x = (dot((r3.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 120: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 121: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 122: mul r0.x, r0.x, cb0[12].z
    r0.x = ((r0.xxxx)*(source[12].zzzz)).x;
    // 123: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 124: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 125: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 126: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 127: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 128: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 129: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 130: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 131: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 132: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 133: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 134: ret
    return output;
}

// source.character.static-map-native-228.v1 / source program 7ca7631fa5ab724f8153f6b1afc6ecbd
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight228(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=float4(input.lightColor,1.0);
    source[12].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r7.xyzw, v4.xyxy, cb0[6].yyww
    r7.xyzw = ((v4.xyxy)*(source[6].yyww)).xyzw;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r7.xy, r7.xyxx, cb0[6].zzzz
    r7.xy = ((r7.xyxx)*(source[6].zzzz)).xy;
    // 27: mad r5.xy, cb0[6].xxxx, r5.xyxx, r7.xyxx
    r5.xy = ((source[6].xxxx)*(r5.xyxx)+(r7.xyxx)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r7.zwzz, t2.xyzw, s3, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 33: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 34: dp2 r1.w, r6.xyxx, r6.xyxx
    r1.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 35: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 37: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 38: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 39: mul r8.xy, r6.xyxx, cb0[7].xxxx
    r8.xy = ((r6.xyxx)*(source[7].xxxx)).xy;
    // 40: max r1.w, cb0[7].y, l(0.000000)
    r1.w = (max(source[7].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 41: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 42: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 43: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 44: dp3 r6.x, r1.xyzx, r5.xywx
    r6.x = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 45: dp3 r6.y, r2.xyzx, r5.xywx
    r6.y = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 46: dp3 r6.z, r0.xyzx, r5.xywx
    r6.z = (dot((r0.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // 47: max r0.xyz, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[1].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 48: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 50: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, r0.x, l(0.500000), cb0[8].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].xxxx)).x;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 53: mul r0.y, r5.w, r5.w
    r0.y = ((r5.wwww)*(r5.wwww)).y;
    // 54: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 55: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 56: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.zwzz, t4.xyzw, s5, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r7.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 57: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 58: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 59: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 60: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 61: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 62: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 63: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 64: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 65: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 66: add r8.xyz, -r5.xywx, r8.xyzx
    r8.xyz = ((-(r5.xywx))+(r8.xyzx)).xyz;
    // 67: mad r8.xyz, r0.yyyy, r8.xyzx, r5.xywx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r5.xywx)).xyz;
    // 68: dp3 r0.y, r8.xyzx, r8.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 69: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 70: mul r9.xyz, r0.yyyy, r8.xyzx
    r9.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 71: dp3 r0.y, r9.xyzx, r3.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 72: mul r9.xyz, r0.yyyy, r9.xyzx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 73: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 74: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 75: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 76: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 77: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 78: sample_indexable(texture2d)(float,float,float,float) r10.xyz, r0.yzyy, t7.xyzw, s0
    r10.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 79: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 80: else
    } else {
    // 81: mov r10.xyz, l(1.000000,1.000000,1.000000,0)
    r10.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 82: endif
    }
    // 83: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 84: dp3 r1.y, r2.xyzx, r9.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 85: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 86: mad r0.yz, cb0[8].yyyy, r1.xxyx, r0.yyzy
    r0.yz = ((source[8].yyyy)*(r1.xxyx)+(r0.yyzy)).yz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t6.xyzw, s6, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 88: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 89: mad r1.xyz, cb0[8].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[8].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 90: add r1.xyz, r1.xyzx, -cb0[8].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[8].zzzz))).xyz;
    // 91: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 92: mul r0.y, r5.z, cb0[8].w
    r0.y = ((r5.zzzz)*(source[8].wwww)).y;
    // 93: mul r9.xyz, cb0[3].xyzx, cb0[9].xxxx
    r9.xyz = ((source[3].xyzx)*(source[9].xxxx)).xyz;
    // 94: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 95: mad r2.xyz, r0.yyyy, r2.xyzx, r6.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 96: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 97: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 98: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 99: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 100: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 101: mul r2.xyz, cb0[4].xyzx, cb0[9].yyyy
    r2.xyz = ((source[4].xyzx)*(source[9].yyyy)).xyz;
    // 102: mul r6.xyz, r7.xyzx, r2.xyzx
    r6.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 103: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 104: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 105: mad r2.xyz, cb0[9].wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((source[9].wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 106: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 107: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 108: dp3 r0.y, r8.xyzx, r4.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 109: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t5.xyzw, s7, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 111: mul r4.xyz, cb0[5].xyzx, cb0[10].xxxx
    r4.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 112: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 113: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 114: add r4.xyz, -r7.xyzx, r1.wwww
    r4.xyz = ((-(r7.xyzx))+(r1.wwww)).xyz;
    // 115: mad r4.xyz, cb0[9].wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((source[9].wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 116: mad r4.xyz, cb0[10].yyyy, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[10].yyyy)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 117: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 118: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 119: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 120: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 121: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 122: dp3 r0.x, r3.xyzx, r5.xywx
    r0.x = (dot((r3.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 123: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 124: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 125: mul r0.x, r0.x, cb0[10].z
    r0.x = ((r0.xxxx)*(source[10].zzzz)).x;
    // 126: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 127: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 128: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 129: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 130: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 131: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 132: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 133: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 134: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 135: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 136: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 137: ret
    return output;
}

// source.character.static-map-native-229.v1 / source program 29ab1bfb6c6b93479e45a82994735cac
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight229(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=float4(input.lightColor,1.0);
    source[12].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r7.xy, v4.xyxx, cb0[6].yyyy
    r7.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r7.xy, r7.xyxx, cb0[6].zzzz
    r7.xy = ((r7.xyxx)*(source[6].zzzz)).xy;
    // 27: mad r5.xy, cb0[6].xxxx, r5.xyxx, r7.xyxx
    r5.xy = ((source[6].xxxx)*(r5.xyxx)+(r7.xyxx)).xy;
    // 28: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 32: max r1.w, cb0[6].w, l(0.000000)
    r1.w = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 36: dp3 r6.x, r1.xyzx, r5.xywx
    r6.x = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 37: dp3 r6.y, r2.xyzx, r5.xywx
    r6.y = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 38: dp3 r6.z, r0.xyzx, r5.xywx
    r6.z = (dot((r0.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // 39: mul r0.xy, cb0[1].xyxx, cb0[7].xxxx
    r0.xy = ((source[1].xyxx)*(source[7].xxxx)).xy;
    // 40: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 41: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 42: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 43: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 44: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 45: mad r0.x, r0.x, l(0.500000), cb0[7].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].wwww)).x;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 47: mul r0.y, r5.w, r5.w
    r0.y = ((r5.wwww)*(r5.wwww)).y;
    // 48: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 49: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 50: mul r7.xy, v4.xyxx, cb0[8].xxxx
    r7.xy = ((v4.xyxx)*(source[8].xxxx)).xy;
    // 51: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 52: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 53: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 54: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 55: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 56: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 57: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 58: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 59: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 60: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 61: add r8.xyz, -r5.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r5.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 62: mad r8.xyz, r0.yyyy, r8.xyzx, r5.xywx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r5.xywx)).xyz;
    // 63: dp3 r0.y, r8.xyzx, r8.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 64: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 65: mul r9.xyz, r0.yyyy, r8.xyzx
    r9.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 66: dp3 r0.y, r9.xyzx, r3.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 67: mul r9.xyz, r0.yyyy, r9.xyzx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 68: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 69: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).y;
    // 70: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 71: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 72: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 73: sample_indexable(texture2d)(float,float,float,float) r10.xyz, r0.yzyy, t6.xyzw, s0
    r10.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 74: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 75: else
    } else {
    // 76: mov r10.xyz, l(1.000000,1.000000,1.000000,0)
    r10.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 77: endif
    }
    // 78: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 79: dp3 r1.y, r2.xyzx, r9.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 80: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 81: mad r0.yz, cb0[8].yyyy, r1.xxyx, r0.yyzy
    r0.yz = ((source[8].yyyy)*(r1.xxyx)+(r0.yyzy)).yz;
    // 82: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 83: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 84: mad r1.xyz, cb0[8].zzzz, r1.xyzx, r1.xyzx
    r1.xyz = ((source[8].zzzz)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 85: add r1.xyz, r1.xyzx, -cb0[8].zzzz
    r1.xyz = ((r1.xyzx)+(-(source[8].zzzz))).xyz;
    // 86: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 87: mul r0.y, r5.z, cb0[8].w
    r0.y = ((r5.zzzz)*(source[8].wwww)).y;
    // 88: mul r9.xyz, cb0[3].xyzx, cb0[9].xxxx
    r9.xyz = ((source[3].xyzx)*(source[9].xxxx)).xyz;
    // 89: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 90: mad r2.xyz, r0.yyyy, r2.xyzx, r6.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 91: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 92: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 93: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 94: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 95: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 96: mul r2.xyz, cb0[4].xyzx, cb0[9].yyyy
    r2.xyz = ((source[4].xyzx)*(source[9].yyyy)).xyz;
    // 97: mul r6.xyz, r7.xyzx, r2.xyzx
    r6.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 98: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 99: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 100: mad r2.xyz, cb0[9].wwww, r2.xyzx, r6.xyzx
    r2.xyz = ((source[9].wwww)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 101: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 102: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 103: dp3 r0.y, r8.xyzx, r4.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 104: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 105: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t4.xyzw, s6, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 106: mul r4.xyz, cb0[5].xyzx, cb0[10].xxxx
    r4.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 107: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 108: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 109: add r4.xyz, -r7.xyzx, r1.wwww
    r4.xyz = ((-(r7.xyzx))+(r1.wwww)).xyz;
    // 110: mad r4.xyz, cb0[9].wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((source[9].wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 111: mad r4.xyz, cb0[10].yyyy, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[10].yyyy)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 112: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 113: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 114: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 115: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 116: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 117: dp3 r0.x, r3.xyzx, r5.xywx
    r0.x = (dot((r3.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 118: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 119: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 120: mul r0.x, r0.x, cb0[10].z
    r0.x = ((r0.xxxx)*(source[10].zzzz)).x;
    // 121: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 122: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 123: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 124: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 125: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 126: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 127: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 128: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 129: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 130: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 131: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 132: ret
    return output;
}

// source.character.static-map-native-230.v1 / source program c474e9f748cd6444b38f913598d81a87
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight230(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[13]=float4(input.lightColor,1.0);
    source[14].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: mul r5.xy, v4.xyxx, cb0[1].xyxx
    r5.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r5.xyxx, t0.xywz, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 18: mad r5.zw, r6.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r6.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r6.xy, r5.xyxx, cb0[8].xxxx
    r6.xy = ((r5.xyxx)*(source[8].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r6.xy, r6.xyxx, cb0[8].yyyy
    r6.xy = ((r6.xyxx)*(source[8].yyyy)).xy;
    // 28: mad r5.zw, cb0[7].wwww, r5.zzzw, r6.xxxy
    r5.zw = ((source[7].wwww)*(r5.zzzw)+(r6.xxxy)).zw;
    // 29: mul r7.xy, r5.zwzz, v2.wwww
    r7.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r6.xyw, r7.xyxz, r1.wwww
    r6.xyw = ((r7.xyxz)/(r1.wwww)).xyw;
    // 33: max r1.w, cb0[8].z, l(0.000000)
    r1.w = (max(source[8].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 35: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 37: dp3 r7.x, r1.xyzx, r6.xywx
    r7.x = (dot((r1.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 38: dp3 r7.y, r2.xyzx, r6.xywx
    r7.y = (dot((r2.xyzx).xyz,(r6.xywx).xyz).xxxx).y;
    // 39: dp3 r7.z, r0.xyzx, r6.xywx
    r7.z = (dot((r0.xyzx).xyz,(r6.xywx).xyz).xxxx).z;
    // 40: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 41: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 43: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 44: mad r0.x, r0.x, l(0.500000), cb0[9].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[9].yyyy)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r0.y, r6.w, r6.w
    r0.y = ((r6.wwww)*(r6.wwww)).y;
    // 47: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 48: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: mul r7.xy, v4.xyxx, cb0[9].zzzz
    r7.xy = ((v4.xyxx)*(source[9].zzzz)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 52: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 53: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 54: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 55: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 56: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 57: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 58: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 59: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 60: add r8.xyz, -r6.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r6.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r8.xyz, r0.yyyy, r8.xyzx, r6.xywx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r6.xywx)).xyz;
    // 62: dp3 r0.y, r8.xyzx, r8.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 63: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 64: mul r9.xyz, r0.yyyy, r8.xyzx
    r9.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 65: dp3 r0.y, r9.xyzx, r3.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 66: mul r9.xyz, r0.yyyy, r9.xyzx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 67: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 69: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 70: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 71: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 72: sample_indexable(texture2d)(float,float,float,float) r10.xyz, r0.yzyy, t5.xyzw, s0
    r10.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 73: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 74: else
    } else {
    // 75: mov r10.xyz, l(1.000000,1.000000,1.000000,0)
    r10.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 76: endif
    }
    // 77: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 78: dp3 r1.y, r2.xyzx, r9.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 79: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 80: mad r0.yz, cb0[9].wwww, r1.xxyx, r0.yyzy
    r0.yz = ((source[9].wwww)*(r1.xxyx)+(r0.yyzy)).yz;
    // 81: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t4.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 82: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 83: mad r1.xyz, cb0[10].xxxx, r1.xyzx, r1.xyzx
    r1.xyz = ((source[10].xxxx)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 84: add r1.xyz, r1.xyzx, -cb0[10].xxxx
    r1.xyz = ((r1.xyzx)+(-(source[10].xxxx))).xyz;
    // 85: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 86: mul r0.y, r6.z, cb0[10].y
    r0.y = ((r6.zzzz)*(source[10].yyyy)).y;
    // 87: dp3 r0.z, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 88: add r9.xyz, -r5.xyzx, r0.zzzz
    r9.xyz = ((-(r5.xyzx))+(r0.zzzz)).xyz;
    // 89: mad r9.xyz, cb0[10].wwww, r9.xyzx, r5.xyzx
    r9.xyz = ((source[10].wwww)*(r9.xyzx)+(r5.xyzx)).xyz;
    // 90: mul r11.xyz, cb0[4].xyzx, cb0[11].xxxx
    r11.xyz = ((source[4].xyzx)*(source[11].xxxx)).xyz;
    // 91: mul r9.xyz, r9.xyzx, r11.xyzx
    r9.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 92: mad r2.xyz, r0.yyyy, r2.xyzx, r9.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 93: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 94: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 95: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 96: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 97: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 98: mul r2.xyz, cb0[5].xyzx, cb0[11].yyyy
    r2.xyz = ((source[5].xyzx)*(source[11].yyyy)).xyz;
    // 99: mul r9.xyz, r7.xyzx, r2.xyzx
    r9.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 100: dp3 r0.y, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 101: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 102: mad r2.xyz, cb0[11].wwww, r2.xyzx, r9.xyzx
    r2.xyz = ((source[11].wwww)*(r2.xyzx)+(r9.xyzx)).xyz;
    // 103: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 104: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 105: dp3 r0.y, r8.xyzx, r4.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 106: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 107: mul r2.xyz, cb0[6].xyzx, cb0[12].xxxx
    r2.xyz = ((source[6].xyzx)*(source[12].xxxx)).xyz;
    // 108: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 109: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 110: add r4.xyz, -r7.xyzx, r1.wwww
    r4.xyz = ((-(r7.xyzx))+(r1.wwww)).xyz;
    // 111: mad r4.xyz, cb0[11].wwww, r4.xyzx, r7.xyzx
    r4.xyz = ((source[11].wwww)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 112: mad r4.xyz, cb0[12].yyyy, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[12].yyyy)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 113: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 114: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 115: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 116: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 117: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 118: dp3 r0.x, r3.xyzx, r6.xywx
    r0.x = (dot((r3.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 119: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 120: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 121: mul r0.x, r0.x, cb0[12].z
    r0.x = ((r0.xxxx)*(source[12].zzzz)).x;
    // 122: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 123: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 124: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 125: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 126: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 127: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 128: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 129: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 130: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 131: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 132: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 133: ret
    return output;
}

// source.character.static-map-native-231.v1 / source program 648145dbb8df3947b6b16792e23b9f4e
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight231(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[14]=float4(input.lightColor,1.0);
    source[15].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0, r11=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: add r5.xy, v4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r5.xy = ((v4.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 17: mad r5.zw, v4.yyyx, l(0.000000, 0.000000, -1.000000, 1.000000), l(0.000000, 0.000000, 0.500000, -0.500000)
    r5.zw = ((v4.yyyx)*(float4(0.000000,0.000000,-1.000000,1.000000))+(float4(0.000000,0.000000,0.500000,-0.500000))).zw;
    // 18: mul r5.xy, r5.xyxx, cb0[7].wwww
    r5.xy = ((r5.xyxx)*(source[7].wwww)).xy;
    // 19: mad r5.xy, cb0[7].zzzz, r5.zwzz, r5.xyxx
    r5.xy = ((source[7].zzzz)*(r5.zwzz)+(r5.xyxx)).xy;
    // 20: add r5.xy, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 21: mul r5.xy, r5.xyxx, cb0[1].xyxx
    r5.xy = ((r5.xyxx)*(source[1].xyxx)).xy;
    // 22: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r5.xyxx, t0.xywz, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 23: mad r5.zw, r6.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r6.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 24: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 25: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 26: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 27: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 28: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 29: mul r6.xy, r5.xyxx, cb0[9].xxxx
    r6.xy = ((r5.xyxx)*(source[9].xxxx)).xy;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: mul r6.xy, r6.xyxx, cb0[9].yyyy
    r6.xy = ((r6.xyxx)*(source[9].yyyy)).xy;
    // 33: mad r5.zw, cb0[8].wwww, r5.zzzw, r6.xxxy
    r5.zw = ((source[8].wwww)*(r5.zzzw)+(r6.xxxy)).zw;
    // 34: mul r7.xy, r5.zwzz, v2.wwww
    r7.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 35: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: div r6.xyw, r7.xyxz, r1.wwww
    r6.xyw = ((r7.xyxz)/(r1.wwww)).xyw;
    // 38: max r1.w, cb0[9].z, l(0.000000)
    r1.w = (max(source[9].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 39: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 40: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 42: dp3 r7.x, r1.xyzx, r6.xywx
    r7.x = (dot((r1.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 43: dp3 r7.y, r2.xyzx, r6.xywx
    r7.y = (dot((r2.xyzx).xyz,(r6.xywx).xyz).xxxx).y;
    // 44: dp3 r7.z, r0.xyzx, r6.xywx
    r7.z = (dot((r0.xyzx).xyz,(r6.xywx).xyz).xxxx).z;
    // 45: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 46: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 47: dp3 r0.x, r7.xyzx, r0.xyzx
    r0.x = (dot((r7.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 48: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mad r0.x, r0.x, l(0.500000), cb0[10].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[10].yyyy)).x;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r0.y, r6.w, r6.w
    r0.y = ((r6.wwww)*(r6.wwww)).y;
    // 52: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 53: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: mul r5.zw, v4.xxxy, cb0[10].zzzz
    r5.zw = ((v4.xxxy)*(source[10].zzzz)).zw;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r5.zwzz, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 56: mul r0.z, r8.w, r8.w
    r0.z = ((r8.wwww)*(r8.wwww)).z;
    // 57: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 58: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 59: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 60: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 61: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 62: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 63: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 64: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 65: add r9.xyz, -r6.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 66: mad r9.xyz, r0.yyyy, r9.xyzx, r6.xywx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)+(r6.xywx)).xyz;
    // 67: dp3 r0.y, r9.xyzx, r9.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 68: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 69: mul r10.xyz, r0.yyyy, r9.xyzx
    r10.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 70: dp3 r0.y, r10.xyzx, r3.xyzx
    r0.y = (dot((r10.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 71: mul r10.xyz, r0.yyyy, r10.xyzx
    r10.xyz = ((r0.yyyy)*(r10.xyzx)).xyz;
    // 72: mad r10.xyz, r10.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r10.xyz = ((r10.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 73: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[15].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[15].xxxx)) * 0xffffffffu)).y;
    // 74: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 75: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 76: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 77: sample_indexable(texture2d)(float,float,float,float) r11.xyz, r0.yzyy, t6.xyzw, s0
    r11.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 78: mul r11.xyz, r11.xyzx, r11.xyzx
    r11.xyz = ((r11.xyzx)*(r11.xyzx)).xyz;
    // 79: else
    } else {
    // 80: mov r11.xyz, l(1.000000,1.000000,1.000000,0)
    r11.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 81: endif
    }
    // 82: dp3 r1.x, r1.xyzx, r10.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r10.xyzx).xyz).xxxx).x;
    // 83: dp3 r1.y, r2.xyzx, r10.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r10.xyzx).xyz).xxxx).y;
    // 84: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 85: mad r0.yz, cb0[10].wwww, r1.xxyx, r0.yyzy
    r0.yz = ((source[10].wwww)*(r1.xxyx)+(r0.yyzy)).yz;
    // 86: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t5.xyzw, s5, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 87: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 88: mad r1.xyz, cb0[11].xxxx, r1.xyzx, r1.xyzx
    r1.xyz = ((source[11].xxxx)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 89: add r1.xyz, r1.xyzx, -cb0[11].xxxx
    r1.xyz = ((r1.xyzx)+(-(source[11].xxxx))).xyz;
    // 90: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 91: mul r0.y, r6.z, cb0[11].y
    r0.y = ((r6.zzzz)*(source[11].yyyy)).y;
    // 92: dp3 r0.z, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 93: add r10.xyz, -r7.xyzx, r0.zzzz
    r10.xyz = ((-(r7.xyzx))+(r0.zzzz)).xyz;
    // 94: mad r7.xyz, cb0[11].wwww, r10.xyzx, r7.xyzx
    r7.xyz = ((source[11].wwww)*(r10.xyzx)+(r7.xyzx)).xyz;
    // 95: mul r10.xyz, cb0[4].xyzx, cb0[12].xxxx
    r10.xyz = ((source[4].xyzx)*(source[12].xxxx)).xyz;
    // 96: mul r7.xyz, r7.xyzx, r10.xyzx
    r7.xyz = ((r7.xyzx)*(r10.xyzx)).xyz;
    // 97: mad r2.xyz, r0.yyyy, r2.xyzx, r7.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 98: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 99: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 100: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 101: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 102: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 103: mul r2.xyz, cb0[5].xyzx, cb0[12].yyyy
    r2.xyz = ((source[5].xyzx)*(source[12].yyyy)).xyz;
    // 104: mul r7.xyz, r8.xyzx, r2.xyzx
    r7.xyz = ((r8.xyzx)*(r2.xyzx)).xyz;
    // 105: dp3 r0.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 106: mad r2.xyz, -r2.xyzx, r8.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r8.xyzx)+(r0.yyyy)).xyz;
    // 107: mad r2.xyz, cb0[12].wwww, r2.xyzx, r7.xyzx
    r2.xyz = ((source[12].wwww)*(r2.xyzx)+(r7.xyzx)).xyz;
    // 108: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 109: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 110: dp3 r0.y, r9.xyzx, r4.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 111: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 112: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, r5.xyxx, t4.xyzw, s6, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 113: mul r4.xyz, cb0[6].xyzx, cb0[13].xxxx
    r4.xyz = ((source[6].xyzx)*(source[13].xxxx)).xyz;
    // 114: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 115: dp3 r1.w, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 116: add r4.xyz, -r8.xyzx, r1.wwww
    r4.xyz = ((-(r8.xyzx))+(r1.wwww)).xyz;
    // 117: mad r4.xyz, cb0[12].wwww, r4.xyzx, r8.xyzx
    r4.xyz = ((source[12].wwww)*(r4.xyzx)+(r8.xyzx)).xyz;
    // 118: mad r4.xyz, cb0[13].yyyy, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[13].yyyy)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 119: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 120: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 121: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 122: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 123: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 124: dp3 r0.x, r3.xyzx, r6.xywx
    r0.x = (dot((r3.xyzx).xyz,(r6.xywx).xyz).xxxx).x;
    // 125: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 126: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 127: mul r0.x, r0.x, cb0[13].z
    r0.x = ((r0.xxxx)*(source[13].zzzz)).x;
    // 128: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 129: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 130: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 131: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 132: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 133: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 134: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 135: mul r0.xyz, r11.xyzx, r0.xyzx
    r0.xyz = ((r11.xyzx)*(r0.xyzx)).xyz;
    // 136: mul o0.xyz, r0.xyzx, cb0[14].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[14].xyzx)).xyz;
    // 137: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 138: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 139: ret
    return output;
}

// source.character.static-map-native-232.v1 / source program bf419bc6d124bd4f80b4fd1b531a6c10
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight232(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[9]=float4(input.lightColor,1.0);
    source[10].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.yzxy
    r2.xyz = ((r0.xyzx)*(r1.yzxy)).xyz;
    // 8: mad r0.xyz, r0.zxyz, r1.zxyz, -r2.xyzx
    r0.xyz = ((r0.zxyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r0.xyz, r0.xyzx, v1.wwww
    r0.xyz = ((r0.xyzx)*(v1.wwww)).xyz;
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r2.xyz, r0.wwww, v7.xyzx
    r2.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r3.xyz, r0.wwww, v5.xyzx
    r3.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r4.xyxx, r4.xyxx
    r1.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r6.xy, v4.xyxx, cb0[5].yyyy
    r6.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r6.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 25: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 26: mul r6.xy, r6.xyxx, cb0[5].zzzz
    r6.xy = ((r6.xyxx)*(source[5].zzzz)).xy;
    // 27: mad r4.xy, cb0[5].xxxx, r4.xyxx, r6.xyxx
    r4.xy = ((source[5].xxxx)*(r4.xyxx)+(r6.xyxx)).xy;
    // 28: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 29: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 30: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 31: div r4.xyw, r5.xyxz, r1.wwww
    r4.xyw = ((r5.xyxz)/(r1.wwww)).xyw;
    // 32: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 34: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 35: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 36: add r3.w, -v2.x, l(1.000000)
    r3.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 38: mul r6.x, r4.w, r4.w
    r6.x = ((r4.wwww)*(r4.wwww)).x;
    // 39: mul_sat r5.w, r5.w, r6.x
    r5.w = (saturate((r5.wwww)*(r6.xxxx))).w;
    // 40: add r5.w, -r5.w, l(1.000000)
    r5.w = ((-(r5.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mul r6.xy, v4.xyxx, cb0[6].xxxx
    r6.xy = ((v4.xyxx)*(source[6].xxxx)).xy;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 44: mul r5.w, r5.w, r6.w
    r5.w = ((r5.wwww)*(r6.wwww)).w;
    // 45: mul r6.w, r1.w, r5.w
    r6.w = ((r1.wwww)*(r5.wwww)).w;
    // 46: mad r3.w, r3.w, r6.w, r3.w
    r3.w = ((r3.wwww)*(r6.wwww)+(r3.wwww)).w;
    // 47: add r1.w, -r1.w, r3.w
    r1.w = ((-(r1.wwww))+(r3.wwww)).w;
    // 48: mul r6.w, r1.w, r2.w
    r6.w = ((r1.wwww)*(r2.wwww)).w;
    // 49: mad r1.w, -r2.w, r1.w, r3.w
    r1.w = ((-(r2.wwww))*(r1.wwww)+(r3.wwww)).w;
    // 50: mad_sat r1.w, r5.w, r1.w, r6.w
    r1.w = (saturate((r5.wwww)*(r1.wwww)+(r6.wwww))).w;
    // 51: mul r2.w, r1.w, l(0.650000)
    r2.w = ((r1.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).w;
    // 52: add r7.xyz, -r4.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r7.xyz = ((-(r4.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 53: mad r7.xyz, r2.wwww, r7.xyzx, r4.xywx
    r7.xyz = ((r2.wwww)*(r7.xyzx)+(r4.xywx)).xyz;
    // 54: dp3 r2.w, r7.xyzx, r7.xyzx
    r2.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 55: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 56: mul r8.xyz, r2.wwww, r7.xyzx
    r8.xyz = ((r2.wwww)*(r7.xyzx)).xyz;
    // 57: dp3 r2.w, r8.xyzx, r2.xyzx
    r2.w = (dot((r8.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 58: mul r8.xyz, r2.wwww, r8.xyzx
    r8.xyz = ((r2.wwww)*(r8.xyzx)).xyz;
    // 59: mad r8.xyz, r8.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r8.xyz = ((r8.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 60: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).w;
    // 61: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 62: div r9.xy, v8.xyxx, v8.wwww
    r9.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 63: mad r9.xy, r9.xyxx, cb2[0].xyxx, cb2[0].wzww
    r9.xy = ((r9.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 64: sample_indexable(texture2d)(float,float,float,float) r9.xyz, r9.xyxx, t6.xyzw, s0
    r9.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 65: mul r9.xyz, r9.xyzx, r9.xyzx
    r9.xyz = ((r9.xyzx)*(r9.xyzx)).xyz;
    // 66: else
    } else {
    // 67: mov r9.xyz, l(1.000000,1.000000,1.000000,0)
    r9.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 68: endif
    }
    // 69: dp3 r1.x, r1.xyzx, r8.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 70: dp3 r1.y, r0.xyzx, r8.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 71: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 72: mad r0.xy, cb0[6].zzzz, r1.xyxx, r0.xyxx
    r0.xy = ((source[6].zzzz)*(r1.xyxx)+(r0.xyxx)).xy;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t5.xyzw, s5, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 74: mul r0.xyz, r0.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)).xyz;
    // 75: mad r0.xyz, cb0[6].wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((source[6].wwww)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 76: add r0.xyz, r0.xyzx, -cb0[6].wwww
    r0.xyz = ((r0.xyzx)+(-(source[6].wwww))).xyz;
    // 77: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 78: mul r2.w, r4.z, cb0[7].x
    r2.w = ((r4.zzzz)*(source[7].xxxx)).w;
    // 79: mul r8.xyz, cb0[2].xyzx, cb0[7].yyyy
    r8.xyz = ((source[2].xyzx)*(source[7].yyyy)).xyz;
    // 80: mul r5.xyz, r5.xyzx, r8.xyzx
    r5.xyz = ((r5.xyzx)*(r8.xyzx)).xyz;
    // 81: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 82: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 83: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 84: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 85: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 86: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 87: mul r1.xyz, cb0[3].xyzx, cb0[7].zzzz
    r1.xyz = ((source[3].xyzx)*(source[7].zzzz)).xyz;
    // 88: mul r5.xyz, r6.xyzx, r1.xyzx
    r5.xyz = ((r6.xyzx)*(r1.xyzx)).xyz;
    // 89: dp3 r2.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 90: mad r1.xyz, -r1.xyzx, r6.xyzx, r2.wwww
    r1.xyz = ((-(r1.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 91: mad r1.xyz, cb0[8].xxxx, r1.xyzx, r5.xyzx
    r1.xyz = ((source[8].xxxx)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 92: add r1.xyz, -r0.xyzx, r1.xyzx
    r1.xyz = ((-(r0.xyzx))+(r1.xyzx)).xyz;
    // 93: mad r0.xyz, r1.wwww, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 94: dp3 r1.x, r7.xyzx, r3.xyzx
    r1.x = (dot((r7.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 95: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 96: min r1.y, r1.x, l(1.000000)
    r1.y = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 97: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t4.xyzw, s6, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture6.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 98: mul r5.xyz, cb0[4].xyzx, cb0[8].yyyy
    r5.xyz = ((source[4].xyzx)*(source[8].yyyy)).xyz;
    // 99: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 100: dp3 r1.z, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.z = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 101: add r5.xyz, -r6.xyzx, r1.zzzz
    r5.xyz = ((-(r6.xyzx))+(r1.zzzz)).xyz;
    // 102: mad r5.xyz, cb0[8].xxxx, r5.xyzx, r6.xyzx
    r5.xyz = ((source[8].xxxx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 103: mad r5.xyz, cb0[8].zzzz, r5.xyzx, -r3.xyzx
    r5.xyz = ((source[8].zzzz)*(r5.xyzx)+(-(r3.xyzx))).xyz;
    // 104: mad r3.xyz, r1.wwww, r5.xyzx, r3.xyzx
    r3.xyz = ((r1.wwww)*(r5.xyzx)+(r3.xyzx)).xyz;
    // 105: mad r2.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r2.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 106: dp3 r0.w, r2.xyzx, r2.xyzx
    r0.w = (dot((r2.xyzx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 107: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 108: div r2.xyz, r2.xyzx, r0.wwww
    r2.xyz = ((r2.xyzx)/(r0.wwww)).xyz;
    // 109: dp3 r0.w, r2.xyzx, r4.xywx
    r0.w = (dot((r2.xyzx).xyz,(r4.xywx).xyz).xxxx).w;
    // 110: lt r1.z, |r0.w|, l(0.000001)
    r1.z = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 111: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 112: mul r0.w, r0.w, cb0[8].w
    r0.w = ((r0.wwww)*(source[8].wwww)).w;
    // 113: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 114: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 115: movc r0.w, r1.z, l(0), r0.w
    r0.w = ((asuint(r1.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 116: mul r2.xyz, r3.xyzx, r0.wwww
    r2.xyz = ((r3.xyzx)*(r0.wwww)).xyz;
    // 117: mad r0.xyz, r1.yyyy, r0.xyzx, r2.xyzx
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.xyzx)).xyz;
    // 118: mul r1.xyz, r1.xxxx, cb2[3].xyzx
    r1.xyz = ((r1.xxxx)*(passValues[3].xyzx)).xyz;
    // 119: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 120: mul r0.xyz, r9.xyzx, r0.xyzx
    r0.xyz = ((r9.xyzx)*(r0.xyzx)).xyz;
    // 121: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 122: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 123: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 124: ret
    return output;
}

// source.character.static-map-native-233.v1 / source program 829844df6e9f6e4b9dcc8e95590cfeb8
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight233(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[11]=float4(input.lightColor,1.0);
    source[12].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0;
    // 1: dp3 r0.x, v1.xyzx, v1.xyzx
    r0.x = (dot((v1.xyzx).xyz,(v1.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v1.zxyz
    r0.xyz = ((r0.xxxx)*(v1.zxyz)).xyz;
    // 4: dp3 r0.w, v0.xyzx, v0.xyzx
    r0.w = (dot((v0.xyzx).xyz,(v0.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v0.xyzx
    r1.xyz = ((r0.wwww)*(v0.xyzx)).xyz;
    // 7: mul r2.xyz, r0.xyzx, r1.yzxy
    r2.xyz = ((r0.xyzx)*(r1.yzxy)).xyz;
    // 8: mad r0.xyz, r0.zxyz, r1.zxyz, -r2.xyzx
    r0.xyz = ((r0.zxyz)*(r1.zxyz)+(-(r2.xyzx))).xyz;
    // 9: mul r0.xyz, r0.xyzx, v1.wwww
    r0.xyz = ((r0.xyzx)*(v1.wwww)).xyz;
    // 10: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).w;
    // 11: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 12: div r2.xy, v8.xyxx, v8.wwww
    r2.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 13: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 14: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 15: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 16: else
    } else {
    // 17: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 18: endif
    }
    // 19: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 20: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 21: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 22: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 23: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 24: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 25: mul r5.xy, v4.xyxx, cb0[1].xyxx
    r5.xy = ((v4.xyxx)*(source[1].xyxx)).xy;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r6.xyz, r5.xyxx, t0.xywz, s1, l(0.000000)
    r6.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 27: mad r5.zw, r6.xxxy, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r6.xxxy)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 28: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 29: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 33: mul r7.xy, r5.zwzz, cb0[6].wwww
    r7.xy = ((r5.zwzz)*(source[6].wwww)).xy;
    // 34: dp3 r1.w, r7.xyzx, r7.xyzx
    r1.w = (dot((r7.xyzx).xyz,(r7.xyzx).xyz).xxxx).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: div r6.xyw, r7.xyxz, r1.wwww
    r6.xyw = ((r7.xyxz)/(r1.wwww)).xyw;
    // 37: mul r5.zw, v4.xxxy, cb0[7].xxxx
    r5.zw = ((v4.xxxy)*(source[7].xxxx)).zw;
    // 38: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.zwzz, t1.xyzw, s2, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 39: add r1.w, -v2.x, l(1.000000)
    r1.w = ((-(v2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: add r2.w, r1.w, -cb0[7].z
    r2.w = ((r1.wwww)+(-(source[7].zzzz))).w;
    // 41: mul_sat r2.w, r2.w, cb0[8].x
    r2.w = (saturate((r2.wwww)*(source[8].xxxx))).w;
    // 42: mul r3.w, r2.w, r2.w
    r3.w = ((r2.wwww)*(r2.wwww)).w;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 44: mul r4.w, r6.w, r8.w
    r4.w = ((r6.wwww)*(r8.wwww)).w;
    // 45: mad r1.w, -r2.w, r2.w, r1.w
    r1.w = ((-(r2.wwww))*(r2.wwww)+(r1.wwww)).w;
    // 46: mad_sat r1.w, r4.w, r1.w, r3.w
    r1.w = (saturate((r4.wwww)*(r1.wwww)+(r3.wwww))).w;
    // 47: mul r1.w, r1.w, r7.w
    r1.w = ((r1.wwww)*(r7.wwww)).w;
    // 48: add r9.xyz, -r6.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r9.xyz = ((-(r6.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 49: mad r6.xyw, r1.wwww, r9.xyxz, r6.xyxw
    r6.xyw = ((r1.wwww)*(r9.xyxz)+(r6.xyxw)).xyw;
    // 50: dp3 r2.w, r6.xywx, r6.xywx
    r2.w = (dot((r6.xywx).xyz,(r6.xywx).xyz).xxxx).w;
    // 51: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 52: mul r9.xyz, r2.wwww, r6.xywx
    r9.xyz = ((r2.wwww)*(r6.xywx)).xyz;
    // 53: dp3 r2.w, r9.xyzx, r3.xyzx
    r2.w = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 54: mul r9.xyz, r2.wwww, r9.xyzx
    r9.xyz = ((r2.wwww)*(r9.xyzx)).xyz;
    // 55: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 56: add r2.w, r8.w, l(-0.333300)
    r2.w = ((r8.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 57: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 58: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 59: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 60: dp3 r1.y, r0.xyzx, r9.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 61: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 62: mad r0.xy, cb0[9].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[9].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 63: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s5, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 64: mul r0.xyz, r0.xyzx, cb0[4].xyzx
    r0.xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 65: mad r0.xyz, cb0[9].zzzz, r0.xyzx, r0.xyzx
    r0.xyz = ((source[9].zzzz)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 66: add r0.xyz, r0.xyzx, -cb0[9].zzzz
    r0.xyz = ((r0.xyzx)+(-(source[9].zzzz))).xyz;
    // 67: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 68: mul r2.w, r6.z, cb0[9].w
    r2.w = ((r6.zzzz)*(source[9].wwww)).w;
    // 69: mul r9.xyz, cb0[5].xyzx, cb0[10].xxxx
    r9.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 70: mul r8.xyz, r8.xyzx, r9.xyzx
    r8.xyz = ((r8.xyzx)*(r9.xyzx)).xyz;
    // 71: mad r1.xyz, r2.wwww, r1.xyzx, r8.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r8.xyzx)).xyz;
    // 72: mov_sat r0.xyz, -r0.xyzx
    r0.xyz = (saturate(-(r0.xyzx))).xyz;
    // 73: mad r0.xyz, -r2.wwww, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r2.wwww))*(r0.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 74: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 75: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 76: min r0.xyz, r0.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 77: mul r1.xyz, cb0[2].xyzx, cb0[8].yyyy
    r1.xyz = ((source[2].xyzx)*(source[8].yyyy)).xyz;
    // 78: mad r8.xyz, r1.xyzx, r7.xyzx, -r0.xyzx
    r8.xyz = ((r1.xyzx)*(r7.xyzx)+(-(r0.xyzx))).xyz;
    // 79: mad r0.xyz, r1.wwww, r8.xyzx, r0.xyzx
    r0.xyz = ((r1.wwww)*(r8.xyzx)+(r0.xyzx)).xyz;
    // 80: dp3 r2.w, r6.xywx, r4.xyzx
    r2.w = (dot((r6.xywx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 81: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 82: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 83: mul r4.xyz, r2.xyzx, r3.wwww
    r4.xyz = ((r2.xyzx)*(r3.wwww)).xyz;
    // 84: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, r5.xyxx, t4.xyzw, s4, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 85: mad r1.xyz, r1.xyzx, r7.xyzx, -r5.xyzx
    r1.xyz = ((r1.xyzx)*(r7.xyzx)+(-(r5.xyzx))).xyz;
    // 86: mad r1.xyz, r1.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r1.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 87: mul r1.xyz, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((r1.xyzx)*(source[3].xyzx)).xyz;
    // 88: mul r1.xyz, r1.xyzx, cb0[8].zzzz
    r1.xyz = ((r1.xyzx)*(source[8].zzzz)).xyz;
    // 89: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 90: dp3 r0.w, r3.xyzx, r3.xyzx
    r0.w = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 91: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 92: div r3.xyz, r3.xyzx, r0.wwww
    r3.xyz = ((r3.xyzx)/(r0.wwww)).xyz;
    // 93: dp3 r0.w, r3.xyzx, r6.xywx
    r0.w = (dot((r3.xyzx).xyz,(r6.xywx).xyz).xxxx).w;
    // 94: lt r1.w, |r0.w|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 95: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 96: mul r0.w, r0.w, cb0[8].w
    r0.w = ((r0.wwww)*(source[8].wwww)).w;
    // 97: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 98: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 99: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 100: mul r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)*(r0.wwww)).xyz;
    // 101: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 102: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 103: min r1.xyz, r1.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 104: mad r0.xyz, r0.xyzx, r4.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r4.xyzx)+(r1.xyzx)).xyz;
    // 105: mul r1.xyz, r2.wwww, cb2[3].xyzx
    r1.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 106: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 107: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 108: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 109: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 110: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 111: ret
    return output;
}

// source.character.static-map-native-234.v1 / source program 161839b31fa7b545bc21941b1df7babb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight234(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[10]=float4(input.lightColor,1.0);
    source[11].x=1.0;
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0 = input.values[0], v1 = input.values[1], v2 = input.values[2], v3 = input.values[3], v4 = input.values[4], v5 = input.values[5], v6 = input.values[6], v7 = input.values[7], v8 = input.values[8], v9 = input.values[9];
    float4 r0=0.0, r1=0.0, r2=0.0, r3=0.0, r4=0.0, r5=0.0, r6=0.0, r7=0.0, r8=0.0, r9=0.0, r10=0.0;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r3.xyz, r0.wwww, v7.xyzx
    r3.xyz = ((r0.wwww)*(v7.xyzx)).xyz;
    // 13: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 14: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 15: mul r4.xyz, r0.wwww, v5.xyzx
    r4.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xyz, v4.xyxx, t0.xywz, s1, l(0.000000)
    r5.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xywz).xyz;
    // 17: mad r5.xy, r5.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r5.xy = ((r5.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 18: dp2 r1.w, r5.xyxx, r5.xyxx
    r1.w = (dot((r5.xyxx).xy,(r5.xyxx).xy).xxxx).w;
    // 19: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 20: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 21: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 22: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 23: mul r5.xy, r5.xyxx, cb0[6].xxxx
    r5.xy = ((r5.xyxx)*(source[6].xxxx)).xy;
    // 24: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r5.xyw, r6.xyxz, r1.wwww
    r5.xyw = ((r6.xyxz)/(r1.wwww)).xyw;
    // 28: max r1.w, cb0[6].y, l(0.000000)
    r1.w = (max(source[6].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 29: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 30: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = (((r2.wwww) != 0.f ? 1.f / (r2.wwww) : 0.f)).w;
    // 32: dp3 r6.x, r1.xyzx, r5.xywx
    r6.x = (dot((r1.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 33: dp3 r6.y, r2.xyzx, r5.xywx
    r6.y = (dot((r2.xyzx).xyz,(r5.xywx).xyz).xxxx).y;
    // 34: dp3 r6.z, r0.xyzx, r5.xywx
    r6.z = (dot((r0.xyzx).xyz,(r5.xywx).xyz).xxxx).z;
    // 35: max r0.xyz, cb0[1].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[1].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 36: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 37: dp3 r0.x, r6.xyzx, r0.xyzx
    r0.x = (dot((r6.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 38: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mad r0.x, r0.x, l(0.500000), cb0[7].x
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].xxxx)).x;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 41: mul r0.y, r5.w, r5.w
    r0.y = ((r5.wwww)*(r5.wwww)).y;
    // 42: mul_sat r0.y, r0.y, r6.w
    r0.y = (saturate((r0.yyyy)*(r6.wwww))).y;
    // 43: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 44: mul r7.xy, v4.xyxx, cb0[7].yyyy
    r7.xy = ((v4.xyxx)*(source[7].yyyy)).xy;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r7.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r0.z, r7.w, r7.w
    r0.z = ((r7.wwww)*(r7.wwww)).z;
    // 47: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 48: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 49: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 50: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 51: mul r1.w, r0.z, r2.w
    r1.w = ((r0.zzzz)*(r2.wwww)).w;
    // 52: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 53: mad_sat r0.x, r0.y, r0.x, r1.w
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.wwww))).x;
    // 54: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 55: add r8.xyz, -r5.xywx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r8.xyz = ((-(r5.xywx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 56: mad r8.xyz, r0.yyyy, r8.xyzx, r5.xywx
    r8.xyz = ((r0.yyyy)*(r8.xyzx)+(r5.xywx)).xyz;
    // 57: dp3 r0.y, r8.xyzx, r8.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).y;
    // 58: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 59: mul r9.xyz, r0.yyyy, r8.xyzx
    r9.xyz = ((r0.yyyy)*(r8.xyzx)).xyz;
    // 60: dp3 r0.y, r9.xyzx, r3.xyzx
    r0.y = (dot((r9.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 61: mul r9.xyz, r0.yyyy, r9.xyzx
    r9.xyz = ((r0.yyyy)*(r9.xyzx)).xyz;
    // 62: mad r9.xyz, r9.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r3.xyzx
    r9.xyz = ((r9.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r3.xyzx))).xyz;
    // 63: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).y;
    // 64: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 65: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 66: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 67: sample_indexable(texture2d)(float,float,float,float) r10.xyz, r0.yzyy, t5.xyzw, s0
    r10.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 68: mul r10.xyz, r10.xyzx, r10.xyzx
    r10.xyz = ((r10.xyzx)*(r10.xyzx)).xyz;
    // 69: else
    } else {
    // 70: mov r10.xyz, l(1.000000,1.000000,1.000000,0)
    r10.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 71: endif
    }
    // 72: dp3 r1.x, r1.xyzx, r9.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).x;
    // 73: dp3 r1.y, r2.xyzx, r9.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r9.xyzx).xyz).xxxx).y;
    // 74: mul r0.yz, cb0[0].xxyx, l(0.000000, 0.000300, 0.000300, 0.000000)
    r0.yz = ((source[0].xxyx)*(float4(0.000000,0.000300,0.000300,0.000000))).yz;
    // 75: mad r0.yz, cb0[7].zzzz, r1.xxyx, r0.yyzy
    r0.yz = ((source[7].zzzz)*(r1.xxyx)+(r0.yyzy)).yz;
    // 76: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r0.yzyy, t4.xyzw, s4, l(0.000000)
    r1.xyz = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 77: mul r1.xyz, r1.xyzx, cb0[2].xyzx
    r1.xyz = ((r1.xyzx)*(source[2].xyzx)).xyz;
    // 78: mad r1.xyz, cb0[7].wwww, r1.xyzx, r1.xyzx
    r1.xyz = ((source[7].wwww)*(r1.xyzx)+(r1.xyzx)).xyz;
    // 79: add r1.xyz, r1.xyzx, -cb0[7].wwww
    r1.xyz = ((r1.xyzx)+(-(source[7].wwww))).xyz;
    // 80: mov_sat r2.xyz, r1.xyzx
    r2.xyz = (saturate(r1.xyzx)).xyz;
    // 81: mul r0.y, r5.z, cb0[8].x
    r0.y = ((r5.zzzz)*(source[8].xxxx)).y;
    // 82: mul r9.xyz, cb0[3].xyzx, cb0[8].yyyy
    r9.xyz = ((source[3].xyzx)*(source[8].yyyy)).xyz;
    // 83: mul r6.xyz, r6.xyzx, r9.xyzx
    r6.xyz = ((r6.xyzx)*(r9.xyzx)).xyz;
    // 84: mad r2.xyz, r0.yyyy, r2.xyzx, r6.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 85: mov_sat r1.xyz, -r1.xyzx
    r1.xyz = (saturate(-(r1.xyzx))).xyz;
    // 86: mad r1.xyz, -r0.yyyy, r1.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r0.yyyy))*(r1.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 87: mul r1.xyz, r1.xyzx, r2.xyzx
    r1.xyz = ((r1.xyzx)*(r2.xyzx)).xyz;
    // 88: max r1.xyz, r1.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r1.xyz = (max(r1.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 89: min r1.xyz, r1.xyzx, l(999.000000, 999.000000, 999.000000, 0.000000)
    r1.xyz = (min(r1.xyzx,float4(999.000000,999.000000,999.000000,0.000000))).xyz;
    // 90: mul r2.xyz, cb0[4].xyzx, cb0[8].zzzz
    r2.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 91: mul r6.xyz, r7.xyzx, r2.xyzx
    r6.xyz = ((r7.xyzx)*(r2.xyzx)).xyz;
    // 92: dp3 r0.y, r6.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r6.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 93: mad r2.xyz, -r2.xyzx, r7.xyzx, r0.yyyy
    r2.xyz = ((-(r2.xyzx))*(r7.xyzx)+(r0.yyyy)).xyz;
    // 94: mad r2.xyz, cb0[9].xxxx, r2.xyzx, r6.xyzx
    r2.xyz = ((source[9].xxxx)*(r2.xyzx)+(r6.xyzx)).xyz;
    // 95: add r2.xyz, -r1.xyzx, r2.xyzx
    r2.xyz = ((-(r1.xyzx))+(r2.xyzx)).xyz;
    // 96: mad r1.xyz, r0.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r0.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 97: dp3 r0.y, r8.xyzx, r4.xyzx
    r0.y = (dot((r8.xyzx).xyz,(r4.xyzx).xyz).xxxx).y;
    // 98: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 99: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t3.xyzw, s5, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 100: mul r4.xyz, cb0[5].xyzx, cb0[9].yyyy
    r4.xyz = ((source[5].xyzx)*(source[9].yyyy)).xyz;
    // 101: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 102: dp3 r1.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 103: add r4.xyz, -r7.xyzx, r1.wwww
    r4.xyz = ((-(r7.xyzx))+(r1.wwww)).xyz;
    // 104: mad r4.xyz, cb0[9].xxxx, r4.xyzx, r7.xyzx
    r4.xyz = ((source[9].xxxx)*(r4.xyzx)+(r7.xyzx)).xyz;
    // 105: mad r4.xyz, cb0[9].zzzz, r4.xyzx, -r2.xyzx
    r4.xyz = ((source[9].zzzz)*(r4.xyzx)+(-(r2.xyzx))).xyz;
    // 106: mad r2.xyz, r0.xxxx, r4.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 107: mad r3.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 108: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 109: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 110: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 111: dp3 r0.x, r3.xyzx, r5.xywx
    r0.x = (dot((r3.xyzx).xyz,(r5.xywx).xyz).xxxx).x;
    // 112: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 114: mul r0.x, r0.x, cb0[9].w
    r0.x = ((r0.xxxx)*(source[9].wwww)).x;
    // 115: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 116: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 117: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 118: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 119: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 120: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 121: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 122: mul r0.xyz, r10.xyzx, r0.xyzx
    r0.xyz = ((r10.xyzx)*(r0.xyzx)).xyz;
    // 123: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 124: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 125: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 126: ret
    return output;
}
