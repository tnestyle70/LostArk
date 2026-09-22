// source.character.static-map-native-224.v1 / source program a54c39add512e849b1b65e7fc6b1ba55
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapSL10Direct224(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[4]=float4(input.lightColor,1.0);
    source[10].x=1.0;
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
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
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapSL10Direct225(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[7]=float4(input.lightColor,1.0);
    source[13].x=1.0;
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
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
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapSL10Direct226(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[3]=float4(input.lightColor,1.0);
    source[9].x=1.0;
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
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

// source.character.static-map-native-237.v1 / source program e73b0ff02ebe4748b14ec48757093949
SOURCE_CHARACTER_NATIVE_OUTPUT SourceMapSL10Direct237(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output = (SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64];
    [unroll] for (uint i=0u;i<64u;++i) source[i]=g_SourceCharacterLightConstants[i];
    source[6].x=(g_SourceCharacterTime.xxxx).x;
    source[8]=float4(input.lightColor,1.0);
    source[0]=float4(1.f,0.f,0.f,1.f);
    float4 projection[4]; [unroll] for(uint p=0u;p<4u;++p) projection[p]=input.projection[p];
    float4 passValues[5] = {float4(0.5,-0.5,0.5,0.5),float4(0.f,0.f,1.f/(g_ProjMatrix._43*100.f),g_ProjMatrix._33/(g_ProjMatrix._43*100.f)),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
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
    r0.x = ((g_SourceMapSceneDepth.SampleLevel(SourceMapDepthSampler,(r0.xyxx).xy,0.f)).xyzw).x;
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
