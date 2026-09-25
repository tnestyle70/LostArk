SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1400(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=float4(input.lightColor,1.f);
    source[11].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s2
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s2
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s2
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s2
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // 19: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 20: movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 21: add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // 22: mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // 23: add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // 24: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 25: mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v0.w, cb0[4].y
    r0.w = ((-(v0.wwww))+(source[4].yyyy)).w;
    // 30: add r0.w, r0.w, cb0[4].x
    r0.w = ((r0.wwww)+(source[4].xxxx)).w;
    // 31: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 32: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 33: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 34: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 35: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 36: endif
    }
    // 37: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 40: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 41: mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t2.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 43: mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // 44: mul r3.xyz, r2.xyzx, r3.xyzx
    r3.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r4.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r4.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 46: mad r4.xy, r4.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r4.xy = ((r4.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 47: dp2 r2.w, r4.xyxx, r4.xyxx
    r2.w = (dot((r4.xyxx).xy,(r4.xyxx).xy).xxxx).w;
    // 48: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 50: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 51: add r5.z, r2.w, l(0.000010)
    r5.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: mul r4.xy, r4.xyxx, cb0[3].xxxx
    r4.xy = ((r4.xyxx)*(source[3].xxxx)).xy;
    // 53: mul r5.xy, r4.xyxx, v0.wwww
    r5.xy = ((r4.xyxx)*(v0.wwww)).xy;
    // 54: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r4.xyz, r5.xyzx, r2.wwww
    r4.xyz = ((r5.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, r4.xyzx, r1.yzwy
    r2.w = (dot((r4.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 58: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 59: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 61: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 62: mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 63: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 64: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 65: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 66: dp3 r1.x, r1.xyzx, r4.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).x;
    // 67: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 68: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 69: mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // 70: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 71: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 72: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 73: mul r1.xyz, r2.xyzx, r1.xxxx
    r1.xyz = ((r2.xyzx)*(r1.xxxx)).xyz;
    // 74: mad r1.xyz, r3.wwww, r3.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r3.xyzx)+(r1.xyzx)).xyz;
    // 75: mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 76: mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 78: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 79: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 80: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 81: ret
    return output;
}

// source.character.static-map-native-1401.v1 / source program bd2d9d76c2e71846a4c52e67e1996291
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1401(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=float4(input.lightColor,1.f);
    source[11].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v6.yyyy, cb0[7].xyzw
    r0.xyzw = ((v6.yyyy)*(source[7].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[6].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[6].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[8].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[9].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[9].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t1.xyzw, s3
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[10].wwzw
    r2.yz = (source[10].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t1.yxzw, s3
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t1.yzxw, s3
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[10].zwzz
    r2.xy = ((r0.xyxx)+(source[10].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t1.yzwx, s3
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[10].xyxx
    r0.xy = ((r0.xyxx)*(source[10].xyxx)).xy;
    // 19: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 20: movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 21: add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // 22: mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // 23: add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // 24: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 25: mul r0.xyz, r0.xxxx, cb0[11].xxxx
    r0.xyz = ((r0.xxxx)*(source[11].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v0.w, l(1.000000)
    r0.w = ((-(v0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 30: add r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)+(source[4].yyyy)).w;
    // 31: add_sat r0.w, r0.w, cb0[4].x
    r0.w = (saturate((r0.wwww)+(source[4].xxxx))).w;
    // 32: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 33: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 34: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 35: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 36: endif
    }
    // 37: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 40: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 41: mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v2.xyxx, t3.xyzw, s1, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 43: mul r3.xyz, cb0[1].xyzx, cb0[3].yyyy
    r3.xyz = ((source[1].xyzx)*(source[3].yyyy)).xyz;
    // 44: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 46: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 47: dp2 r2.w, r3.xyxx, r3.xyxx
    r2.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 48: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 49: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 50: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 51: add r4.z, r2.w, l(0.000010)
    r4.z = ((r2.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 52: mul r3.xy, r3.xyxx, cb0[3].xxxx
    r3.xy = ((r3.xyxx)*(source[3].xxxx)).xy;
    // 53: mul r4.xy, r3.xyxx, v0.wwww
    r4.xy = ((r3.xyxx)*(v0.wwww)).xy;
    // 54: dp3 r2.w, r4.xyzx, r4.xyzx
    r2.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 55: sqrt r2.w, r2.w
    r2.w = (sqrt(r2.wwww)).w;
    // 56: div r3.xyz, r4.xyzx, r2.wwww
    r3.xyz = ((r4.xyzx)/(r2.wwww)).xyz;
    // 57: dp3 r2.w, r3.xyzx, r1.yzwy
    r2.w = (dot((r3.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 58: max r2.w, r2.w, l(0.000000)
    r2.w = (max(r2.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 59: min r3.w, r2.w, l(1.000000)
    r3.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 60: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v2.xyxx, t2.xyzw, s2, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 61: mul r5.xyz, cb0[2].xyzx, cb0[3].zzzz
    r5.xyz = ((source[2].xyzx)*(source[3].zzzz)).xyz;
    // 62: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 63: mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 64: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 65: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 66: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 67: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 68: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 69: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 70: mul r1.x, r1.x, cb0[3].w
    r1.x = ((r1.xxxx)*(source[3].wwww)).x;
    // 71: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 72: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 73: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 74: mul r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)*(r1.xxxx)).xyz;
    // 75: mad r1.xyz, r3.wwww, r2.xyzx, r1.xyzx
    r1.xyz = ((r3.wwww)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 76: mul r2.xyz, r2.wwww, cb2[3].xyzx
    r2.xyz = ((r2.wwww)*(passValues[3].xyzx)).xyz;
    // 77: mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 78: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 79: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 80: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 81: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 82: ret
    return output;
}

// source.character.static-map-native-1402.v1 / source program 5d326d0fc39d694f834d8be4978cb8ee
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1402(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=float4(input.lightColor,1.f);
    source[5].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[5].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[5].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v8.xyxx, v8.wwww
    r0.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s0
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 14: add r0.w, r2.w, l(-0.333300)
    r0.w = ((r2.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 15: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 16: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 17: dp3 r0.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 18: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t2.zwxy, s3, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).xy;
    // 19: mul r1.w, r3.y, cb0[2].z
    r1.w = ((r3.yyyy)*(source[2].zzzz)).w;
    // 20: add r4.xyz, -r2.xyzx, r0.wwww
    r4.xyz = ((-(r2.xyzx))+(r0.wwww)).xyz;
    // 21: mad r2.xyz, r1.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r1.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 22: mad r4.xyz, cb0[2].wwww, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[2].wwww)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 23: mad r3.yzw, r3.yyyy, r4.xxyz, l(0.000000, 1.000000, 1.000000, 1.000000)
    r3.yzw = ((r3.yyyy)*(r4.xxyz)+(float4(0.000000,1.000000,1.000000,1.000000))).yzw;
    // 24: mul r2.xyz, r2.xyzx, r3.yzwy
    r2.xyz = ((r2.xyzx)*(r3.yzwy)).xyz;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r3.yz, v4.xyxx, t0.zxyw, s1, l(0.000000)
    r3.yz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zxyw).yz;
    // 26: mad r3.yz, r3.yyzy, l(0.000000, 2.000000, 2.000000, 0.000000), l(0.000000, -1.000000, -1.000000, 0.000000)
    r3.yz = ((r3.yyzy)*(float4(0.000000,2.000000,2.000000,0.000000))+(float4(0.000000,-1.000000,-1.000000,0.000000))).yz;
    // 27: dp2 r0.w, r3.yzyy, r3.yzyy
    r0.w = (dot((r3.yzyy).xy,(r3.yzyy).xy).xxxx).w;
    // 28: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 29: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 30: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 31: add r4.z, r0.w, l(0.000010)
    r4.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 32: mul r4.xy, r3.yzyy, cb0[2].xxxx
    r4.xy = ((r3.yzyy)*(source[2].xxxx)).xy;
    // 33: dp3 r0.w, r4.xyzx, r4.xyzx
    r0.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 34: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 35: div r3.yzw, r4.xxyz, r0.wwww
    r3.yzw = ((r4.xxyz)/(r0.wwww)).yzw;
    // 36: dp3 r0.w, r3.yzwy, r1.xyzx
    r0.w = (dot((r3.yzwy).xyz,(r1.xyzx).xyz).xxxx).w;
    // 37: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: min r1.x, r0.w, l(1.000000)
    r1.x = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 39: mul r1.xyz, r0.xyzx, r1.xxxx
    r1.xyz = ((r0.xyzx)*(r1.xxxx)).xyz;
    // 40: mul r3.yzw, cb0[1].xxyz, cb0[3].xxxx
    r3.yzw = ((source[1].xxyz)*(source[3].xxxx)).yzw;
    // 41: mul r3.xyz, r3.yzwy, r3.xxxx
    r3.xyz = ((r3.yzwy)*(r3.xxxx)).xyz;
    // 42: mad r0.xyz, r3.xyzx, r0.xyzx, -r1.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 43: mad r0.xyz, r3.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 44: mul r0.xyz, r0.xyzx, r2.xyzx
    r0.xyz = ((r0.xyzx)*(r2.xyzx)).xyz;
    // 45: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 46: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 47: mul o0.xyz, r0.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 48: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 49: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 50: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 51: ret
    return output;
}

// source.character.static-map-native-1403.v1 / source program e4f30a099c13d740bd43b3575f7202cb
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1403(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[6];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=float4(input.lightColor,1.f);
    source[11].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t4.xyzw, s1
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t2.yzwx, s4, l(0.000000)
    r0.w = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 11: div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t3.xyzw, s0, l(0.000000)
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 14: min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 15: mad r1.y, r1.x, cb2[1].x, cb2[1].y
    r1.y = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // 16: mad r1.x, r1.x, cb2[1].z, -cb2[1].w
    r1.x = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).x;
    // 17: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 18: add r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 19: add r1.y, -cb0[9].y, l(1.000000)
    r1.y = ((-(source[9].yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: add r1.x, r1.x, -v6.w
    r1.x = ((r1.xxxx)+(-(v6.wwww))).x;
    // 21: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 22: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 23: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 24: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 25: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 26: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 27: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 28: endif
    }
    // 29: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 30: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 31: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 32: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 33: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 34: mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v2.xyxx, t0.xyzw, s2, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 36: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 37: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 38: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 41: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 42: mul r4.xy, r3.xyxx, cb0[7].xxxx
    r4.xy = ((r3.xyxx)*(source[7].xxxx)).xy;
    // 43: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 44: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 45: mul r3.xyz, r1.wwww, r4.xyzx
    r3.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 46: dp3 r1.w, r3.xyzx, r1.xyzx
    r1.w = (dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 47: mul r4.xyz, r1.wwww, r3.xyzx
    r4.xyz = ((r1.wwww)*(r3.xyzx)).xyz;
    // 48: mad r1.xyz, r4.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r4.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 49: add r4.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r4.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 50: dp2 r5.x, cb0[2].xyxx, r4.xyxx
    r5.x = (dot((source[2].xyxx).xy,(r4.xyxx).xy).xxxx).x;
    // 51: dp2 r5.y, cb0[3].xyxx, r4.xyxx
    r5.y = (dot((source[3].xyxx).xy,(r4.xyxx).xy).xxxx).y;
    // 52: add r4.xy, r5.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r4.xy = ((r5.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 53: mul r4.xy, r4.xyxx, cb0[4].xyxx
    r4.xy = ((r4.xyxx)*(source[4].xyxx)).xy;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t1.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 55: dp3 r1.w, r4.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r4.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 56: add r5.xyz, -r4.xyzx, r1.wwww
    r5.xyz = ((-(r4.xyzx))+(r1.wwww)).xyz;
    // 57: mad r4.xyz, cb0[8].yyyy, r5.xyzx, r4.xyzx
    r4.xyz = ((source[8].yyyy)*(r5.xyzx)+(r4.xyzx)).xyz;
    // 58: mul r5.xyz, r4.xyzx, cb0[8].wwww
    r5.xyz = ((r4.xyzx)*(source[8].wwww)).xyz;
    // 59: mul r5.xyz, r5.xyzx, cb0[6].xyzx
    r5.xyz = ((r5.xyzx)*(source[6].xyzx)).xyz;
    // 60: mad r5.xyz, r5.xyzx, cb2[4].wwww, cb2[4].xyzx
    r5.xyz = ((r5.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 61: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 62: mul r4.xyz, r4.xyzx, cb0[8].zzzz
    r4.xyz = ((r4.xyzx)*(source[8].zzzz)).xyz;
    // 63: mul r4.xyz, r4.xyzx, cb0[5].xyzx
    r4.xyz = ((r4.xyzx)*(source[5].xyzx)).xyz;
    // 64: mul r4.xyz, r6.xyzx, r4.xyzx
    r4.xyz = ((r6.xyzx)*(r4.xyzx)).xyz;
    // 65: mad r4.xyz, r4.xyzx, cb2[3].wwww, cb2[3].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 66: dp3_sat r1.w, r3.xyzx, r2.xyzx
    r1.w = (saturate(dot((r3.xyzx).xyz,(r2.xyzx).xyz).xxxx)).w;
    // 67: lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 68: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 69: dp3_sat r1.x, r1.xyzx, r2.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 70: lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 71: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 72: mul r1.x, r1.x, cb0[9].x
    r1.x = ((r1.xxxx)*(source[9].xxxx)).x;
    // 73: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 74: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 75: mul r1.xyz, r5.xyzx, r1.xxxx
    r1.xyz = ((r5.xyzx)*(r1.xxxx)).xyz;
    // 76: mad r1.xyz, r4.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r4.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 77: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 78: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 79: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 80: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 81: ret
    return output;
}

// source.character.static-map-native-1404.v1 / source program 2396a13d2b0c334595e196d5e8dc89f6
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1404(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=float4(input.lightColor,1.f);
    source[5].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 4: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 5: mul r0.yzw, r0.yyyy, v5.xxyz
    r0.yzw = ((r0.yyyy)*(v5.xxyz)).yzw;
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[5].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[5].xxxx)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: div r1.xy, v8.xyxx, v8.wwww
    r1.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 9: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t3.xyzw, s0
    r1.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyz, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r2.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 16: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 17: add r3.xyz, -r2.xyzx, r1.wwww
    r3.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 18: mad r2.xyz, cb0[2].zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((source[2].zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 19: mul r3.xyz, cb0[0].xyzx, cb0[2].wwww
    r3.xyz = ((source[0].xyzx)*(source[2].wwww)).xyz;
    // 20: mul r2.xyz, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r3.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r3.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 22: mad r3.xy, r3.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r3.xy = ((r3.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 23: dp2 r1.w, r3.xyxx, r3.xyxx
    r1.w = (dot((r3.xyxx).xy,(r3.xyxx).xy).xxxx).w;
    // 24: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 25: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 28: mul r3.xy, r3.xyxx, cb0[2].xxxx
    r3.xy = ((r3.xyxx)*(source[2].xxxx)).xy;
    // 29: mul r4.xy, r3.xyxx, v2.wwww
    r4.xy = ((r3.xyxx)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r3.xyz, r4.xyzx, r1.wwww
    r3.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 33: dp3 r1.w, r3.xyzx, r0.yzwy
    r1.w = (dot((r3.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 34: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: sample_b_indexable(texture2d)(float,float,float,float) r4.xyz, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r4.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 37: mul r5.xyz, cb0[1].xyzx, cb0[3].xxxx
    r5.xyz = ((source[1].xyzx)*(source[3].xxxx)).xyz;
    // 38: mul r4.xyz, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.xyzx)*(r5.xyzx)).xyz;
    // 39: mad r0.xyz, v7.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v7.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 40: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 41: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 42: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 43: dp3 r0.x, r0.xyzx, r3.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 44: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 46: mul r0.x, r0.x, cb0[3].y
    r0.x = ((r0.xxxx)*(source[3].yyyy)).x;
    // 47: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 48: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 50: mul r0.xyz, r4.xyzx, r0.xxxx
    r0.xyz = ((r4.xyzx)*(r0.xxxx)).xyz;
    // 51: mad r0.xyz, r2.wwww, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r2.xyzx)+(r0.xyzx)).xyz;
    // 52: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 53: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 54: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 55: mul o0.xyz, r0.xyzx, cb0[4].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[4].xyzx)).xyz;
    // 56: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 57: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 58: ret
    return output;
}

// source.character.static-map-native-1405.v1 / source program 40de4f033dc0f840b8bb4402216205f2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1405(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=float4(input.lightColor,1.f);
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.x, cb0[6].x, l(0.017453)
    r0.x = ((source[6].xxxx)*(float4(0.017453,0.017453,0.017453,0.017453))).x;
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
    // 9: dp3 r0.y, v5.xyzx, v5.xyzx
    r0.y = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).y;
    // 10: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 11: mul r0.yzw, r0.yyyy, v5.xxyz
    r0.yzw = ((r0.yyyy)*(v5.xxyz)).yzw;
    // 12: dp3 r1.x, v3.xyzx, v3.xyzx
    r1.x = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).x;
    // 13: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 14: mul r1.xyz, r1.xxxx, v3.xyzx
    r1.xyz = ((r1.xxxx)*(v3.xyzx)).xyz;
    // 15: dp3 r0.y, r1.xyzx, r0.yzwy
    r0.y = (dot((r1.xyzx).xyz,(r0.yzwy).xyz).xxxx).y;
    // 16: max r0.z, r1.z, l(0.000000)
    r0.z = (max(r1.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 17: mul r1.xyz, r0.zzzz, cb2[3].xyzx
    r1.xyz = ((r0.zzzz)*(passValues[3].xyzx)).xyz;
    // 18: mad r0.y, -r0.y, l(0.499000), l(0.500000)
    r0.y = ((-(r0.yyyy))*(float4(0.499000,0.499000,0.499000,0.499000))+(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 19: log r0.y, r0.y
    r0.y = (log2(r0.yyyy)).y;
    // 20: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 21: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 22: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 23: mul r0.y, r0.x, cb0[6].z
    r0.y = ((r0.xxxx)*(source[6].zzzz)).y;
    // 24: mov r2.x, v2.y
    r2.x = (v2.yyyy).x;
    // 25: mov r2.y, cb0[4].z
    r2.y = (source[4].zzzz).y;
    // 26: add r0.zw, -r2.xxxy, l(0.000000, 0.000000, 1.000000, 1.000000)
    r0.zw = ((-(r2.xxxy))+(float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 27: ge r1.w, v2.y, cb0[4].z
    r1.w = (asfloat((uint4)((v2.yyyy)>=(source[4].zzzz)) * 0xffffffffu)).w;
    // 28: movc r0.z, r1.w, r0.z, v2.y
    r0.z = ((asuint(r1.wwww) != 0u) ? (r0.zzzz) : (v2.yyyy)).z;
    // 29: movc r0.w, r1.w, r0.w, cb0[4].z
    r0.w = ((asuint(r1.wwww) != 0u) ? (r0.wwww) : (source[4].zzzz)).w;
    // 30: movc_sat r1.w, r1.w, cb0[4].w, cb0[5].x
    r1.w = (saturate((asuint(r1.wwww) != 0u) ? (source[4].wwww) : (source[5].xxxx))).w;
    // 31: div r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)/(r0.wwww)).z;
    // 32: add r0.w, -r1.w, l(1.000000)
    r0.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: mad r0.z, r0.z, r0.w, r1.w
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.wwww)).z;
    // 34: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 35: mul r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)*(r0.zzzz)).z;
    // 36: mul_sat r0.z, r0.z, cb0[5].y
    r0.z = (saturate((r0.zzzz)*(source[5].yyyy))).z;
    // 37: mad r0.w, -cb0[6].z, r0.x, r0.z
    r0.w = ((-(source[6].zzzz))*(r0.xxxx)+(r0.zzzz)).w;
    // 38: mad r0.y, cb0[6].z, r0.w, r0.y
    r0.y = ((source[6].zzzz)*(r0.wwww)+(r0.yyyy)).y;
    // 39: mul r2.xyz, cb0[2].xyzx, cb0[2].wwww
    r2.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 40: mul r2.xyz, r0.zzzz, r2.xyzx
    r2.xyz = ((r0.zzzz)*(r2.xyzx)).xyz;
    // 41: mul r3.xyz, cb0[3].xyzx, cb0[3].wwww
    r3.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 42: mad r3.xyz, r3.xyzx, r0.yyyy, -r2.xyzx
    r3.xyz = ((r3.xyzx)*(r0.yyyy)+(-(r2.xyzx))).xyz;
    // 43: mad r0.xyw, r0.xxxx, r3.xyxz, r2.xyxz
    r0.xyw = ((r0.xxxx)*(r3.xyxz)+(r2.xyxz)).xyw;
    // 44: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 45: mul r3.xyz, r2.xyzx, cb0[1].xyzx
    r3.xyz = ((r2.xyzx)*(source[1].xyzx)).xyz;
    // 46: mad r2.xyz, -r2.xyzx, cb0[1].xyzx, r2.xyzx
    r2.xyz = ((-(r2.xyzx))*(source[1].xyzx)+(r2.xyzx)).xyz;
    // 47: mad r2.xyz, r2.wwww, r2.xyzx, r3.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r3.xyzx)).xyz;
    // 48: add r0.xyw, r0.xyxw, -r2.xyxz
    r0.xyw = ((r0.xyxw)+(-(r2.xyxz))).xyw;
    // 49: add r1.w, -r0.z, l(1.000000)
    r1.w = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 50: mad_sat r0.z, -r1.w, r2.w, r0.z
    r0.z = (saturate((-(r1.wwww))*(r2.wwww)+(r0.zzzz))).z;
    // 51: mad r0.xyz, r0.zzzz, r0.xywx, r2.xyzx
    r0.xyz = ((r0.zzzz)*(r0.xywx)+(r2.xyzx)).xyz;
    // 52: mul r0.xyz, r0.xyzx, cb0[6].wwww
    r0.xyz = ((r0.xyzx)*(source[6].wwww)).xyz;
    // 53: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 54: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 55: mov o0.w, cb0[0].x
    output.targets[0].w = (source[0].xxxx).w;
    // 56: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 57: ret
    return output;
}

// source.character.static-map-native-1406.v1 / source program ede6f867c5cff246ad5e420f2c8bbd76
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1406(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=float4(input.lightColor,1.f);
    source[13].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].y
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].yyyy)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: mul r0.xyzw, v6.yyyy, cb0[9].xyzw
    r0.xyzw = ((v6.yyyy)*(source[9].xyzw)).xyzw;
    // 4: mad r0.xyzw, cb0[8].xyzw, v6.xxxx, r0.xyzw
    r0.xyzw = ((source[8].xyzw)*(v6.xxxx)+(r0.xyzw)).xyzw;
    // 5: mad r0.xyzw, cb0[10].xyzw, v6.zzzz, r0.xyzw
    r0.xyzw = ((source[10].xyzw)*(v6.zzzz)+(r0.xyzw)).xyzw;
    // 6: mad r0.xyzw, cb0[11].xyzw, v6.wwww, r0.xyzw
    r0.xyzw = ((source[11].xyzw)*(v6.wwww)+(r0.xyzw)).xyzw;
    // 7: div r0.xy, r0.xyxx, r0.wwww
    r0.xy = ((r0.xyxx)/(r0.wwww)).xy;
    // 8: sample_indexable(texture2d)(float,float,float,float) r1.x, r0.xyxx, t3.xyzw, s4
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 9: mov r2.xw, l(0,0,0,0)
    r2.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 10: mov r2.yz, cb0[12].wwzw
    r2.yz = (source[12].wwzw).yz;
    // 11: add r2.xyzw, r0.xyxy, r2.xyzw
    r2.xyzw = ((r0.xyxy)+(r2.xyzw)).xyzw;
    // 12: sample_indexable(texture2d)(float,float,float,float) r1.y, r2.xyxx, t3.yxzw, s4
    r1.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 13: sample_indexable(texture2d)(float,float,float,float) r1.z, r2.zwzz, t3.yzxw, s4
    r1.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 14: add r2.xy, r0.xyxx, cb0[12].zwzz
    r2.xy = ((r0.xyxx)+(source[12].zwzz)).xy;
    // 15: sample_indexable(texture2d)(float,float,float,float) r1.w, r2.xyxx, t3.yzwx, s4
    r1.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 16: lt r1.xyzw, r0.zzzz, r1.xyzw
    r1.xyzw = (asfloat((uint4)((r0.zzzz)<(r1.xyzw)) * 0xffffffffu)).xyzw;
    // 17: and r2.xyzw, r1.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r2.xyzw = (asfloat(asuint(r1.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 18: mul r0.xy, r0.xyxx, cb0[12].xyxx
    r0.xy = ((r0.xyxx)*(source[12].xyxx)).xy;
    // 19: frc r0.xy, r0.xyxx
    r0.xy = (frac(r0.xyxx)).xy;
    // 20: movc r0.zw, r1.xxxy, l(0,0,-1.000000,-1.000000), l(0,0,-0.000000,-0.000000)
    r0.zw = ((asuint(r1.xxxy) != 0u) ? (float4(asfloat(0u),asfloat(0u),-1.000000,-1.000000)) : (float4(asfloat(0u),asfloat(0u),-0.000000,-0.000000))).zw;
    // 21: add r0.zw, r0.zzzw, r2.zzzw
    r0.zw = ((r0.zzzw)+(r2.zzzw)).zw;
    // 22: mad r0.xz, r0.xxxx, r0.zzwz, r2.xxyx
    r0.xz = ((r0.xxxx)*(r0.zzwz)+(r2.xxyx)).xz;
    // 23: add r0.z, -r0.x, r0.z
    r0.z = ((-(r0.xxxx))+(r0.zzzz)).z;
    // 24: mad r0.x, r0.y, r0.z, r0.x
    r0.x = ((r0.yyyy)*(r0.zzzz)+(r0.xxxx)).x;
    // 25: mul r0.xyz, r0.xxxx, cb0[13].xxxx
    r0.xyz = ((r0.xxxx)*(source[13].xxxx)).xyz;
    // 26: else
    } else {
    // 27: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 28: endif
    }
    // 29: add r0.w, -v0.w, cb0[6].w
    r0.w = ((-(v0.wwww))+(source[6].wwww)).w;
    // 30: add r0.w, r0.w, cb0[6].z
    r0.w = ((r0.wwww)+(source[6].zzzz)).w;
    // 31: add_sat r0.w, r0.w, l(1.000000)
    r0.w = (saturate((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000)))).w;
    // 32: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 33: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 34: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 35: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 36: endif
    }
    // 37: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 38: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 39: dp3 r1.y, v3.xyzx, v3.xyzx
    r1.y = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).y;
    // 40: rsq r1.y, r1.y
    r1.y = (rsqrt(r1.yyyy)).y;
    // 41: mul r1.yzw, r1.yyyy, v3.xxyz
    r1.yzw = ((r1.yyyy)*(v3.xxyz)).yzw;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v2.xyxx, t1.xyzw, s1, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 43: mul r3.xyz, cb0[1].xyzx, cb0[4].wwww
    r3.xyz = ((source[1].xyzx)*(source[4].wwww)).xyz;
    // 44: mul r4.xyz, r2.xyzx, r3.xyzx
    r4.xyz = ((r2.xyzx)*(r3.xyzx)).xyz;
    // 45: mul r5.xyz, cb0[2].xyzx, cb0[5].xxxx
    r5.xyz = ((source[2].xyzx)*(source[5].xxxx)).xyz;
    // 46: mul r6.xy, v2.xyxx, cb0[4].zzzz
    r6.xy = ((v2.xyxx)*(source[4].zzzz)).xy;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t2.xyzw, s2, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 48: mul r7.xyz, r5.xyzx, r6.xyzx
    r7.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 49: dp3 r3.w, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r3.w = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 50: mad r5.xyz, -r5.xyzx, r6.xyzx, r3.wwww
    r5.xyz = ((-(r5.xyzx))*(r6.xyzx)+(r3.wwww)).xyz;
    // 51: mad r5.xyz, cb0[5].zzzz, r5.xyzx, r7.xyzx
    r5.xyz = ((source[5].zzzz)*(r5.xyzx)+(r7.xyzx)).xyz;
    // 52: max r3.w, cb0[4].y, l(0.000000)
    r3.w = (max(source[4].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 53: min r3.w, r3.w, l(0.990000)
    r3.w = (min(r3.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 54: add r4.w, -r3.w, l(1.000000)
    r4.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 55: div r4.w, l(1.000000, 1.000000, 1.000000, 1.000000), r4.w
    r4.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r4.wwww)).w;
    // 56: add r5.w, -v0.x, l(1.000000)
    r5.w = ((-(v0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 57: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 58: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 59: dp2 r7.z, r7.xyxx, r7.xyxx
    r7.z = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).z;
    // 60: add r7.z, -r7.z, l(1.000000)
    r7.z = ((-(r7.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 61: max r7.z, r7.z, l(0.000000)
    r7.z = (max(r7.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).z;
    // 62: sqrt r7.z, r7.z
    r7.z = (sqrt(r7.zzzz)).z;
    // 63: add r8.z, r7.z, l(0.000010)
    r8.z = ((r7.zzzz)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 64: mul r7.xy, r7.xyxx, cb0[4].xxxx
    r7.xy = ((r7.xyxx)*(source[4].xxxx)).xy;
    // 65: mul r8.xy, r7.xyxx, v0.wwww
    r8.xy = ((r7.xyxx)*(v0.wwww)).xy;
    // 66: dp3 r7.x, r8.xyzx, r8.xyzx
    r7.x = (dot((r8.xyzx).xyz,(r8.xyzx).xyz).xxxx).x;
    // 67: sqrt r7.x, r7.x
    r7.x = (sqrt(r7.xxxx)).x;
    // 68: div r7.xyz, r8.xyzx, r7.xxxx
    r7.xyz = ((r8.xyzx)/(r7.xxxx)).xyz;
    // 69: mul r7.w, r7.z, r7.z
    r7.w = ((r7.zzzz)*(r7.zzzz)).w;
    // 70: mul_sat r2.w, r2.w, r7.w
    r2.w = (saturate((r2.wwww)*(r7.wwww))).w;
    // 71: add r2.w, -r2.w, l(1.000000)
    r2.w = ((-(r2.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 72: mul r6.w, r6.w, r6.w
    r6.w = ((r6.wwww)*(r6.wwww)).w;
    // 73: mul r2.w, r2.w, r6.w
    r2.w = ((r2.wwww)*(r6.wwww)).w;
    // 74: mul r6.w, r2.w, r3.w
    r6.w = ((r2.wwww)*(r3.wwww)).w;
    // 75: mad r5.w, r5.w, r6.w, r5.w
    r5.w = ((r5.wwww)*(r6.wwww)+(r5.wwww)).w;
    // 76: add r3.w, -r3.w, r5.w
    r3.w = ((-(r3.wwww))+(r5.wwww)).w;
    // 77: mul r6.w, r3.w, r4.w
    r6.w = ((r3.wwww)*(r4.wwww)).w;
    // 78: mad r3.w, -r4.w, r3.w, r5.w
    r3.w = ((-(r4.wwww))*(r3.wwww)+(r5.wwww)).w;
    // 79: mad_sat r2.w, r2.w, r3.w, r6.w
    r2.w = (saturate((r2.wwww)*(r3.wwww)+(r6.wwww))).w;
    // 80: mad r2.xyz, -r2.xyzx, r3.xyzx, r5.xyzx
    r2.xyz = ((-(r2.xyzx))*(r3.xyzx)+(r5.xyzx)).xyz;
    // 81: mad r2.xyz, r2.wwww, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.wwww)*(r2.xyzx)+(r4.xyzx)).xyz;
    // 82: mul r3.x, r2.w, l(0.650000)
    r3.x = ((r2.wwww)*(float4(0.650000,0.650000,0.650000,0.650000))).x;
    // 83: add r3.yzw, -r7.xxyz, l(0.000000, 0.000000, 0.000000, 1.000000)
    r3.yzw = ((-(r7.xxyz))+(float4(0.000000,0.000000,0.000000,1.000000))).yzw;
    // 84: mad r3.xyz, r3.xxxx, r3.yzwy, r7.xyzx
    r3.xyz = ((r3.xxxx)*(r3.yzwy)+(r7.xyzx)).xyz;
    // 85: dp3 r3.w, r3.xyzx, r1.yzwy
    r3.w = (dot((r3.xyzx).xyz,(r1.yzwy).xyz).xxxx).w;
    // 86: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 87: min r4.x, r3.w, l(1.000000)
    r4.x = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r4.yzw, v2.xyxx, t4.wxyz, s3, l(0.000000)
    r4.yzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 89: mul r5.xyz, cb0[3].xyzx, cb0[5].wwww
    r5.xyz = ((source[3].xyzx)*(source[5].wwww)).xyz;
    // 90: mul r4.yzw, r4.yyzw, r5.xxyz
    r4.yzw = ((r4.yyzw)*(r5.xxyz)).yzw;
    // 91: mad r5.xyz, cb0[6].xxxx, r6.xyzx, -r4.yzwy
    r5.xyz = ((source[6].xxxx)*(r6.xyzx)+(-(r4.yzwy))).xyz;
    // 92: mad r4.yzw, r2.wwww, r5.xxyz, r4.yyzw
    r4.yzw = ((r2.wwww)*(r5.xxyz)+(r4.yyzw)).yzw;
    // 93: mad r1.xyz, v5.xyzx, r1.xxxx, r1.yzwy
    r1.xyz = ((v5.xyzx)*(r1.xxxx)+(r1.yzwy)).xyz;
    // 94: dp3 r1.w, r1.xyzx, r1.xyzx
    r1.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 95: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 96: div r1.xyz, r1.xyzx, r1.wwww
    r1.xyz = ((r1.xyzx)/(r1.wwww)).xyz;
    // 97: dp3 r1.x, r1.xyzx, r3.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 98: lt r1.y, |r1.x|, l(0.000001)
    r1.y = (asfloat((uint4)((abs(r1.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 99: log r1.x, |r1.x|
    r1.x = (log2(abs(r1.xxxx))).x;
    // 100: mul r1.x, r1.x, cb0[6].y
    r1.x = ((r1.xxxx)*(source[6].yyyy)).x;
    // 101: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 102: min r1.x, r1.x, l(1.000000)
    r1.x = (min(r1.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 104: mul r1.xyz, r4.yzwy, r1.xxxx
    r1.xyz = ((r4.yzwy)*(r1.xxxx)).xyz;
    // 105: mad r1.xyz, r4.xxxx, r2.xyzx, r1.xyzx
    r1.xyz = ((r4.xxxx)*(r2.xyzx)+(r1.xyzx)).xyz;
    // 106: mul r2.xyz, r3.wwww, cb2[3].xyzx
    r2.xyz = ((r3.wwww)*(passValues[3].xyzx)).xyz;
    // 107: mad r1.xyz, r1.xyzx, cb2[3].wwww, r2.xyzx
    r1.xyz = ((r1.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 108: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 109: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 110: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 111: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 112: ret
    return output;
}

// source.character.static-map-native-1407.v1 / source program e9aba0e747c157419724e12633e4e3b2
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1407(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[3];
    source[5]=g_SourceCharacterLightConstants[4];
    source[6]=g_SourceCharacterLightConstants[5];
    source[7]=g_SourceCharacterLightConstants[8];
    source[8]=g_SourceCharacterLightConstants[9];
    source[9]=float4(input.lightColor,1.f);
    source[10].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[10].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[10].xxxx)) * 0xffffffffu)).x;
    // 2: if_nz r0.x
    if ((asuint(r0.xxxx)).x != 0u) {
    // 3: div r0.xy, v6.xyxx, v6.wwww
    r0.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 4: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 5: sample_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1
    r0.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 6: mul r0.xyz, r0.xyzx, r0.xyzx
    r0.xyz = ((r0.xyzx)*(r0.xyzx)).xyz;
    // 7: else
    } else {
    // 8: mov r0.xyz, l(1.000000,1.000000,1.000000,0)
    r0.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 9: endif
    }
    // 10: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzwx).w;
    // 11: div r1.xy, v6.xyxx, v6.wwww
    r1.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 12: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 13: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t2.xyzw, s0, l(0.000000)
    r1.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 14: min r1.x, r1.x, l(0.999000)
    r1.x = (min(r1.xxxx,float4(0.999000,0.999000,0.999000,0.999000))).x;
    // 15: mad r1.y, r1.x, cb2[1].x, cb2[1].y
    r1.y = ((r1.xxxx)*(passValues[1].xxxx)+(passValues[1].yyyy)).y;
    // 16: mad r1.x, r1.x, cb2[1].z, -cb2[1].w
    r1.x = ((r1.xxxx)*(passValues[1].zzzz)+(-(passValues[1].wwww))).x;
    // 17: div r1.x, l(1.000000, 1.000000, 1.000000, 1.000000), r1.x
    r1.x = ((float4(1.000000,1.000000,1.000000,1.000000))/(r1.xxxx)).x;
    // 18: add r1.x, r1.x, r1.y
    r1.x = ((r1.xxxx)+(r1.yyyy)).x;
    // 19: add r1.y, -cb0[8].x, l(1.000000)
    r1.y = ((-(source[8].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 20: add r1.x, r1.x, -v6.w
    r1.x = ((r1.xxxx)+(-(v6.wwww))).x;
    // 21: max r1.y, r1.y, l(0.001000)
    r1.y = (max(r1.yyyy,float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 22: div_sat r1.x, r1.x, r1.y
    r1.x = (saturate((r1.xxxx)/(r1.yyyy))).x;
    // 23: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 24: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 25: lt r1.x, r0.w, l(0.003000)
    r1.x = (asfloat((uint4)((r0.wwww)<(float4(0.003000,0.003000,0.003000,0.003000))) * 0xffffffffu)).x;
    // 26: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 27: discard_nz l(-1)
    if ((uint4(4294967295u,4294967295u,4294967295u,4294967295u)).x != 0u) { output.discarded = true; return output; }
    // 28: endif
    }
    // 29: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 30: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 31: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 32: dp3 r1.w, v3.xyzx, v3.xyzx
    r1.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 33: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 34: mul r2.xyz, r1.wwww, v3.xyzx
    r2.xyz = ((r1.wwww)*(v3.xyzx)).xyz;
    // 35: mad r1.xyz, r1.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r1.xyzx
    r1.xyz = ((r1.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r1.xyzx))).xyz;
    // 36: add r3.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r3.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 37: dp2 r4.x, cb0[2].xyxx, r3.xyxx
    r4.x = (dot((source[2].xyxx).xy,(r3.xyxx).xy).xxxx).x;
    // 38: dp2 r4.y, cb0[3].xyxx, r3.xyxx
    r4.y = (dot((source[3].xyxx).xy,(r3.xyxx).xy).xxxx).y;
    // 39: add r3.xy, r4.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r3.xy = ((r4.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 40: mul r3.xy, r3.xyxx, cb0[4].xyxx
    r3.xy = ((r3.xyxx)*(source[4].xyxx)).xy;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t0.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r3.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 42: dp3 r1.w, r3.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r3.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 43: add r4.xyz, -r3.xyzx, r1.wwww
    r4.xyz = ((-(r3.xyzx))+(r1.wwww)).xyz;
    // 44: mad r3.xyz, cb0[7].xxxx, r4.xyzx, r3.xyzx
    r3.xyz = ((source[7].xxxx)*(r4.xyzx)+(r3.xyzx)).xyz;
    // 45: mul r4.xyz, r3.xyzx, cb0[7].zzzz
    r4.xyz = ((r3.xyzx)*(source[7].zzzz)).xyz;
    // 46: mul r4.xyz, r4.xyzx, cb0[6].xyzx
    r4.xyz = ((r4.xyzx)*(source[6].xyzx)).xyz;
    // 47: mad r4.xyz, r4.xyzx, cb2[4].wwww, cb2[4].xyzx
    r4.xyz = ((r4.xyzx)*(passValues[4].wwww)+(passValues[4].xyzx)).xyz;
    // 48: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 49: mul r3.xyz, r3.xyzx, cb0[7].yyyy
    r3.xyz = ((r3.xyzx)*(source[7].yyyy)).xyz;
    // 50: mul r3.xyz, r3.xyzx, cb0[5].xyzx
    r3.xyz = ((r3.xyzx)*(source[5].xyzx)).xyz;
    // 51: mul r3.xyz, r5.xyzx, r3.xyzx
    r3.xyz = ((r5.xyzx)*(r3.xyzx)).xyz;
    // 52: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 53: mov_sat r1.w, r2.z
    r1.w = (saturate(r2.zzzz)).w;
    // 54: lt r2.w, r1.w, l(0.000001)
    r2.w = (asfloat((uint4)((r1.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 55: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 56: dp3_sat r1.x, r1.xyzx, r2.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r2.xyzx).xyz).xxxx)).x;
    // 57: lt r1.y, r1.x, l(0.000001)
    r1.y = (asfloat((uint4)((r1.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 58: log r1.x, r1.x
    r1.x = (log2(r1.xxxx)).x;
    // 59: mul r1.x, r1.x, cb0[7].w
    r1.x = ((r1.xxxx)*(source[7].wwww)).x;
    // 60: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 61: movc r1.x, r1.y, l(0), r1.x
    r1.x = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).x;
    // 62: mul r1.xyz, r4.xyzx, r1.xxxx
    r1.xyz = ((r4.xyzx)*(r1.xxxx)).xyz;
    // 63: mad r1.xyz, r3.xyzx, r1.wwww, r1.xyzx
    r1.xyz = ((r3.xyzx)*(r1.wwww)+(r1.xyzx)).xyz;
    // 64: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 65: mul o0.xyz, r0.xyzx, cb0[9].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[9].xyzx)).xyz;
    // 66: mov o0.w, r0.w
    output.targets[0].w = (r0.wwww).w;
    // 67: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 68: ret
    return output;
}

// source.character.static-map-native-1408.v1 / source program f3453c86658eb6478d0b461aef37ae42
