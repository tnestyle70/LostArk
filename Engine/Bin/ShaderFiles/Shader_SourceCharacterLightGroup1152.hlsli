SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1152(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=float4(input.lightColor,1.f);
    source[6].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f;
    // 1: ne r0.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[6].x
    r0.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[6].xxxx)) * 0xffffffffu)).x;
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
    // 10: dp3 r0.w, v7.xyzx, v7.xyzx
    r0.w = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).w;
    // 11: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 12: dp3 r1.x, v5.xyzx, v5.xyzx
    r1.x = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).x;
    // 13: rsq r1.x, r1.x
    r1.x = (rsqrt(r1.xxxx)).x;
    // 14: mul r1.xyz, r1.xxxx, v5.xyzx
    r1.xyz = ((r1.xxxx)*(v5.xyzx)).xyz;
    // 15: sample_b_indexable(texture2d)(float,float,float,float) r2.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 16: add r1.w, r2.w, l(-0.333300)
    r1.w = ((r2.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 17: lt r1.w, r1.w, l(0.000000)
    r1.w = (asfloat((uint4)((r1.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 18: discard_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) { output.discarded = true; return output; }
    // 19: dp3 r1.w, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, v4.xyxx, t2.xzwy, s3, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xzwy).xyz;
    // 21: mul r2.w, r3.z, cb0[3].z
    r2.w = ((r3.zzzz)*(source[3].zzzz)).w;
    // 22: add r4.xyz, -r2.xyzx, r1.wwww
    r4.xyz = ((-(r2.xyzx))+(r1.wwww)).xyz;
    // 23: mad r2.xyz, r2.wwww, r4.xyzx, r2.xyzx
    r2.xyz = ((r2.wwww)*(r4.xyzx)+(r2.xyzx)).xyz;
    // 24: mad r4.xyz, cb0[3].wwww, cb0[0].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r4.xyz = ((source[3].wwww)*(source[0].xyzx)+(float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 25: mad r4.xyz, r3.zzzz, r4.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r4.xyz = ((r3.zzzz)*(r4.xyzx)+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 26: mul r2.xyz, r2.xyzx, r4.xyzx
    r2.xyz = ((r2.xyzx)*(r4.xyzx)).xyz;
    // 27: sample_b_indexable(texture2d)(float,float,float,float) r3.zw, v4.xyxx, t0.zwxy, s1, l(0.000000)
    r3.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 28: mad r3.zw, r3.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r3.zw = ((r3.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 29: dp2 r1.w, r3.zwzz, r3.zwzz
    r1.w = (dot((r3.zwzz).xy,(r3.zwzz).xy).xxxx).w;
    // 30: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 31: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 32: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 33: add r4.z, r1.w, l(0.000010)
    r4.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 34: mul r4.xy, r3.zwzz, cb0[3].xxxx
    r4.xy = ((r3.zwzz)*(source[3].xxxx)).xy;
    // 35: dp3 r1.w, r4.xyzx, r4.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 36: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 37: div r4.xyz, r4.xyzx, r1.wwww
    r4.xyz = ((r4.xyzx)/(r1.wwww)).xyz;
    // 38: dp3 r1.w, r4.xyzx, r1.xyzx
    r1.w = (dot((r4.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 39: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 40: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: mul r5.xyz, r0.xyzx, r2.wwww
    r5.xyz = ((r0.xyzx)*(r2.wwww)).xyz;
    // 42: mul r6.xyz, cb0[2].xyzx, cb0[4].zzzz
    r6.xyz = ((source[2].xyzx)*(source[4].zzzz)).xyz;
    // 43: mul r3.yzw, r3.yyyy, r6.xxyz
    r3.yzw = ((r3.yyyy)*(r6.xxyz)).yzw;
    // 44: mad r6.xyz, r3.yzwy, r0.xyzx, -r5.xyzx
    r6.xyz = ((r3.yzwy)*(r0.xyzx)+(-(r5.xyzx))).xyz;
    // 45: mad r3.yzw, r3.yyzw, r6.xxyz, r5.xxyz
    r3.yzw = ((r3.yyzw)*(r6.xxyz)+(r5.xxyz)).yzw;
    // 46: mul r5.xyz, r3.xxxx, cb0[1].xyzx
    r5.xyz = ((r3.xxxx)*(source[1].xyzx)).xyz;
    // 47: mul r5.xyz, r5.xyzx, cb0[4].xxxx
    r5.xyz = ((r5.xyzx)*(source[4].xxxx)).xyz;
    // 48: mad r1.xyz, v7.xyzx, r0.wwww, r1.xyzx
    r1.xyz = ((v7.xyzx)*(r0.wwww)+(r1.xyzx)).xyz;
    // 49: dp3 r0.w, r1.xyzx, r1.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).w;
    // 50: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 51: div r1.xyz, r1.xyzx, r0.wwww
    r1.xyz = ((r1.xyzx)/(r0.wwww)).xyz;
    // 52: dp3 r0.w, r1.xyzx, r4.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 53: lt r1.x, |r0.w|, l(0.000001)
    r1.x = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 54: log r0.w, |r0.w|
    r0.w = (log2(abs(r0.wwww))).w;
    // 55: mul r0.w, r0.w, cb0[4].y
    r0.w = ((r0.wwww)*(source[4].yyyy)).w;
    // 56: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 57: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 58: movc r0.w, r1.x, l(0), r0.w
    r0.w = ((asuint(r1.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 59: mul r1.xyz, r5.xyzx, r0.wwww
    r1.xyz = ((r5.xyzx)*(r0.wwww)).xyz;
    // 60: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 61: max r0.xyz, r0.xyzx, l(0.000000, 0.000000, 0.000000, 0.000000)
    r0.xyz = (max(r0.xyzx,float4(0.000000,0.000000,0.000000,0.000000))).xyz;
    // 62: min r0.xyz, r0.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(2.000000,2.000000,2.000000,0.000000))).xyz;
    // 63: mad r0.xyz, r2.xyzx, r3.yzwy, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 64: mul r1.xyz, r1.wwww, cb2[3].xyzx
    r1.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 66: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 67: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 68: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 69: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 70: ret
    return output;
}

// source.character.static-map-native-1153.v1 / source program 91f8da7d3428ca4daf760ce068b82b25
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1153(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[4];
    source[3]=g_SourceCharacterLightConstants[5];
    source[4]=g_SourceCharacterLightConstants[6];
    source[5]=g_SourceCharacterLightConstants[7];
    source[6]=g_SourceCharacterLightConstants[8];
    source[7]=float4(input.lightColor,1.f);
    source[13].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f;
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

// source.character.static-map-native-1154.v1 / source program 0e1fdb81dc39b54e9d66f69e1f59bef5
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1154(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=g_SourceCharacterLightConstants[11];
    source[12]=g_SourceCharacterLightConstants[12];
    source[13]=float4(input.lightColor,1.f);
    source[14].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
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
    // 16: mul r5.xy, v4.xyxx, cb0[2].xyxx
    r5.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t0.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 18: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r7.xy, r5.xyxx, cb0[7].xxxx
    r7.xy = ((r5.xyxx)*(source[7].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r7.xy, r7.xyxx, cb0[7].yyyy
    r7.xy = ((r7.xyxx)*(source[7].yyyy)).xy;
    // 28: mad r5.zw, cb0[6].wwww, r5.zzzw, r7.xxxy
    r5.zw = ((source[6].wwww)*(r5.zzzw)+(r7.xxxy)).zw;
    // 29: mul r6.xy, r5.zwzz, v2.wwww
    r6.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 33: max r1.w, cb0[7].z, l(0.000000)
    r1.w = (max(source[7].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 35: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 36: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 37: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 38: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 39: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 40: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 41: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 42: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 43: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 44: mad r0.x, r0.x, l(0.500000), cb0[8].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].yyyy)).x;
    // 45: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 46: mul r0.y, r6.z, r6.z
    r0.y = ((r6.zzzz)*(r6.zzzz)).y;
    // 47: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 48: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 49: mul r1.xy, v4.xyxx, cb0[8].zzzz
    r1.xy = ((v4.xyxx)*(source[8].zzzz)).xy;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, r1.xyxx, t3.xyzw, s4, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
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
    // 56: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 57: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 58: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 59: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 60: add r1.xyz, -r6.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r1.xyz = ((-(r6.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 61: mad r1.xyz, r0.yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 62: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 63: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 64: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 65: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 66: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 67: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 68: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 69: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 70: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 71: else
    } else {
    // 72: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 73: endif
    }
    // 74: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 75: dp3 r0.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 76: add r9.xyz, -r7.xyzx, r0.yyyy
    r9.xyz = ((-(r7.xyzx))+(r0.yyyy)).xyz;
    // 77: mad r7.xyz, cb0[9].xxxx, r9.xyzx, r7.xyzx
    r7.xyz = ((source[9].xxxx)*(r9.xyzx)+(r7.xyzx)).xyz;
    // 78: mul r9.xyz, cb0[4].xyzx, cb0[9].yyyy
    r9.xyz = ((source[4].xyzx)*(source[9].yyyy)).xyz;
    // 79: mul r10.xyz, r7.xyzx, r9.xyzx
    r10.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 80: mul r11.xyz, cb0[5].xyzx, cb0[9].zzzz
    r11.xyz = ((source[5].xyzx)*(source[9].zzzz)).xyz;
    // 81: mul r12.xyz, r8.xyzx, r11.xyzx
    r12.xyz = ((r8.xyzx)*(r11.xyzx)).xyz;
    // 82: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 83: mad r8.xyz, -r11.xyzx, r8.xyzx, r0.yyyy
    r8.xyz = ((-(r11.xyzx))*(r8.xyzx)+(r0.yyyy)).xyz;
    // 84: mad r8.xyz, cb0[10].xxxx, r8.xyzx, r12.xyzx
    r8.xyz = ((source[10].xxxx)*(r8.xyzx)+(r12.xyzx)).xyz;
    // 85: mad r7.xyz, -r7.xyzx, r9.xyzx, r8.xyzx
    r7.xyz = ((-(r7.xyzx))*(r9.xyzx)+(r8.xyzx)).xyz;
    // 86: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 87: mul r7.xyz, r0.xyzx, cb0[10].yyyy
    r7.xyz = ((r0.xyzx)*(source[10].yyyy)).xyz;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t4.yzxw, s5, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 89: mul r1.w, r5.y, cb0[10].w
    r1.w = ((r5.yyyy)*(source[10].wwww)).w;
    // 90: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 91: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 92: mul r1.w, r1.w, cb0[11].x
    r1.w = ((r1.wwww)*(source[11].xxxx)).w;
    // 93: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 94: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 95: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: mad r0.xyz, cb0[10].zzzz, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[10].zzzz)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 97: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 98: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 99: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 100: mov_sat r2.w, cb0[11].y
    r2.w = (saturate(source[11].yyyy)).w;
    // 101: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 102: mul r3.w, r5.x, cb0[11].w
    r3.w = ((r5.xxxx)*(source[11].wwww)).w;
    // 103: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 104: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 105: mul r3.w, r3.w, cb0[12].x
    r3.w = ((r3.wwww)*(source[12].xxxx)).w;
    // 106: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 107: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 108: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 109: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 111: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 112: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 113: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 114: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 115: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 116: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 117: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 119: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 120: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 123: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 126: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 127: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 128: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 129: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 130: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 131: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 132: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 133: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 134: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 136: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 138: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 139: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 140: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 141: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 142: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 143: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 144: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 146: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 147: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 148: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 149: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 150: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 152: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 153: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 154: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 155: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 157: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 158: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 159: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 160: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 161: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 162: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 163: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 164: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 165: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 166: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 167: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 168: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 169: ret
    return output;
}

// source.character.static-map-native-1155.v1 / source program 86530ace727d8842a59dc3e964706a58
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1155(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=g_SourceCharacterLightConstants[11];
    source[12]=g_SourceCharacterLightConstants[12];
    source[13]=float4(input.lightColor,1.f);
    source[14].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
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
    // 16: mul r5.xy, v4.xyxx, cb0[2].xyxx
    r5.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t0.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 18: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r7.xy, r5.xyxx, cb0[7].xxxx
    r7.xy = ((r5.xyxx)*(source[7].xxxx)).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r7.xyxx, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 26: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 27: mul r7.xy, r7.xyxx, cb0[7].yyyy
    r7.xy = ((r7.xyxx)*(source[7].yyyy)).xy;
    // 28: mad r5.zw, cb0[6].wwww, r5.zzzw, r7.xxxy
    r5.zw = ((source[6].wwww)*(r5.zzzw)+(r7.xxxy)).zw;
    // 29: mul r6.xy, r5.zwzz, v2.wwww
    r6.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 30: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 31: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 32: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 33: mul r5.zw, v4.xxxy, cb0[7].zzzz
    r5.zw = ((v4.xxxy)*(source[7].zzzz)).zw;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r5.zwzz, t2.xyzw, s3, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 35: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 36: dp2 r1.w, r7.xyxx, r7.xyxx
    r1.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 37: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 38: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 39: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 40: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 41: mul r8.xy, r7.xyxx, cb0[7].wwww
    r8.xy = ((r7.xyxx)*(source[7].wwww)).xy;
    // 42: max r1.w, cb0[8].x, l(0.000000)
    r1.w = (max(source[8].xxxx,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 43: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 44: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 46: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 47: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 48: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 49: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 50: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 51: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 52: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 53: mad r0.x, r0.x, l(0.500000), cb0[8].w
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].wwww)).x;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t3.xyzw, s4, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r0.y, r6.z, r6.z
    r0.y = ((r6.zzzz)*(r6.zzzz)).y;
    // 56: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 57: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 58: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.zwzz, t4.xyzw, s5, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 59: mul r0.z, r9.w, r9.w
    r0.z = ((r9.wwww)*(r9.wwww)).z;
    // 60: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 61: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 62: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 63: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 64: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 65: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 66: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 67: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 68: add r1.xyz, -r6.xyzx, r8.xyzx
    r1.xyz = ((-(r6.xyzx))+(r8.xyzx)).xyz;
    // 69: mad r1.xyz, r0.yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 70: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 71: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 72: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 73: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 74: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 75: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 76: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 77: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t6.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 78: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 79: else
    } else {
    // 80: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 81: endif
    }
    // 82: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 83: dp3 r0.y, r7.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r7.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 84: add r8.xyz, -r7.xyzx, r0.yyyy
    r8.xyz = ((-(r7.xyzx))+(r0.yyyy)).xyz;
    // 85: mad r7.xyz, cb0[9].yyyy, r8.xyzx, r7.xyzx
    r7.xyz = ((source[9].yyyy)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 86: mul r8.xyz, cb0[4].xyzx, cb0[9].zzzz
    r8.xyz = ((source[4].xyzx)*(source[9].zzzz)).xyz;
    // 87: mul r10.xyz, r7.xyzx, r8.xyzx
    r10.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 88: mul r11.xyz, cb0[5].xyzx, cb0[9].wwww
    r11.xyz = ((source[5].xyzx)*(source[9].wwww)).xyz;
    // 89: mul r12.xyz, r9.xyzx, r11.xyzx
    r12.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 90: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 91: mad r9.xyz, -r11.xyzx, r9.xyzx, r0.yyyy
    r9.xyz = ((-(r11.xyzx))*(r9.xyzx)+(r0.yyyy)).xyz;
    // 92: mad r9.xyz, cb0[10].yyyy, r9.xyzx, r12.xyzx
    r9.xyz = ((source[10].yyyy)*(r9.xyzx)+(r12.xyzx)).xyz;
    // 93: mad r7.xyz, -r7.xyzx, r8.xyzx, r9.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r9.xyzx)).xyz;
    // 94: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 95: mul r7.xyz, r0.xyzx, cb0[10].zzzz
    r7.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 96: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t5.yzxw, s6, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture5.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 97: mul r1.w, r5.y, cb0[11].x
    r1.w = ((r5.yyyy)*(source[11].xxxx)).w;
    // 98: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 99: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 100: mul r1.w, r1.w, cb0[11].y
    r1.w = ((r1.wwww)*(source[11].yyyy)).w;
    // 101: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 102: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 103: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 104: mad r0.xyz, cb0[10].wwww, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[10].wwww)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 105: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 106: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 107: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 108: mov_sat r2.w, cb0[11].z
    r2.w = (saturate(source[11].zzzz)).w;
    // 109: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 110: mul r3.w, r5.x, cb0[12].x
    r3.w = ((r5.xxxx)*(source[12].xxxx)).w;
    // 111: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 112: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 113: mul r3.w, r3.w, cb0[12].y
    r3.w = ((r3.wwww)*(source[12].yyyy)).w;
    // 114: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 115: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 116: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 117: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 119: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 120: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 121: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 122: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 123: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 125: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 127: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 128: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 130: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 131: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 132: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 134: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 135: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 136: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 137: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 138: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 139: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 140: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 141: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 142: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 143: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 144: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 145: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 146: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 147: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 148: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 149: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 150: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 151: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 152: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 153: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 154: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 155: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 156: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 157: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 158: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 159: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 160: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 161: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 162: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 163: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 164: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 165: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 166: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 167: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 168: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 169: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 170: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 171: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 172: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 173: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 174: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 175: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 176: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 177: ret
    return output;
}

SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1156(SOURCE_CHARACTER_NATIVE_INPUT input)
{return (SOURCE_CHARACTER_NATIVE_OUTPUT)0;}

// source.character.static-map-native-1157.v1 / source program 3952ec3773eef54387e69d74117f6556
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1157(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.00800000038,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[5]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(-0.00400000019,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=SourceCharacterAppend((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0))))),(sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0,0,0,0))))),1u);
    source[9]=g_SourceCharacterLightConstants[9];
    source[9].y=(g_SourceCharacterTime.xxxx).x;
    source[9].w=((source[63].xxxx*g_SourceCharacterTime.xxxx)).x;
    source[10]=g_SourceCharacterLightConstants[12];
    source[10].x=((sign(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))*frac(abs(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(0.0199999996,0,0,0)))))).x;
    source[11]=float4(input.lightColor,1.f);
    source[12].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
    // 1: add r0.xyz, v6.xyzx, cb0[0].xyzx
    r0.xyz = ((v6.xyzx)+(source[0].xyzx)).xyz;
    // 2: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 3: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 4: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 5: dp3 r0.w, v3.xyzx, v3.xyzx
    r0.w = (dot((v3.xyzx).xyz,(v3.xyzx).xyz).xxxx).w;
    // 6: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 7: mul r2.xyz, r0.wwww, v3.xyzx
    r2.xyz = ((r0.wwww)*(v3.xyzx)).xyz;
    // 8: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[12].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[12].xxxx)) * 0xffffffffu)).w;
    // 9: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 10: mul r3.xyz, v6.yyyy, cb1[1].xywx
    r3.xyz = ((v6.yyyy)*(projection[1].xywx)).xyz;
    // 11: mad r3.xyz, cb1[0].xywx, v6.xxxx, r3.xyzx
    r3.xyz = ((projection[0].xywx)*(v6.xxxx)+(r3.xyzx)).xyz;
    // 12: mad r3.xyz, cb1[2].xywx, v6.zzzz, r3.xyzx
    r3.xyz = ((projection[2].xywx)*(v6.zzzz)+(r3.xyzx)).xyz;
    // 13: mad r3.xyz, cb1[3].xywx, v6.wwww, r3.xyzx
    r3.xyz = ((projection[3].xywx)*(v6.wwww)+(r3.xyzx)).xyz;
    // 14: div r3.xy, r3.xyxx, r3.zzzz
    r3.xy = ((r3.xyxx)/(r3.zzzz)).xy;
    // 15: mad r3.xy, r3.xyxx, cb2[0].xyxx, cb2[0].wzww
    r3.xy = ((r3.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r3.xyz, r3.xyxx, t3.xyzw, s0
    r3.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 17: mul r3.xyz, r3.xyzx, r3.xyzx
    r3.xyz = ((r3.xyzx)*(r3.xyzx)).xyz;
    // 18: else
    } else {
    // 19: mov r3.xyz, l(1.000000,1.000000,1.000000,0)
    r3.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 20: endif
    }
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 22: mul r5.xyz, r4.xyzx, cb0[1].xyzx
    r5.xyz = ((r4.xyzx)*(source[1].xyzx)).xyz;
    // 23: mad r4.xyz, -r4.xyzx, cb0[1].xyzx, r4.xyzx
    r4.xyz = ((-(r4.xyzx))*(source[1].xyzx)+(r4.xyzx)).xyz;
    // 24: mad r4.xyz, r4.wwww, r4.xyzx, r5.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)+(r5.xyzx)).xyz;
    // 25: dp3 r0.w, cb0[2].xyzx, cb0[2].xyzx
    r0.w = (dot((source[2].xyzx).xyz,(source[2].xyzx).xyz).xxxx).w;
    // 26: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 27: div r5.xyz, cb0[2].xyzx, r0.wwww
    r5.xyz = ((source[2].xyzx)/(r0.wwww)).xyz;
    // 28: add r0.xyz, -r0.xyzx, cb0[0].xyzx
    r0.xyz = ((-(r0.xyzx))+(source[0].xyzx)).xyz;
    // 29: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 30: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 31: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 32: dp3 r0.x, r5.xyzx, r0.xyzx
    r0.x = (dot((r5.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 33: mov_sat r0.x, -r0.x
    r0.x = (saturate(-(r0.xxxx))).x;
    // 34: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 35: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 36: mul r0.x, r0.x, cb0[9].x
    r0.x = ((r0.xxxx)*(source[9].xxxx)).x;
    // 37: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 38: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 39: mov r5.xw, l(1.000000,0,0,2.000000)
    r5.xw = (float4(1.000000,asfloat(0u),asfloat(0u),2.000000)).xw;
    // 40: mov r5.yz, cb0[3].yyxy
    r5.yz = (source[3].yyxy).yz;
    // 41: mul r0.yz, r5.xxyx, v2.xxyx
    r0.yz = ((r5.xxyx)*(v2.xxyx)).yz;
    // 42: mad r5.xy, r5.zwzz, r0.yzyy, cb0[4].xyxx
    r5.xy = ((r5.zwzz)*(r0.yzyy)+(source[4].xyxx)).xy;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r6.xy, r5.xyxx, t1.xyzw, s2, l(0.000000)
    r6.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 44: mad r6.xy, r6.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r6.xy = ((r6.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: dp2 r0.w, r6.xyxx, r6.xyxx
    r0.w = (dot((r6.xyxx).xy,(r6.xyxx).xy).xxxx).w;
    // 46: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 47: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 48: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 49: add r6.z, r0.w, l(0.000010)
    r6.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 50: mul r7.xy, v2.xyxx, cb0[3].xyxx
    r7.xy = ((v2.xyxx)*(source[3].xyxx)).xy;
    // 51: mad r7.xy, r7.xyxx, l(-1.000000, 2.000000, 0.000000, 0.000000), cb0[5].xyxx
    r7.xy = ((r7.xyxx)*(float4(-1.000000,2.000000,0.000000,0.000000))+(source[5].xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r7.zw, r7.xyxx, t1.zwxy, s2, l(0.000000)
    r7.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 53: mad r8.xy, r7.zwzz, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r8.xy = ((r7.zwzz)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 54: dp2 r0.w, r8.xyxx, r8.xyxx
    r0.w = (dot((r8.xyxx).xy,(r8.xyxx).xy).xxxx).w;
    // 55: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 56: max r0.w, r0.w, l(0.000000)
    r0.w = (max(r0.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 57: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 58: add r8.z, r0.w, l(0.000010)
    r8.z = ((r0.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 59: add r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)+(r8.xyzx)).xyz;
    // 60: dp3 r0.w, r9.xyzx, r9.xyzx
    r0.w = (dot((r9.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 61: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 62: div r9.xyz, r9.xyzx, r0.wwww
    r9.xyz = ((r9.xyzx)/(r0.wwww)).xyz;
    // 63: dp3 r0.w, r1.xyzx, r9.xyzx
    r0.w = (dot((r1.xyzx).xyz,(r9.xyzx).xyz).xxxx).w;
    // 64: add r0.w, r0.w, l(1.000000)
    r0.w = ((r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 65: mad r0.w, -r0.w, l(0.500000), l(1.000000)
    r0.w = ((-(r0.wwww))*(float4(0.500000,0.500000,0.500000,0.500000))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 66: mul r1.xyz, r0.wwww, cb0[6].xyzx
    r1.xyz = ((r0.wwww)*(source[6].xyzx)).xyz;
    // 67: mul r1.xyz, r1.xyzx, cb0[6].wwww
    r1.xyz = ((r1.xyzx)*(source[6].wwww)).xyz;
    // 68: sample_b_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r7.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 69: sample_b_indexable(texture2d)(float,float,float,float) r9.xyz, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r9.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 70: mad r7.yzw, r9.xxyz, l(0.000000, 0.500000, 0.500000, 0.500000), r7.xxyz
    r7.yzw = ((r9.xxyz)*(float4(0.000000,0.500000,0.500000,0.500000))+(r7.xxyz)).yzw;
    // 71: mul r7.yzw, r7.yyzw, l(0.000000, 0.500000, 0.500000, 0.500000)
    r7.yzw = ((r7.yyzw)*(float4(0.000000,0.500000,0.500000,0.500000))).yzw;
    // 72: mad r0.yz, r5.zzwz, r0.yyzy, cb0[8].xxyx
    r0.yz = ((r5.zzwz)*(r0.yyzy)+(source[8].xxyx)).yz;
    // 73: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r0.yzyy, t2.wxyz, s3, l(0.000000)
    r0.yzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).wxyz).yzw;
    // 74: mad_sat r0.yzw, r0.yyzw, r7.yyzw, r7.yyzw
    r0.yzw = (saturate((r0.yyzw)*(r7.yyzw)+(r7.yyzw))).yzw;
    // 75: add r5.xyz, -r6.xyzx, r8.xyzx
    r5.xyz = ((-(r6.xyzx))+(r8.xyzx)).xyz;
    // 76: mad r5.xyz, r7.xxxx, r5.xyzx, r6.xyzx
    r5.xyz = ((r7.xxxx)*(r5.xyzx)+(r6.xyzx)).xyz;
    // 77: dp3 r1.w, r2.xyzx, r5.xyzx
    r1.w = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 78: add r1.w, r1.w, l(1.000000)
    r1.w = ((r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 79: mul r1.w, r1.w, l(0.500000)
    r1.w = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).w;
    // 80: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 81: mul r2.xyw, r0.yzyw, r1.wwww
    r2.xyw = ((r0.yzyw)*(r1.wwww)).xyw;
    // 82: add r0.z, cb0[7].w, l(-0.030000)
    r0.z = ((source[7].wwww)+(float4(-0.030000,-0.030000,-0.030000,-0.030000))).z;
    // 83: mad r2.xyw, r2.xyxw, r0.zzzz, l(0.030000, 0.030000, 0.000000, 0.030000)
    r2.xyw = ((r2.xyxw)*(r0.zzzz)+(float4(0.030000,0.030000,0.000000,0.030000))).xyw;
    // 84: mul r2.xyw, r2.xyxw, cb0[7].xyxz
    r2.xyw = ((r2.xyxw)*(source[7].xyxz)).xyw;
    // 85: mad r0.xzw, r0.xxxx, r1.xxyz, r2.xxyw
    r0.xzw = ((r0.xxxx)*(r1.xxyz)+(r2.xxyw)).xzw;
    // 86: mul r0.y, r0.y, cb0[10].y
    r0.y = ((r0.yyyy)*(source[10].yyyy)).y;
    // 87: add r1.x, -r4.w, l(0.200000)
    r1.x = ((-(r4.wwww))+(float4(0.200000,0.200000,0.200000,0.200000))).x;
    // 88: mul_sat r1.x, r1.x, l(5.000000)
    r1.x = (saturate((r1.xxxx)*(float4(5.000000,5.000000,5.000000,5.000000)))).x;
    // 89: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 90: add r0.xzw, -r4.xxyz, r0.xxzw
    r0.xzw = ((-(r4.xxyz))+(r0.xxzw)).xzw;
    // 91: mad r0.xyz, r0.yyyy, r0.xzwx, r4.xyzx
    r0.xyz = ((r0.yyyy)*(r0.xzwx)+(r4.xyzx)).xyz;
    // 92: mul r0.xyz, r0.xyzx, cb0[10].zzzz
    r0.xyz = ((r0.xyzx)*(source[10].zzzz)).xyz;
    // 93: max r0.w, r2.z, l(0.000000)
    r0.w = (max(r2.zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 94: mul r1.xyz, r0.wwww, cb2[3].xyzx
    r1.xyz = ((r0.wwww)*(passValues[3].xyzx)).xyz;
    // 95: mad r0.xyz, r0.xyzx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 96: mul r0.xyz, r3.xyzx, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.xyzx)).xyz;
    // 97: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 98: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 99: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 100: ret
    return output;
}

// source.character.static-map-native-1158.v1 / source program e93a314bc8c25c42988b8107a7084189
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1158(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[8];
    source[3]=g_SourceCharacterLightConstants[9];
    source[4]=g_SourceCharacterLightConstants[10];
    source[5]=g_SourceCharacterLightConstants[11];
    source[6]=g_SourceCharacterLightConstants[15];
    source[7]=g_SourceCharacterLightConstants[16];
    source[8]=g_SourceCharacterLightConstants[17];
    source[9]=g_SourceCharacterLightConstants[18];
    source[10]=float4(input.lightColor,1.f);
    source[11].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f;
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
    // 23: mul r4.xy, r4.xyxx, cb0[5].xxxx
    r4.xy = ((r4.xyxx)*(source[5].xxxx)).xy;
    // 24: mul r5.xy, r4.xyxx, v2.wwww
    r5.xy = ((r4.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r4.xyw, r5.xyxz, r1.wwww
    r4.xyw = ((r5.xyxz)/(r1.wwww)).xyw;
    // 28: dp3 r1.w, r4.xywx, r4.xywx
    r1.w = (dot((r4.xywx).xyz,(r4.xywx).xyz).xxxx).w;
    // 29: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 30: mul r4.xyw, r1.wwww, r4.xyxw
    r4.xyw = ((r1.wwww)*(r4.xyxw)).xyw;
    // 31: dp3 r1.w, r4.xywx, r2.xyzx
    r1.w = (dot((r4.xywx).xyz,(r2.xyzx).xyz).xxxx).w;
    // 32: mul r5.xyz, r1.wwww, r4.xywx
    r5.xyz = ((r1.wwww)*(r4.xywx)).xyz;
    // 33: mad r5.xyz, r5.xyzx, l(2.000000, 2.000000, 2.000000, 0.000000), -r2.xyzx
    r5.xyz = ((r5.xyzx)*(float4(2.000000,2.000000,2.000000,0.000000))+(-(r2.xyzx))).xyz;
    // 34: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, v4.xyxx, t1.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 35: add r2.w, r6.w, l(-0.333300)
    r2.w = ((r6.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 36: lt r2.w, r2.w, l(0.000000)
    r2.w = (asfloat((uint4)((r2.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 37: discard_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) { output.discarded = true; return output; }
    // 38: ne r2.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r2.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).w;
    // 39: if_nz r2.w
    if ((asuint(r2.wwww)).x != 0u) {
    // 40: div r7.xy, v8.xyxx, v8.wwww
    r7.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 41: mad r7.xy, r7.xyxx, cb2[0].xyxx, cb2[0].wzww
    r7.xy = ((r7.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 42: sample_indexable(texture2d)(float,float,float,float) r7.xyz, r7.xyxx, t4.xyzw, s0
    r7.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 43: mul r7.xyz, r7.xyzx, r7.xyzx
    r7.xyz = ((r7.xyzx)*(r7.xyzx)).xyz;
    // 44: else
    } else {
    // 45: mov r7.xyz, l(1.000000,1.000000,1.000000,0)
    r7.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 46: endif
    }
    // 47: add r8.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r8.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 48: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 49: dp3 r1.y, r0.xyzx, r5.xyzx
    r1.y = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 50: mul r0.xy, cb0[0].xyxx, l(0.000300, 0.000300, 0.000000, 0.000000)
    r0.xy = ((source[0].xyxx)*(float4(0.000300,0.000300,0.000000,0.000000))).xy;
    // 51: mad r0.xy, cb0[6].yyyy, r1.xyxx, r0.xyxx
    r0.xy = ((source[6].yyyy)*(r1.xyxx)+(r0.xyxx)).xy;
    // 52: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t2.xyzw, s2, l(0.000000)
    r0.xyz = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 53: mul r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 54: mad r0.xyz, cb0[6].zzzz, r0.xyzx, r0.xyzx
    r0.xyz = ((source[6].zzzz)*(r0.xyzx)+(r0.xyzx)).xyz;
    // 55: add r0.xyz, r0.xyzx, -cb0[6].zzzz
    r0.xyz = ((r0.xyzx)+(-(source[6].zzzz))).xyz;
    // 56: mov_sat r1.xyz, r0.xyzx
    r1.xyz = (saturate(r0.xyzx)).xyz;
    // 57: mul r2.w, r4.z, cb0[6].w
    r2.w = ((r4.zzzz)*(source[6].wwww)).w;
    // 58: mul r5.xyz, cb0[3].xyzx, cb0[7].xxxx
    r5.xyz = ((source[3].xyzx)*(source[7].xxxx)).xyz;
    // 59: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 60: mul r9.xyz, cb0[4].xyzx, cb0[7].yyyy
    r9.xyz = ((source[4].xyzx)*(source[7].yyyy)).xyz;
    // 61: sample_b_indexable(texture2d)(float,float,float,float) r10.xy, v4.xyxx, t3.yzxw, s4, l(0.000000)
    r10.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 62: mul r3.w, r10.y, cb0[7].z
    r3.w = ((r10.yyyy)*(source[7].zzzz)).w;
    // 63: lt r4.z, |r3.w|, l(0.000001)
    r4.z = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 64: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 65: mul r3.w, r3.w, cb0[7].w
    r3.w = ((r3.wwww)*(source[7].wwww)).w;
    // 66: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 67: movc r3.w, r4.z, l(0), r3.w
    r3.w = ((asuint(r4.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 68: min r4.z, r3.w, l(1.000000)
    r4.z = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 69: mad r6.xyz, r9.xyzx, r6.xyzx, -r5.xyzx
    r6.xyz = ((r9.xyzx)*(r6.xyzx)+(-(r5.xyzx))).xyz;
    // 70: mad r5.xyz, r4.zzzz, r6.xyzx, r5.xyzx
    r5.xyz = ((r4.zzzz)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 71: mad r1.xyz, r2.wwww, r1.xyzx, r5.xyzx
    r1.xyz = ((r2.wwww)*(r1.xyzx)+(r5.xyzx)).xyz;
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
    // 77: mul r1.xyz, r0.xyzx, cb0[8].xxxx
    r1.xyz = ((r0.xyzx)*(source[8].xxxx)).xyz;
    // 78: mad r0.xyz, cb0[8].yyyy, r0.xyzx, -r1.xyzx
    r0.xyz = ((source[8].yyyy)*(r0.xyzx)+(-(r1.xyzx))).xyz;
    // 79: mad r0.xyz, r4.zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((r4.zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 80: mul r0.xyz, r8.xyzx, r0.xyzx
    r0.xyz = ((r8.xyzx)*(r0.xyzx)).xyz;
    // 81: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 82: mov_sat r1.x, cb0[8].z
    r1.x = (saturate(source[8].zzzz)).x;
    // 83: mul_sat r1.y, r3.w, cb2[3].w
    r1.y = (saturate((r3.wwww)*(passValues[3].wwww))).y;
    // 84: mul r1.z, r10.x, cb0[9].x
    r1.z = ((r10.xxxx)*(source[9].xxxx)).z;
    // 85: lt r2.w, |r1.z|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 86: log r1.z, |r1.z|
    r1.z = (log2(abs(r1.zzzz))).z;
    // 87: mul r1.z, r1.z, cb0[9].y
    r1.z = ((r1.zzzz)*(source[9].yyyy)).z;
    // 88: exp r1.z, r1.z
    r1.z = (exp2(r1.zzzz)).z;
    // 89: movc r1.z, r2.w, l(0), r1.z
    r1.z = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.zzzz)).z;
    // 90: max r1.z, r1.z, cb0[0].w
    r1.z = (max(r1.zzzz,source[0].wwww)).z;
    // 91: mad r5.xyz, v5.xyzx, r0.wwww, r2.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r2.xyzx)).xyz;
    // 92: dp3 r2.w, r5.xyzx, r5.xyzx
    r2.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 93: rsq r2.w, r2.w
    r2.w = (rsqrt(r2.wwww)).w;
    // 94: mul r5.xyz, r2.wwww, r5.xyzx
    r5.xyz = ((r2.wwww)*(r5.xyzx)).xyz;
    // 95: dp3_sat r2.w, r4.xywx, r5.xyzx
    r2.w = (saturate(dot((r4.xywx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 96: add r1.w, |r1.w|, l(0.000010)
    r1.w = ((abs(r1.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 97: min r1.zw, r1.zzzw, l(0.000000, 0.000000, 1.000000, 1.000000)
    r1.zw = (min(r1.zzzw,float4(0.000000,0.000000,1.000000,1.000000))).zw;
    // 98: dp3_sat r3.x, r4.xywx, r3.xyzx
    r3.x = (saturate(dot((r4.xywx).xyz,(r3.xyzx).xyz).xxxx)).x;
    // 99: dp3_sat r2.x, r2.xyzx, r5.xyzx
    r2.x = (saturate(dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx)).x;
    // 100: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 101: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 102: add r2.x, r2.x, l(1.000000)
    r2.x = ((r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: add r0.w, -r0.w, r2.x
    r0.w = ((-(r0.wwww))+(r2.xxxx)).w;
    // 104: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 105: mad r2.xyz, -r0.xyzx, r1.yyyy, r0.xyzx
    r2.xyz = ((-(r0.xyzx))*(r1.yyyy)+(r0.xyzx)).xyz;
    // 106: mul r3.y, r1.z, r1.z
    r3.y = ((r1.zzzz)*(r1.zzzz)).y;
    // 107: mul r3.z, r3.y, r3.y
    r3.z = ((r3.yyyy)*(r3.yyyy)).z;
    // 108: mad r3.w, r2.w, r3.z, -r2.w
    r3.w = ((r2.wwww)*(r3.zzzz)+(-(r2.wwww))).w;
    // 109: mad r2.w, r3.w, r2.w, l(1.000000)
    r2.w = ((r3.wwww)*(r2.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mul r2.w, r2.w, r2.w
    r2.w = ((r2.wwww)*(r2.wwww)).w;
    // 111: mul r2.xyzw, r2.xyzw, l(0.318310, 0.318310, 0.318310, 3.141593)
    r2.xyzw = ((r2.xyzw)*(float4(0.318310,0.318310,0.318310,3.141593))).xyzw;
    // 112: div r2.w, r3.z, r2.w
    r2.w = ((r3.zzzz)/(r2.wwww)).w;
    // 113: mad r3.z, -r1.z, r1.z, l(1.000000)
    r3.z = ((-(r1.zzzz))*(r1.zzzz)+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 114: mad r3.w, r1.w, r3.z, r3.y
    r3.w = ((r1.wwww)*(r3.zzzz)+(r3.yyyy)).w;
    // 115: mad r3.y, r3.x, r3.z, r3.y
    r3.y = ((r3.xxxx)*(r3.zzzz)+(r3.yyyy)).y;
    // 116: mul r1.w, r1.w, r3.y
    r1.w = ((r1.wwww)*(r3.yyyy)).w;
    // 117: mad r1.w, r3.x, r3.w, r1.w
    r1.w = ((r3.xxxx)*(r3.wwww)+(r1.wwww)).w;
    // 118: rcp r1.w, r1.w
    r1.w = (1.0/(r1.wwww)).w;
    // 119: mul r1.w, r1.w, r2.w
    r1.w = ((r1.wwww)*(r2.wwww)).w;
    // 120: mul r2.w, r1.x, l(0.080000)
    r2.w = ((r1.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).w;
    // 121: mad r0.xyz, -r1.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r1.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 122: mad r0.xyz, r1.yyyy, r0.xyzx, r2.wwww
    r0.xyz = ((r1.yyyy)*(r0.xyzx)+(r2.wwww)).xyz;
    // 123: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 124: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 125: mul r1.x, r1.x, r1.x
    r1.x = ((r1.xxxx)*(r1.xxxx)).x;
    // 126: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 127: mul_sat r1.x, r0.y, l(50.000000)
    r1.x = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).x;
    // 128: mul r1.x, r0.w, r1.x
    r1.x = ((r0.wwww)*(r1.xxxx)).x;
    // 129: add r1.y, -r1.z, l(1.000000)
    r1.y = ((-(r1.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 130: max r3.yzw, r0.xxyz, r1.yyyy
    r3.yzw = (max(r0.xxyz,r1.yyyy)).yzw;
    // 131: add r3.yzw, -r0.xxyz, r3.yyzw
    r3.yzw = ((-(r0.xxyz))+(r3.yyzw)).yzw;
    // 132: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 133: mad r0.xyz, r1.xxxx, r3.yzwy, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r3.yzwy)+(r0.xyzx)).xyz;
    // 134: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 135: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 136: mul r1.x, r1.w, l(0.500000)
    r1.x = ((r1.wwww)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 137: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 138: min r0.w, r0.w, r1.x
    r0.w = (min(r0.wwww,r1.xxxx)).w;
    // 139: mul r1.xyz, r0.xyzx, r0.wwww
    r1.xyz = ((r0.xyzx)*(r0.wwww)).xyz;
    // 140: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 141: mad r0.xyz, r2.xyzx, r0.xyzx, r1.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 142: mul r0.xyz, r3.xxxx, r0.xyzx
    r0.xyz = ((r3.xxxx)*(r0.xyzx)).xyz;
    // 143: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 144: mul r0.xyz, r7.xyzx, r0.xyzx
    r0.xyz = ((r7.xyzx)*(r0.xyzx)).xyz;
    // 145: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 146: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 147: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 148: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 149: ret
    return output;
}

// source.character.static-map-native-1159.v1 / source program 0b1910aebaac454aa1dd5d95e2b7889c
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1159(SOURCE_CHARACTER_NATIVE_INPUT input)
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
    source[8]=g_SourceCharacterLightConstants[7];
    source[9]=g_SourceCharacterLightConstants[8];
    source[10]=g_SourceCharacterLightConstants[9];
    source[11]=float4(input.lightColor,1.f);
    source[17].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
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
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s0, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // 23: mul r5.xy, r5.xyxx, cb0[5].xxxx
    r5.xy = ((r5.xyxx)*(source[5].xxxx)).xy;
    // 24: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 28: mul r6.xy, v4.xyxx, cb0[5].yyyy
    r6.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r6.zw, r6.xyxx, t1.zwxy, s1, l(0.000000)
    r6.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 30: mad r6.zw, r6.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r6.zw = ((r6.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 31: dp2 r1.w, r6.zwzz, r6.zwzz
    r1.w = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 35: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 36: mul r7.xy, r6.zwzz, cb0[5].zzzz
    r7.xy = ((r6.zwzz)*(source[5].zzzz)).xy;
    // 37: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 39: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 41: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 42: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 43: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 44: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 45: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 47: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: mad r0.x, r0.x, l(0.500000), cb0[6].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s2, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 51: mul_sat r0.y, r0.y, r8.w
    r0.y = (saturate((r0.yyyy)*(r8.wwww))).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s3, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 54: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 55: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 56: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 57: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 58: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 59: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 60: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 61: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 62: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 63: add r1.xyz, -r5.xyzx, r7.xyzx
    r1.xyz = ((-(r5.xyzx))+(r7.xyzx)).xyz;
    // 64: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 65: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[17].y
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[17].yyyy)) * 0xffffffffu)).y;
    // 69: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 70: mul r2.xyzw, v8.yyyy, cb0[13].xyzw
    r2.xyzw = ((v8.yyyy)*(source[13].xyzw)).xyzw;
    // 71: mad r2.xyzw, cb0[12].xyzw, v8.xxxx, r2.xyzw
    r2.xyzw = ((source[12].xyzw)*(v8.xxxx)+(r2.xyzw)).xyzw;
    // 72: mad r2.xyzw, cb0[14].xyzw, v8.zzzz, r2.xyzw
    r2.xyzw = ((source[14].xyzw)*(v8.zzzz)+(r2.xyzw)).xyzw;
    // 73: mad r2.xyzw, cb0[15].xyzw, v8.wwww, r2.xyzw
    r2.xyzw = ((source[15].xyzw)*(v8.wwww)+(r2.xyzw)).xyzw;
    // 74: div r0.yz, r2.xxyx, r2.wwww
    r0.yz = ((r2.xxyx)/(r2.wwww)).yz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r5.x, r0.yzyy, t4.xyzw, s5
    r5.x = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).x;
    // 76: mov r7.xw, l(0,0,0,0)
    r7.xw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xw;
    // 77: mov r7.yz, cb0[16].wwzw
    r7.yz = (source[16].wwzw).yz;
    // 78: add r7.xyzw, r0.yzyz, r7.xyzw
    r7.xyzw = ((r0.yzyz)+(r7.xyzw)).xyzw;
    // 79: sample_indexable(texture2d)(float,float,float,float) r5.y, r7.xyxx, t4.yxzw, s5
    r5.y = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yxzw).y;
    // 80: sample_indexable(texture2d)(float,float,float,float) r5.z, r7.zwzz, t4.yzxw, s5
    r5.z = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzxw).z;
    // 81: add r2.xy, r0.yzyy, cb0[16].zwzz
    r2.xy = ((r0.yzyy)+(source[16].zwzz)).xy;
    // 82: sample_indexable(texture2d)(float,float,float,float) r5.w, r2.xyxx, t4.yzwx, s5
    r5.w = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).yzwx).w;
    // 83: lt r2.xyzw, r2.zzzz, r5.xyzw
    r2.xyzw = (asfloat((uint4)((r2.zzzz)<(r5.xyzw)) * 0xffffffffu)).xyzw;
    // 84: and r5.xyzw, r2.xyzw, l(0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
    r5.xyzw = (asfloat(asuint(r2.xyzw) & uint4(0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u))).xyzw;
    // 85: mul r0.yz, r0.yyzy, cb0[16].xxyx
    r0.yz = ((r0.yyzy)*(source[16].xxyx)).yz;
    // 86: frc r0.yz, r0.yyzy
    r0.yz = (frac(r0.yyzy)).yz;
    // 87: movc r2.xy, r2.xyxx, l(-1.000000,-1.000000,0,0), l(-0.000000,-0.000000,0,0)
    r2.xy = ((asuint(r2.xyxx) != 0u) ? (float4(-1.000000,-1.000000,asfloat(0u),asfloat(0u))) : (float4(-0.000000,-0.000000,asfloat(0u),asfloat(0u)))).xy;
    // 88: add r2.xy, r2.xyxx, r5.zwzz
    r2.xy = ((r2.xyxx)+(r5.zwzz)).xy;
    // 89: mad r2.xy, r0.yyyy, r2.xyxx, r5.xyxx
    r2.xy = ((r0.yyyy)*(r2.xyxx)+(r5.xyxx)).xy;
    // 90: add r0.y, -r2.x, r2.y
    r0.y = ((-(r2.xxxx))+(r2.yyyy)).y;
    // 91: mad r0.y, r0.z, r0.y, r2.x
    r0.y = ((r0.zzzz)*(r0.yyyy)+(r2.xxxx)).y;
    // 92: mul r2.xyz, r0.yyyy, cb0[17].xxxx
    r2.xyz = ((r0.yyyy)*(source[17].xxxx)).xyz;
    // 93: else
    } else {
    // 94: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 95: endif
    }
    // 96: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 97: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 98: add r7.xyz, -r8.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))+(r0.yyyy)).xyz;
    // 99: mad r7.xyz, cb0[7].xxxx, r7.xyzx, r8.xyzx
    r7.xyz = ((source[7].xxxx)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 100: mul r8.xyz, cb0[3].xyzx, cb0[7].yyyy
    r8.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 101: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 102: mul r10.xyz, cb0[4].xyzx, cb0[7].zzzz
    r10.xyz = ((source[4].xyzx)*(source[7].zzzz)).xyz;
    // 103: mul r11.xyz, r6.xyzx, r10.xyzx
    r11.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 104: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 105: mad r6.xyz, -r10.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r10.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 106: mad r6.xyz, cb0[8].xxxx, r6.xyzx, r11.xyzx
    r6.xyz = ((source[8].xxxx)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 107: mad r6.xyz, -r7.xyzx, r8.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r6.xyzx)).xyz;
    // 108: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 109: mul r6.xyz, r0.xyzx, cb0[8].yyyy
    r6.xyz = ((r0.xyzx)*(source[8].yyyy)).xyz;
    // 110: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t5.yzxw, s4, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 111: mul r1.w, r7.y, cb0[8].w
    r1.w = ((r7.yyyy)*(source[8].wwww)).w;
    // 112: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 114: mul r1.w, r1.w, cb0[9].x
    r1.w = ((r1.wwww)*(source[9].xxxx)).w;
    // 115: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 116: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 117: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: mad r0.xyz, cb0[8].zzzz, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].zzzz)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 119: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 120: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 121: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 122: mov_sat r2.w, cb0[9].y
    r2.w = (saturate(source[9].yyyy)).w;
    // 123: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 124: mul r3.w, r7.x, cb0[10].y
    r3.w = ((r7.xxxx)*(source[10].yyyy)).w;
    // 125: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 126: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 127: mul r3.w, r3.w, cb0[10].z
    r3.w = ((r3.wwww)*(source[10].zzzz)).w;
    // 128: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 129: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 130: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 131: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 132: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 133: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 134: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 135: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 136: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 137: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 138: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 139: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 140: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 141: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 142: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 144: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 145: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 146: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 147: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 148: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 149: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 150: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 151: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 152: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 153: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 154: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 155: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 156: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 157: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 158: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 159: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 160: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 161: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 162: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 163: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 164: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 165: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 166: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 167: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 168: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 169: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 170: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 171: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 172: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 173: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 174: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 175: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 176: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 177: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 178: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 179: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 180: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 181: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 182: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 183: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 184: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 185: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 186: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 187: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 188: mul o0.xyz, r0.xyzx, cb0[11].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[11].xyzx)).xyz;
    // 189: add r0.x, -v2.w, cb0[9].w
    r0.x = ((-(v2.wwww))+(source[9].wwww)).x;
    // 190: add r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)+(source[9].zzzz)).x;
    // 191: add_sat r0.x, r0.x, l(1.000000)
    r0.x = (saturate((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000)))).x;
    // 192: mul o0.w, r0.x, cb0[0].y
    output.targets[0].w = ((r0.xxxx)*(source[0].yyyy)).w;
    // 193: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 194: ret
    return output;
}

// source.character.static-map-native-1160.v1 / source program 3fcfaeb383f46f489dc2e865e3bdaf17
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1160(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[8];
    source[3]=g_SourceCharacterLightConstants[9];
    source[4]=g_SourceCharacterLightConstants[14];
    source[5]=g_SourceCharacterLightConstants[15];
    source[6]=g_SourceCharacterLightConstants[16];
    source[7]=float4(input.lightColor,1.f);
    source[8].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f;
    // 1: dp3 r0.x, v7.xyzx, v7.xyzx
    r0.x = (dot((v7.xyzx).xyz,(v7.xyzx).xyz).xxxx).x;
    // 2: rsq r0.x, r0.x
    r0.x = (rsqrt(r0.xxxx)).x;
    // 3: mul r0.xyz, r0.xxxx, v7.xyzx
    r0.xyz = ((r0.xxxx)*(v7.xyzx)).xyz;
    // 4: dp3 r0.w, v5.xyzx, v5.xyzx
    r0.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 5: rsq r0.w, r0.w
    r0.w = (rsqrt(r0.wwww)).w;
    // 6: mul r1.xyz, r0.wwww, v5.xyzx
    r1.xyz = ((r0.wwww)*(v5.xyzx)).xyz;
    // 7: sample_b_indexable(texture2d)(float,float,float,float) r1.w, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r1.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 8: add r2.x, r1.w, -cb0[3].x
    r2.x = ((r1.wwww)+(-(source[3].xxxx))).x;
    // 9: mul r2.x, r2.x, cb0[3].y
    r2.x = ((r2.xxxx)*(source[3].yyyy)).x;
    // 10: mad r2.xy, r0.xyxx, r2.xxxx, v4.xyxx
    r2.xy = ((r0.xyxx)*(r2.xxxx)+(v4.xyxx)).xy;
    // 11: sample_b_indexable(texture2d)(float,float,float,float) r2.zw, r2.xyxx, t1.zwxy, s2, l(0.000000)
    r2.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 12: mad r2.zw, r2.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r2.zw = ((r2.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 13: dp2 r3.x, r2.zwzz, r2.zwzz
    r3.x = (dot((r2.zwzz).xy,(r2.zwzz).xy).xxxx).x;
    // 14: add r3.x, -r3.x, l(1.000000)
    r3.x = ((-(r3.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 15: max r3.x, r3.x, l(0.000000)
    r3.x = (max(r3.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 16: sqrt r3.x, r3.x
    r3.x = (sqrt(r3.xxxx)).x;
    // 17: add r3.z, r3.x, l(0.000010)
    r3.z = ((r3.xxxx)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 18: mul r2.zw, r2.zzzw, cb0[3].zzzz
    r2.zw = ((r2.zzzw)*(source[3].zzzz)).zw;
    // 19: mul r3.xy, r2.zwzz, v2.wwww
    r3.xy = ((r2.zwzz)*(v2.wwww)).xy;
    // 20: dp3 r2.z, r3.xyzx, r3.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 21: sqrt r2.z, r2.z
    r2.z = (sqrt(r2.zzzz)).z;
    // 22: div r3.xyz, r3.xyzx, r2.zzzz
    r3.xyz = ((r3.xyzx)/(r2.zzzz)).xyz;
    // 23: dp3 r2.z, r3.xyzx, r3.xyzx
    r2.z = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).z;
    // 24: rsq r2.z, r2.z
    r2.z = (rsqrt(r2.zzzz)).z;
    // 25: mul r3.xyz, r2.zzzz, r3.xyzx
    r3.xyz = ((r2.zzzz)*(r3.xyzx)).xyz;
    // 26: sample_b_indexable(texture2d)(float,float,float,float) r4.xyzw, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r4.xyzw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 27: add_sat r2.z, r1.w, r4.w
    r2.z = (saturate((r1.wwww)+(r4.wwww))).z;
    // 28: add r2.z, r2.z, l(-0.333300)
    r2.z = ((r2.zzzz)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).z;
    // 29: lt r2.z, r2.z, l(0.000000)
    r2.z = (asfloat((uint4)((r2.zzzz)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).z;
    // 30: discard_nz r2.z
    if ((asuint(r2.zzzz)).x != 0u) { output.discarded = true; return output; }
    // 31: ne r2.z, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[8].x
    r2.z = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[8].xxxx)) * 0xffffffffu)).z;
    // 32: if_nz r2.z
    if ((asuint(r2.zzzz)).x != 0u) {
    // 33: div r2.zw, v8.xxxy, v8.wwww
    r2.zw = ((v8.xxxy)/(v8.wwww)).zw;
    // 34: mad r2.zw, r2.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r2.zw = ((r2.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // 35: sample_indexable(texture2d)(float,float,float,float) r5.xyz, r2.zwzz, t3.xyzw, s0
    r5.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 36: mul r5.xyz, r5.xyzx, r5.xyzx
    r5.xyz = ((r5.xyzx)*(r5.xyzx)).xyz;
    // 37: else
    } else {
    // 38: mov r5.xyz, l(1.000000,1.000000,1.000000,0)
    r5.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 39: endif
    }
    // 40: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 41: mul r7.xyz, cb0[2].xyzx, cb0[4].yyyy
    r7.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 42: mul r4.xyz, r4.xyzx, r7.xyzx
    r4.xyz = ((r4.xyzx)*(r7.xyzx)).xyz;
    // 43: add_sat r1.w, r1.w, cb0[4].z
    r1.w = (saturate((r1.wwww)+(source[4].zzzz))).w;
    // 44: mul r4.xyz, r1.wwww, r4.xyzx
    r4.xyz = ((r1.wwww)*(r4.xyzx)).xyz;
    // 45: mul r7.xyz, r4.xyzx, cb0[4].wwww
    r7.xyz = ((r4.xyzx)*(source[4].wwww)).xyz;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t2.yzxw, s3, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 47: mul r1.w, r2.y, cb0[5].y
    r1.w = ((r2.yyyy)*(source[5].yyyy)).w;
    // 48: lt r2.y, |r1.w|, l(0.000001)
    r2.y = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 49: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 50: mul r1.w, r1.w, cb0[5].z
    r1.w = ((r1.wwww)*(source[5].zzzz)).w;
    // 51: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 52: movc r1.w, r2.y, l(0), r1.w
    r1.w = ((asuint(r2.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 53: min r2.y, r1.w, l(1.000000)
    r2.y = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: mad r4.xyz, cb0[5].xxxx, r4.xyzx, -r7.xyzx
    r4.xyz = ((source[5].xxxx)*(r4.xyzx)+(-(r7.xyzx))).xyz;
    // 55: mad r2.yzw, r2.yyyy, r4.xxyz, r7.xxyz
    r2.yzw = ((r2.yyyy)*(r4.xxyz)+(r7.xxyz)).yzw;
    // 56: mul r2.yzw, r6.xxyz, r2.yyzw
    r2.yzw = ((r6.xxyz)*(r2.yyzw)).yzw;
    // 57: mad_sat r2.yzw, r2.yyzw, cb2[3].wwww, cb2[3].xxyz
    r2.yzw = (saturate((r2.yyzw)*(passValues[3].wwww)+(passValues[3].xxyz))).yzw;
    // 58: mov_sat r3.w, cb0[5].w
    r3.w = (saturate(source[5].wwww)).w;
    // 59: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 60: mul r2.x, r2.x, cb0[6].y
    r2.x = ((r2.xxxx)*(source[6].yyyy)).x;
    // 61: lt r4.x, |r2.x|, l(0.000001)
    r4.x = (asfloat((uint4)((abs(r2.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 62: log r2.x, |r2.x|
    r2.x = (log2(abs(r2.xxxx))).x;
    // 63: mul r2.x, r2.x, cb0[6].z
    r2.x = ((r2.xxxx)*(source[6].zzzz)).x;
    // 64: exp r2.x, r2.x
    r2.x = (exp2(r2.xxxx)).x;
    // 65: movc r2.x, r4.x, l(0), r2.x
    r2.x = ((asuint(r4.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r2.xxxx)).x;
    // 66: max r2.x, r2.x, cb0[0].x
    r2.x = (max(r2.xxxx,source[0].xxxx)).x;
    // 67: min r2.x, r2.x, l(1.000000)
    r2.x = (min(r2.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 68: mad r4.xyz, v5.xyzx, r0.wwww, r0.xyzx
    r4.xyz = ((v5.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 69: dp3 r4.w, r4.xyzx, r4.xyzx
    r4.w = (dot((r4.xyzx).xyz,(r4.xyzx).xyz).xxxx).w;
    // 70: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 71: mul r4.xyz, r4.wwww, r4.xyzx
    r4.xyz = ((r4.wwww)*(r4.xyzx)).xyz;
    // 72: dp3_sat r4.w, r3.xyzx, r4.xyzx
    r4.w = (saturate(dot((r3.xyzx).xyz,(r4.xyzx).xyz).xxxx)).w;
    // 73: dp3 r5.w, r3.xyzx, r0.xyzx
    r5.w = (dot((r3.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 74: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 75: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 76: dp3_sat r1.x, r3.xyzx, r1.xyzx
    r1.x = (saturate(dot((r3.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 77: dp3_sat r0.x, r0.xyzx, r4.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 78: mad r0.y, v5.z, r0.w, l(1.000000)
    r0.y = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 79: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 80: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 81: add r0.x, -r0.y, r0.x
    r0.x = ((-(r0.yyyy))+(r0.xxxx)).x;
    // 82: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 83: mad r0.yzw, -r2.yyzw, r1.wwww, r2.yyzw
    r0.yzw = ((-(r2.yyzw))*(r1.wwww)+(r2.yyzw)).yzw;
    // 84: mul r0.yzw, r0.yyzw, l(0.000000, 0.318310, 0.318310, 0.318310)
    r0.yzw = ((r0.yyzw)*(float4(0.000000,0.318310,0.318310,0.318310))).yzw;
    // 85: mul r1.y, r2.x, r2.x
    r1.y = ((r2.xxxx)*(r2.xxxx)).y;
    // 86: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 87: mad r3.x, r4.w, r1.z, -r4.w
    r3.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 88: mad r3.x, r3.x, r4.w, l(1.000000)
    r3.x = ((r3.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 89: mul r3.x, r3.x, r3.x
    r3.x = ((r3.xxxx)*(r3.xxxx)).x;
    // 90: mul r3.x, r3.x, l(3.141593)
    r3.x = ((r3.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 91: div r1.z, r1.z, r3.x
    r1.z = ((r1.zzzz)/(r3.xxxx)).z;
    // 92: mad r3.x, -r2.x, r2.x, l(1.000000)
    r3.x = ((-(r2.xxxx))*(r2.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 93: mad r3.y, r5.w, r3.x, r1.y
    r3.y = ((r5.wwww)*(r3.xxxx)+(r1.yyyy)).y;
    // 94: mad r1.y, r1.x, r3.x, r1.y
    r1.y = ((r1.xxxx)*(r3.xxxx)+(r1.yyyy)).y;
    // 95: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 96: mad r1.y, r1.x, r3.y, r1.y
    r1.y = ((r1.xxxx)*(r3.yyyy)+(r1.yyyy)).y;
    // 97: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 98: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 99: mul r1.z, r3.w, l(0.080000)
    r1.z = ((r3.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 100: mad r2.yzw, -r3.wwww, l(0.000000, 0.080000, 0.080000, 0.080000), r2.yyzw
    r2.yzw = ((-(r3.wwww))*(float4(0.000000,0.080000,0.080000,0.080000))+(r2.yyzw)).yzw;
    // 101: mad r2.yzw, r1.wwww, r2.yyzw, r1.zzzz
    r2.yzw = ((r1.wwww)*(r2.yyzw)+(r1.zzzz)).yzw;
    // 102: add r0.x, -r0.x, l(1.000000)
    r0.x = ((-(r0.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 103: mul r1.z, r0.x, r0.x
    r1.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 104: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 105: mul r0.x, r0.x, r1.z
    r0.x = ((r0.xxxx)*(r1.zzzz)).x;
    // 106: mul_sat r1.z, r2.z, l(50.000000)
    r1.z = (saturate((r2.zzzz)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 107: mul r1.z, r0.x, r1.z
    r1.z = ((r0.xxxx)*(r1.zzzz)).z;
    // 108: add r1.w, -r2.x, l(1.000000)
    r1.w = ((-(r2.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 109: max r3.xyz, r2.yzwy, r1.wwww
    r3.xyz = (max(r2.yzwy,r1.wwww)).xyz;
    // 110: add r3.xyz, -r2.yzwy, r3.xyzx
    r3.xyz = ((-(r2.yzwy))+(r3.xyzx)).xyz;
    // 111: mad r2.xyz, -r0.xxxx, r2.yzwy, r2.yzwy
    r2.xyz = ((-(r0.xxxx))*(r2.yzwy)+(r2.yzwy)).xyz;
    // 112: mad r2.xyz, r1.zzzz, r3.xyzx, r2.xyzx
    r2.xyz = ((r1.zzzz)*(r3.xyzx)+(r2.xyzx)).xyz;
    // 113: dp3 r0.x, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 114: add r0.x, r0.x, l(0.000100)
    r0.x = ((r0.xxxx)+(float4(0.000100,0.000100,0.000100,0.000100))).x;
    // 115: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 116: div r0.x, l(3.000000), r0.x
    r0.x = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.xxxx)).x;
    // 117: min r0.x, r0.x, r1.y
    r0.x = (min(r0.xxxx,r1.yyyy)).x;
    // 118: mul r1.yzw, r2.xxyz, r0.xxxx
    r1.yzw = ((r2.xxyz)*(r0.xxxx)).yzw;
    // 119: add r2.xyz, -r2.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r2.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 120: mad r0.xyz, r0.yzwy, r2.xyzx, r1.yzwy
    r0.xyz = ((r0.yzwy)*(r2.xyzx)+(r1.yzwy)).xyz;
    // 121: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 122: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 123: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 124: mul o0.xyz, r0.xyzx, cb0[7].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[7].xyzx)).xyz;
    // 125: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 126: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 127: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 128: ret
    return output;
}

// source.character.static-map-native-1161.v1 / source program e5341398e6dc2f419fd71ed7c5730657
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1161(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[63]=g_SourceCharacterLightConstants[63];
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[7];
    source[4]=g_SourceCharacterLightConstants[8];
    source[5]=g_SourceCharacterLightConstants[9];
    source[6]=g_SourceCharacterLightConstants[10];
    source[7]=g_SourceCharacterLightConstants[11];
    source[8]=g_SourceCharacterLightConstants[15];
    source[8].x=(((float4(1.5,0,0,0)+sin(((source[63].xxxx*g_SourceCharacterTime.xxxx)*float4(6.28318548,0,0,0))))*float4(0.400000006,0,0,0))).x;
    source[9]=g_SourceCharacterLightConstants[16];
    source[10]=g_SourceCharacterLightConstants[17];
    source[11]=g_SourceCharacterLightConstants[18];
    source[12]=g_SourceCharacterLightConstants[19];
    source[13]=float4(input.lightColor,1.f);
    source[14].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f;
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
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // 27: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 28: mul r6.xy, v4.xyxx, cb0[6].yyyy
    r6.xy = ((v4.xyxx)*(source[6].yyyy)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r6.zw, r6.xyxx, t1.zwxy, s2, l(0.000000)
    r6.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 30: mad r6.zw, r6.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r6.zw = ((r6.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 31: dp2 r1.w, r6.zwzz, r6.zwzz
    r1.w = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 35: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 36: mul r7.xy, r6.zwzz, cb0[6].zzzz
    r7.xy = ((r6.zwzz)*(source[6].zzzz)).xy;
    // 37: max r1.w, cb0[6].w, l(0.000000)
    r1.w = (max(source[6].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 39: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 41: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 42: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 43: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 44: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 45: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 47: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: mad r0.x, r0.x, l(0.500000), cb0[7].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[7].zzzz)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 51: mul_sat r0.y, r0.y, r8.w
    r0.y = (saturate((r0.yyyy)*(r8.wwww))).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 54: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 55: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 56: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 57: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 58: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 59: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 60: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 61: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 62: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 63: add r1.xyz, -r5.xyzx, r7.xyzx
    r1.xyz = ((-(r5.xyzx))+(r7.xyzx)).xyz;
    // 64: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 65: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 68: add r0.y, r8.w, l(-0.333300)
    r0.y = ((r8.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).y;
    // 69: lt r0.y, r0.y, l(0.000000)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 70: discard_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) { output.discarded = true; return output; }
    // 71: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[14].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[14].xxxx)) * 0xffffffffu)).y;
    // 72: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 73: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 74: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 75: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 76: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 77: else
    } else {
    // 78: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 79: endif
    }
    // 80: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 81: dp3 r0.y, r8.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r8.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 82: add r7.xyz, -r8.xyzx, r0.yyyy
    r7.xyz = ((-(r8.xyzx))+(r0.yyyy)).xyz;
    // 83: mad r7.xyz, cb0[8].wwww, r7.xyzx, r8.xyzx
    r7.xyz = ((source[8].wwww)*(r7.xyzx)+(r8.xyzx)).xyz;
    // 84: mul r9.xyz, cb0[3].xyzx, cb0[9].xxxx
    r9.xyz = ((source[3].xyzx)*(source[9].xxxx)).xyz;
    // 85: mul r7.xyz, r7.xyzx, r9.xyzx
    r7.xyz = ((r7.xyzx)*(r9.xyzx)).xyz;
    // 86: mul r9.xyz, cb0[4].xyzx, cb0[9].yyyy
    r9.xyz = ((source[4].xyzx)*(source[9].yyyy)).xyz;
    // 87: sample_b_indexable(texture2d)(float,float,float,float) r0.yz, v4.xyxx, t4.xyzw, s5, l(0.000000)
    r0.yz = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).yz;
    // 88: mul r0.z, r0.z, cb0[9].z
    r0.z = ((r0.zzzz)*(source[9].zzzz)).z;
    // 89: lt r1.w, |r0.z|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.zzzz))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 90: log r0.z, |r0.z|
    r0.z = (log2(abs(r0.zzzz))).z;
    // 91: mul r0.z, r0.z, cb0[9].w
    r0.z = ((r0.zzzz)*(source[9].wwww)).z;
    // 92: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 93: movc r0.z, r1.w, l(0), r0.z
    r0.z = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 94: min r1.w, r0.z, l(1.000000)
    r1.w = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 95: mad r8.xyz, r9.xyzx, r8.xyzx, -r7.xyzx
    r8.xyz = ((r9.xyzx)*(r8.xyzx)+(-(r7.xyzx))).xyz;
    // 96: mad r7.xyz, r1.wwww, r8.xyzx, r7.xyzx
    r7.xyz = ((r1.wwww)*(r8.xyzx)+(r7.xyzx)).xyz;
    // 97: mul r8.xyz, cb0[5].xyzx, cb0[10].xxxx
    r8.xyz = ((source[5].xyzx)*(source[10].xxxx)).xyz;
    // 98: mul r9.xyz, r6.xyzx, r8.xyzx
    r9.xyz = ((r6.xyzx)*(r8.xyzx)).xyz;
    // 99: dp3 r2.w, r9.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r2.w = (dot((r9.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 100: mad r6.xyz, -r8.xyzx, r6.xyzx, r2.wwww
    r6.xyz = ((-(r8.xyzx))*(r6.xyzx)+(r2.wwww)).xyz;
    // 101: mad r6.xyz, cb0[10].zzzz, r6.xyzx, r9.xyzx
    r6.xyz = ((source[10].zzzz)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 102: add r6.xyz, -r7.xyzx, r6.xyzx
    r6.xyz = ((-(r7.xyzx))+(r6.xyzx)).xyz;
    // 103: mad r6.xyz, r0.xxxx, r6.xyzx, r7.xyzx
    r6.xyz = ((r0.xxxx)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 104: mul r7.xyz, r6.xyzx, cb0[10].wwww
    r7.xyz = ((r6.xyzx)*(source[10].wwww)).xyz;
    // 105: mad r6.xyz, cb0[11].xxxx, r6.xyzx, -r7.xyzx
    r6.xyz = ((source[11].xxxx)*(r6.xyzx)+(-(r7.xyzx))).xyz;
    // 106: mad r6.xyz, r1.wwww, r6.xyzx, r7.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)+(r7.xyzx)).xyz;
    // 107: mul r5.xyz, r5.xyzx, r6.xyzx
    r5.xyz = ((r5.xyzx)*(r6.xyzx)).xyz;
    // 108: mad_sat r5.xyz, r5.xyzx, cb2[3].wwww, cb2[3].xyzx
    r5.xyz = (saturate((r5.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 109: mov_sat r0.x, cb0[11].y
    r0.x = (saturate(source[11].yyyy)).x;
    // 110: mul_sat r0.z, r0.z, cb2[3].w
    r0.z = (saturate((r0.zzzz)*(passValues[3].wwww))).z;
    // 111: mul r0.y, r0.y, cb0[11].w
    r0.y = ((r0.yyyy)*(source[11].wwww)).y;
    // 112: lt r1.w, |r0.y|, l(0.000001)
    r1.w = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 113: log r0.y, |r0.y|
    r0.y = (log2(abs(r0.yyyy))).y;
    // 114: mul r0.y, r0.y, cb0[12].x
    r0.y = ((r0.yyyy)*(source[12].xxxx)).y;
    // 115: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 116: movc r0.y, r1.w, l(0), r0.y
    r0.y = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).y;
    // 117: max r0.y, r0.y, cb0[0].x
    r0.y = (max(r0.yyyy,source[0].xxxx)).y;
    // 118: mad r6.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r6.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 119: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 120: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 121: mul r6.xyz, r1.wwww, r6.xyzx
    r6.xyz = ((r1.wwww)*(r6.xyzx)).xyz;
    // 122: dp3_sat r1.w, r1.xyzx, r6.xyzx
    r1.w = (saturate(dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx)).w;
    // 123: dp3 r2.w, r1.xyzx, r3.xyzx
    r2.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 124: add r2.w, |r2.w|, l(0.000010)
    r2.w = ((abs(r2.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 125: min r2.w, r2.w, l(1.000000)
    r2.w = (min(r2.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 127: dp3_sat r1.y, r3.xyzx, r6.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r6.xyzx).xyz).xxxx)).y;
    // 128: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 129: min r0.yw, r0.yyyw, l(0.000000, 1.000000, 0.000000, 1.000000)
    r0.yw = (min(r0.yyyw,float4(0.000000,1.000000,0.000000,1.000000))).yw;
    // 130: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 131: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 132: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 133: mad r3.xyz, -r5.xyzx, r0.zzzz, r5.xyzx
    r3.xyz = ((-(r5.xyzx))*(r0.zzzz)+(r5.xyzx)).xyz;
    // 134: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 135: mul r1.y, r0.y, r0.y
    r1.y = ((r0.yyyy)*(r0.yyyy)).y;
    // 136: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 137: mad r3.w, r1.w, r1.z, -r1.w
    r3.w = ((r1.wwww)*(r1.zzzz)+(-(r1.wwww))).w;
    // 138: mad r1.w, r3.w, r1.w, l(1.000000)
    r1.w = ((r3.wwww)*(r1.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 139: mul r1.w, r1.w, r1.w
    r1.w = ((r1.wwww)*(r1.wwww)).w;
    // 140: mul r1.w, r1.w, l(3.141593)
    r1.w = ((r1.wwww)*(float4(3.141593,3.141593,3.141593,3.141593))).w;
    // 141: div r1.z, r1.z, r1.w
    r1.z = ((r1.zzzz)/(r1.wwww)).z;
    // 142: mad r1.w, -r0.y, r0.y, l(1.000000)
    r1.w = ((-(r0.yyyy))*(r0.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 143: mad r3.w, r2.w, r1.w, r1.y
    r3.w = ((r2.wwww)*(r1.wwww)+(r1.yyyy)).w;
    // 144: mad r1.y, r1.x, r1.w, r1.y
    r1.y = ((r1.xxxx)*(r1.wwww)+(r1.yyyy)).y;
    // 145: mul r1.y, r1.y, r2.w
    r1.y = ((r1.yyyy)*(r2.wwww)).y;
    // 146: mad r1.y, r1.x, r3.w, r1.y
    r1.y = ((r1.xxxx)*(r3.wwww)+(r1.yyyy)).y;
    // 147: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 148: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 149: mul r1.z, r0.x, l(0.080000)
    r1.z = ((r0.xxxx)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 150: mad r4.xyz, -r0.xxxx, l(0.080000, 0.080000, 0.080000, 0.000000), r5.xyzx
    r4.xyz = ((-(r0.xxxx))*(float4(0.080000,0.080000,0.080000,0.000000))+(r5.xyzx)).xyz;
    // 151: mad r4.xyz, r0.zzzz, r4.xyzx, r1.zzzz
    r4.xyz = ((r0.zzzz)*(r4.xyzx)+(r1.zzzz)).xyz;
    // 152: add r0.xy, -r0.wyww, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = ((-(r0.wyww))+(float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 153: mul r0.z, r0.x, r0.x
    r0.z = ((r0.xxxx)*(r0.xxxx)).z;
    // 154: mul r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)*(r0.zzzz)).z;
    // 155: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 156: mul_sat r0.z, r4.y, l(50.000000)
    r0.z = (saturate((r4.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 157: mul r0.z, r0.x, r0.z
    r0.z = ((r0.xxxx)*(r0.zzzz)).z;
    // 158: max r5.xyz, r4.xyzx, r0.yyyy
    r5.xyz = (max(r4.xyzx,r0.yyyy)).xyz;
    // 159: add r5.xyz, -r4.xyzx, r5.xyzx
    r5.xyz = ((-(r4.xyzx))+(r5.xyzx)).xyz;
    // 160: mad r0.xyw, -r0.xxxx, r4.xyxz, r4.xyxz
    r0.xyw = ((-(r0.xxxx))*(r4.xyxz)+(r4.xyxz)).xyw;
    // 161: mad r0.xyz, r0.zzzz, r5.xyzx, r0.xywx
    r0.xyz = ((r0.zzzz)*(r5.xyzx)+(r0.xywx)).xyz;
    // 162: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 163: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 164: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 165: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 166: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 167: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 168: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 169: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 170: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 171: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 172: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 173: mul o0.xyz, r0.xyzx, cb0[13].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[13].xyzx)).xyz;
    // 174: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 175: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 176: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 177: ret
    return output;
}

// source.character.static-map-native-1162.v1 / source program f69b6ffd785a964cb0d7914eeec5378a
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1162(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=float4(input.lightColor,1.f);
    source[11].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f;
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
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
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
    // 23: mul r5.xy, r5.xyxx, cb0[5].xxxx
    r5.xy = ((r5.xyxx)*(source[5].xxxx)).xy;
    // 24: mul r6.xy, r5.xyxx, v2.wwww
    r6.xy = ((r5.xyxx)*(v2.wwww)).xy;
    // 25: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 26: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 27: div r5.xyz, r6.xyzx, r1.wwww
    r5.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 28: mul r6.xy, v4.xyxx, cb0[5].yyyy
    r6.xy = ((v4.xyxx)*(source[5].yyyy)).xy;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r6.zw, r6.xyxx, t1.zwxy, s2, l(0.000000)
    r6.zw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 30: mad r6.zw, r6.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r6.zw = ((r6.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 31: dp2 r1.w, r6.zwzz, r6.zwzz
    r1.w = (dot((r6.zwzz).xy,(r6.zwzz).xy).xxxx).w;
    // 32: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 33: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 34: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 35: add r7.z, r1.w, l(0.000010)
    r7.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 36: mul r7.xy, r6.zwzz, cb0[5].zzzz
    r7.xy = ((r6.zwzz)*(source[5].zzzz)).xy;
    // 37: max r1.w, cb0[5].w, l(0.000000)
    r1.w = (max(source[5].wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 39: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 41: dp3 r1.x, r1.xyzx, r5.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx).x;
    // 42: dp3 r1.y, r2.xyzx, r5.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r5.xyzx).xyz).xxxx).y;
    // 43: dp3 r1.z, r0.xyzx, r5.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r5.xyzx).xyz).xxxx).z;
    // 44: max r0.xyz, cb0[2].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[2].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 45: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 46: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 47: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 48: mad r0.x, r0.x, l(0.500000), cb0[6].z
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[6].zzzz)).x;
    // 49: sample_b_indexable(texture2d)(float,float,float,float) r8.xyzw, v4.xyxx, t2.xyzw, s3, l(0.000000)
    r8.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 50: mul r0.y, r5.z, r5.z
    r0.y = ((r5.zzzz)*(r5.zzzz)).y;
    // 51: mul_sat r0.y, r0.y, r8.w
    r0.y = (saturate((r0.yyyy)*(r8.wwww))).y;
    // 52: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 53: sample_b_indexable(texture2d)(float,float,float,float) r6.xyzw, r6.xyxx, t3.xyzw, s4, l(0.000000)
    r6.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r6.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 54: mul r0.z, r6.w, r6.w
    r0.z = ((r6.wwww)*(r6.wwww)).z;
    // 55: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 56: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 57: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 58: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 59: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 60: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 61: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 62: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 63: add r1.xyz, -r5.xyzx, r7.xyzx
    r1.xyz = ((-(r5.xyzx))+(r7.xyzx)).xyz;
    // 64: mad r1.xyz, r0.yyyy, r1.xyzx, r5.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r5.xyzx)).xyz;
    // 65: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 66: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 67: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 68: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[11].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[11].xxxx)) * 0xffffffffu)).y;
    // 69: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 70: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 71: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 72: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 73: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 74: else
    } else {
    // 75: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 76: endif
    }
    // 77: add r5.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r5.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 78: mul r7.xyz, cb0[3].xyzx, cb0[6].wwww
    r7.xyz = ((source[3].xyzx)*(source[6].wwww)).xyz;
    // 79: mul r9.xyz, r7.xyzx, r8.xyzx
    r9.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 80: mul r10.xyz, cb0[4].xyzx, cb0[7].xxxx
    r10.xyz = ((source[4].xyzx)*(source[7].xxxx)).xyz;
    // 81: mul r11.xyz, r6.xyzx, r10.xyzx
    r11.xyz = ((r6.xyzx)*(r10.xyzx)).xyz;
    // 82: dp3 r0.y, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 83: mad r6.xyz, -r10.xyzx, r6.xyzx, r0.yyyy
    r6.xyz = ((-(r10.xyzx))*(r6.xyzx)+(r0.yyyy)).xyz;
    // 84: mad r6.xyz, cb0[7].zzzz, r6.xyzx, r11.xyzx
    r6.xyz = ((source[7].zzzz)*(r6.xyzx)+(r11.xyzx)).xyz;
    // 85: mad r6.xyz, -r8.xyzx, r7.xyzx, r6.xyzx
    r6.xyz = ((-(r8.xyzx))*(r7.xyzx)+(r6.xyzx)).xyz;
    // 86: mad r0.xyz, r0.xxxx, r6.xyzx, r9.xyzx
    r0.xyz = ((r0.xxxx)*(r6.xyzx)+(r9.xyzx)).xyz;
    // 87: mul r6.xyz, r0.xyzx, cb0[7].wwww
    r6.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 88: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, v4.xyxx, t4.yzxw, s5, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 89: mul r1.w, r7.y, cb0[8].y
    r1.w = ((r7.yyyy)*(source[8].yyyy)).w;
    // 90: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 91: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 92: mul r1.w, r1.w, cb0[8].z
    r1.w = ((r1.wwww)*(source[8].zzzz)).w;
    // 93: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 94: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 95: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 96: mad r0.xyz, cb0[8].xxxx, r0.xyzx, -r6.xyzx
    r0.xyz = ((source[8].xxxx)*(r0.xyzx)+(-(r6.xyzx))).xyz;
    // 97: mad r0.xyz, r2.wwww, r0.xyzx, r6.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r6.xyzx)).xyz;
    // 98: mul r0.xyz, r5.xyzx, r0.xyzx
    r0.xyz = ((r5.xyzx)*(r0.xyzx)).xyz;
    // 99: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 100: mov_sat r2.w, cb0[8].w
    r2.w = (saturate(source[8].wwww)).w;
    // 101: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 102: mul r3.w, r7.x, cb0[9].y
    r3.w = ((r7.xxxx)*(source[9].yyyy)).w;
    // 103: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 104: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 105: mul r3.w, r3.w, cb0[9].z
    r3.w = ((r3.wwww)*(source[9].zzzz)).w;
    // 106: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 107: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 108: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 109: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 110: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 111: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 112: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 113: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 114: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 115: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 116: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 117: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 118: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 119: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 120: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 121: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 123: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 124: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 125: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 126: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 127: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 128: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 129: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 130: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 131: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 132: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 133: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 134: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 135: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 136: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 137: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 138: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 139: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 140: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 141: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 142: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 143: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 144: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 145: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 146: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 147: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 148: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 149: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 150: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 151: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 152: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 153: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 154: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 155: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 156: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 157: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 158: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 159: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 160: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 161: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 162: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 163: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 164: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 165: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 166: mul o0.xyz, r0.xyzx, cb0[10].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[10].xyzx)).xyz;
    // 167: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 168: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 169: ret
    return output;
}

// source.character.static-map-native-1163.v1 / source program 91f91ee81cf28a4e9688d566a67b80dc
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1163(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[0];
    source[2]=g_SourceCharacterLightConstants[1];
    source[3]=g_SourceCharacterLightConstants[2];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=g_SourceCharacterLightConstants[8];
    source[9]=g_SourceCharacterLightConstants[9];
    source[10]=g_SourceCharacterLightConstants[10];
    source[11]=g_SourceCharacterLightConstants[11];
    source[12]=float4(input.lightColor,1.f);
    source[13].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
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
    // 16: mul r5.xy, v4.xyxx, cb0[2].xyxx
    r5.xy = ((v4.xyxx)*(source[2].xyxx)).xy;
    // 17: sample_b_indexable(texture2d)(float,float,float,float) r5.zw, r5.xyxx, t0.zwxy, s1, l(0.000000)
    r5.zw = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).zwxy).zw;
    // 18: mad r5.zw, r5.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r5.zw = ((r5.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 19: dp2 r1.w, r5.zwzz, r5.zwzz
    r1.w = (dot((r5.zwzz).xy,(r5.zwzz).xy).xxxx).w;
    // 20: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 21: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 22: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 23: add r6.z, r1.w, l(0.000010)
    r6.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 24: mul r5.zw, r5.zzzw, cb0[6].wwww
    r5.zw = ((r5.zzzw)*(source[6].wwww)).zw;
    // 25: mul r6.xy, r5.zwzz, v2.wwww
    r6.xy = ((r5.zwzz)*(v2.wwww)).xy;
    // 26: dp3 r1.w, r6.xyzx, r6.xyzx
    r1.w = (dot((r6.xyzx).xyz,(r6.xyzx).xyz).xxxx).w;
    // 27: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 28: div r6.xyz, r6.xyzx, r1.wwww
    r6.xyz = ((r6.xyzx)/(r1.wwww)).xyz;
    // 29: mul r5.zw, v4.xxxy, cb0[7].xxxx
    r5.zw = ((v4.xxxy)*(source[7].xxxx)).zw;
    // 30: sample_b_indexable(texture2d)(float,float,float,float) r7.xy, r5.zwzz, t1.xyzw, s2, l(0.000000)
    r7.xy = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 31: mad r7.xy, r7.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r7.xy = ((r7.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 32: dp2 r1.w, r7.xyxx, r7.xyxx
    r1.w = (dot((r7.xyxx).xy,(r7.xyxx).xy).xxxx).w;
    // 33: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 35: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 36: add r8.z, r1.w, l(0.000010)
    r8.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 37: mul r8.xy, r7.xyxx, cb0[7].yyyy
    r8.xy = ((r7.xyxx)*(source[7].yyyy)).xy;
    // 38: max r1.w, cb0[7].z, l(0.000000)
    r1.w = (max(source[7].zzzz,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 39: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 40: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 41: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 42: dp3 r1.x, r1.xyzx, r6.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r6.xyzx).xyz).xxxx).x;
    // 43: dp3 r1.y, r2.xyzx, r6.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r6.xyzx).xyz).xxxx).y;
    // 44: dp3 r1.z, r0.xyzx, r6.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r6.xyzx).xyz).xxxx).z;
    // 45: max r0.xyz, cb0[3].xyzx, l(-1.000000, -1.000000, -1.000000, 0.000000)
    r0.xyz = (max(source[3].xyzx,float4(-1.000000,-1.000000,-1.000000,0.000000))).xyz;
    // 46: min r0.xyz, r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = (min(r0.xyzx,float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 47: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 48: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 49: mad r0.x, r0.x, l(0.500000), cb0[8].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[8].yyyy)).x;
    // 50: sample_b_indexable(texture2d)(float,float,float,float) r7.xyzw, r5.xyxx, t2.xyzw, s3, l(0.000000)
    r7.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 51: mul r0.y, r6.z, r6.z
    r0.y = ((r6.zzzz)*(r6.zzzz)).y;
    // 52: mul_sat r0.y, r0.y, r7.w
    r0.y = (saturate((r0.yyyy)*(r7.wwww))).y;
    // 53: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r9.xyzw, r5.zwzz, t3.xyzw, s4, l(0.000000)
    r9.xyzw = ((g_SourceCharacterTexture3.SampleBias(SourceCharacterSampler, (r5.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 55: mul r0.z, r9.w, r9.w
    r0.z = ((r9.wwww)*(r9.wwww)).z;
    // 56: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 57: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 58: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 59: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 60: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 61: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 62: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 63: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 64: add r1.xyz, -r6.xyzx, r8.xyzx
    r1.xyz = ((-(r6.xyzx))+(r8.xyzx)).xyz;
    // 65: mad r1.xyz, r0.yyyy, r1.xyzx, r6.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)+(r6.xyzx)).xyz;
    // 66: dp3 r0.y, r1.xyzx, r1.xyzx
    r0.y = (dot((r1.xyzx).xyz,(r1.xyzx).xyz).xxxx).y;
    // 67: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 68: mul r1.xyz, r0.yyyy, r1.xyzx
    r1.xyz = ((r0.yyyy)*(r1.xyzx)).xyz;
    // 69: ne r0.y, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[13].x
    r0.y = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[13].xxxx)) * 0xffffffffu)).y;
    // 70: if_nz r0.y
    if ((asuint(r0.yyyy)).x != 0u) {
    // 71: div r0.yz, v8.xxyx, v8.wwww
    r0.yz = ((v8.xxyx)/(v8.wwww)).yz;
    // 72: mad r0.yz, r0.yyzy, cb2[0].xxyx, cb2[0].wwzw
    r0.yz = ((r0.yyzy)*(passValues[0].xxyx)+(passValues[0].wwzw)).yz;
    // 73: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r0.yzyy, t5.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 74: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 75: else
    } else {
    // 76: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 77: endif
    }
    // 78: add r6.xyz, -cb0[1].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r6.xyz = ((-(source[1].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 79: mul r8.xyz, cb0[4].xyzx, cb0[8].zzzz
    r8.xyz = ((source[4].xyzx)*(source[8].zzzz)).xyz;
    // 80: mul r10.xyz, r7.xyzx, r8.xyzx
    r10.xyz = ((r7.xyzx)*(r8.xyzx)).xyz;
    // 81: mul r11.xyz, cb0[5].xyzx, cb0[8].wwww
    r11.xyz = ((source[5].xyzx)*(source[8].wwww)).xyz;
    // 82: mul r12.xyz, r9.xyzx, r11.xyzx
    r12.xyz = ((r9.xyzx)*(r11.xyzx)).xyz;
    // 83: dp3 r0.y, r12.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.y = (dot((r12.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).y;
    // 84: mad r9.xyz, -r11.xyzx, r9.xyzx, r0.yyyy
    r9.xyz = ((-(r11.xyzx))*(r9.xyzx)+(r0.yyyy)).xyz;
    // 85: mad r9.xyz, cb0[9].yyyy, r9.xyzx, r12.xyzx
    r9.xyz = ((source[9].yyyy)*(r9.xyzx)+(r12.xyzx)).xyz;
    // 86: mad r7.xyz, -r7.xyzx, r8.xyzx, r9.xyzx
    r7.xyz = ((-(r7.xyzx))*(r8.xyzx)+(r9.xyzx)).xyz;
    // 87: mad r0.xyz, r0.xxxx, r7.xyzx, r10.xyzx
    r0.xyz = ((r0.xxxx)*(r7.xyzx)+(r10.xyzx)).xyz;
    // 88: mul r7.xyz, r0.xyzx, cb0[9].zzzz
    r7.xyz = ((r0.xyzx)*(source[9].zzzz)).xyz;
    // 89: sample_b_indexable(texture2d)(float,float,float,float) r5.xy, r5.xyxx, t4.yzxw, s5, l(0.000000)
    r5.xy = ((g_SourceCharacterTexture4.SampleBias(SourceCharacterSampler, (r5.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).yzxw).xy;
    // 90: mul r1.w, r5.y, cb0[10].x
    r1.w = ((r5.yyyy)*(source[10].xxxx)).w;
    // 91: lt r2.w, |r1.w|, l(0.000001)
    r2.w = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 92: log r1.w, |r1.w|
    r1.w = (log2(abs(r1.wwww))).w;
    // 93: mul r1.w, r1.w, cb0[10].y
    r1.w = ((r1.wwww)*(source[10].yyyy)).w;
    // 94: exp r1.w, r1.w
    r1.w = (exp2(r1.wwww)).w;
    // 95: movc r1.w, r2.w, l(0), r1.w
    r1.w = ((asuint(r2.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.wwww)).w;
    // 96: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 97: mad r0.xyz, cb0[9].wwww, r0.xyzx, -r7.xyzx
    r0.xyz = ((source[9].wwww)*(r0.xyzx)+(-(r7.xyzx))).xyz;
    // 98: mad r0.xyz, r2.wwww, r0.xyzx, r7.xyzx
    r0.xyz = ((r2.wwww)*(r0.xyzx)+(r7.xyzx)).xyz;
    // 99: mul r0.xyz, r6.xyzx, r0.xyzx
    r0.xyz = ((r6.xyzx)*(r0.xyzx)).xyz;
    // 100: mad_sat r0.xyz, r0.xyzx, cb2[3].wwww, cb2[3].xyzx
    r0.xyz = (saturate((r0.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx))).xyz;
    // 101: mov_sat r2.w, cb0[10].z
    r2.w = (saturate(source[10].zzzz)).w;
    // 102: mul_sat r1.w, r1.w, cb2[3].w
    r1.w = (saturate((r1.wwww)*(passValues[3].wwww))).w;
    // 103: mul r3.w, r5.x, cb0[11].x
    r3.w = ((r5.xxxx)*(source[11].xxxx)).w;
    // 104: lt r4.w, |r3.w|, l(0.000001)
    r4.w = (asfloat((uint4)((abs(r3.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 105: log r3.w, |r3.w|
    r3.w = (log2(abs(r3.wwww))).w;
    // 106: mul r3.w, r3.w, cb0[11].y
    r3.w = ((r3.wwww)*(source[11].yyyy)).w;
    // 107: exp r3.w, r3.w
    r3.w = (exp2(r3.wwww)).w;
    // 108: movc r3.w, r4.w, l(0), r3.w
    r3.w = ((asuint(r4.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r3.wwww)).w;
    // 109: max r3.w, r3.w, cb0[0].x
    r3.w = (max(r3.wwww,source[0].xxxx)).w;
    // 110: min r3.w, r3.w, l(1.000000)
    r3.w = (min(r3.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 111: mad r5.xyz, v5.xyzx, r0.wwww, r3.xyzx
    r5.xyz = ((v5.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 112: dp3 r4.w, r5.xyzx, r5.xyzx
    r4.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 113: rsq r4.w, r4.w
    r4.w = (rsqrt(r4.wwww)).w;
    // 114: mul r5.xyz, r4.wwww, r5.xyzx
    r5.xyz = ((r4.wwww)*(r5.xyzx)).xyz;
    // 115: dp3_sat r4.w, r1.xyzx, r5.xyzx
    r4.w = (saturate(dot((r1.xyzx).xyz,(r5.xyzx).xyz).xxxx)).w;
    // 116: dp3 r5.w, r1.xyzx, r3.xyzx
    r5.w = (dot((r1.xyzx).xyz,(r3.xyzx).xyz).xxxx).w;
    // 117: add r5.w, |r5.w|, l(0.000010)
    r5.w = ((abs(r5.wwww))+(float4(0.000010,0.000010,0.000010,0.000010))).w;
    // 118: min r5.w, r5.w, l(1.000000)
    r5.w = (min(r5.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 119: dp3_sat r1.x, r1.xyzx, r4.xyzx
    r1.x = (saturate(dot((r1.xyzx).xyz,(r4.xyzx).xyz).xxxx)).x;
    // 120: dp3_sat r1.y, r3.xyzx, r5.xyzx
    r1.y = (saturate(dot((r3.xyzx).xyz,(r5.xyzx).xyz).xxxx)).y;
    // 121: mad r0.w, v5.z, r0.w, l(1.000000)
    r0.w = ((v5.zzzz)*(r0.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 122: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 123: add r1.y, r1.y, l(1.000000)
    r1.y = ((r1.yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 124: add r0.w, -r0.w, r1.y
    r0.w = ((-(r0.wwww))+(r1.yyyy)).w;
    // 125: min r0.w, r0.w, l(1.000000)
    r0.w = (min(r0.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 126: mad r3.xyz, -r0.xyzx, r1.wwww, r0.xyzx
    r3.xyz = ((-(r0.xyzx))*(r1.wwww)+(r0.xyzx)).xyz;
    // 127: mul r3.xyz, r3.xyzx, l(0.318310, 0.318310, 0.318310, 0.000000)
    r3.xyz = ((r3.xyzx)*(float4(0.318310,0.318310,0.318310,0.000000))).xyz;
    // 128: mul r1.y, r3.w, r3.w
    r1.y = ((r3.wwww)*(r3.wwww)).y;
    // 129: mul r1.z, r1.y, r1.y
    r1.z = ((r1.yyyy)*(r1.yyyy)).z;
    // 130: mad r4.x, r4.w, r1.z, -r4.w
    r4.x = ((r4.wwww)*(r1.zzzz)+(-(r4.wwww))).x;
    // 131: mad r4.x, r4.x, r4.w, l(1.000000)
    r4.x = ((r4.xxxx)*(r4.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 132: mul r4.x, r4.x, r4.x
    r4.x = ((r4.xxxx)*(r4.xxxx)).x;
    // 133: mul r4.x, r4.x, l(3.141593)
    r4.x = ((r4.xxxx)*(float4(3.141593,3.141593,3.141593,3.141593))).x;
    // 134: div r1.z, r1.z, r4.x
    r1.z = ((r1.zzzz)/(r4.xxxx)).z;
    // 135: mad r4.x, -r3.w, r3.w, l(1.000000)
    r4.x = ((-(r3.wwww))*(r3.wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 136: mad r4.y, r5.w, r4.x, r1.y
    r4.y = ((r5.wwww)*(r4.xxxx)+(r1.yyyy)).y;
    // 137: mad r1.y, r1.x, r4.x, r1.y
    r1.y = ((r1.xxxx)*(r4.xxxx)+(r1.yyyy)).y;
    // 138: mul r1.y, r1.y, r5.w
    r1.y = ((r1.yyyy)*(r5.wwww)).y;
    // 139: mad r1.y, r1.x, r4.y, r1.y
    r1.y = ((r1.xxxx)*(r4.yyyy)+(r1.yyyy)).y;
    // 140: rcp r1.y, r1.y
    r1.y = (1.0/(r1.yyyy)).y;
    // 141: mul r1.y, r1.y, r1.z
    r1.y = ((r1.yyyy)*(r1.zzzz)).y;
    // 142: mul r1.z, r2.w, l(0.080000)
    r1.z = ((r2.wwww)*(float4(0.080000,0.080000,0.080000,0.080000))).z;
    // 143: mad r0.xyz, -r2.wwww, l(0.080000, 0.080000, 0.080000, 0.000000), r0.xyzx
    r0.xyz = ((-(r2.wwww))*(float4(0.080000,0.080000,0.080000,0.000000))+(r0.xyzx)).xyz;
    // 144: mad r0.xyz, r1.wwww, r0.xyzx, r1.zzzz
    r0.xyz = ((r1.wwww)*(r0.xyzx)+(r1.zzzz)).xyz;
    // 145: add r0.w, -r0.w, l(1.000000)
    r0.w = ((-(r0.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 146: mul r1.z, r0.w, r0.w
    r1.z = ((r0.wwww)*(r0.wwww)).z;
    // 147: mul r1.z, r1.z, r1.z
    r1.z = ((r1.zzzz)*(r1.zzzz)).z;
    // 148: mul r0.w, r0.w, r1.z
    r0.w = ((r0.wwww)*(r1.zzzz)).w;
    // 149: mul_sat r1.z, r0.y, l(50.000000)
    r1.z = (saturate((r0.yyyy)*(float4(50.000000,50.000000,50.000000,50.000000)))).z;
    // 150: mul r1.z, r0.w, r1.z
    r1.z = ((r0.wwww)*(r1.zzzz)).z;
    // 151: add r1.w, -r3.w, l(1.000000)
    r1.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 152: max r4.xyz, r0.xyzx, r1.wwww
    r4.xyz = (max(r0.xyzx,r1.wwww)).xyz;
    // 153: add r4.xyz, -r0.xyzx, r4.xyzx
    r4.xyz = ((-(r0.xyzx))+(r4.xyzx)).xyz;
    // 154: mad r0.xyz, -r0.wwww, r0.xyzx, r0.xyzx
    r0.xyz = ((-(r0.wwww))*(r0.xyzx)+(r0.xyzx)).xyz;
    // 155: mad r0.xyz, r1.zzzz, r4.xyzx, r0.xyzx
    r0.xyz = ((r1.zzzz)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 156: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 157: add r0.w, r0.w, l(0.000100)
    r0.w = ((r0.wwww)+(float4(0.000100,0.000100,0.000100,0.000100))).w;
    // 158: mul r1.y, r1.y, l(0.500000)
    r1.y = ((r1.yyyy)*(float4(0.500000,0.500000,0.500000,0.500000))).y;
    // 159: div r0.w, l(3.000000), r0.w
    r0.w = ((float4(3.000000,3.000000,3.000000,3.000000))/(r0.wwww)).w;
    // 160: min r0.w, r0.w, r1.y
    r0.w = (min(r0.wwww,r1.yyyy)).w;
    // 161: mul r1.yzw, r0.xxyz, r0.wwww
    r1.yzw = ((r0.xxyz)*(r0.wwww)).yzw;
    // 162: add r0.xyz, -r0.xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r0.xyz = ((-(r0.xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 163: mad r0.xyz, r3.xyzx, r0.xyzx, r1.yzwy
    r0.xyz = ((r3.xyzx)*(r0.xyzx)+(r1.yzwy)).xyz;
    // 164: mul r0.xyz, r1.xxxx, r0.xyzx
    r0.xyz = ((r1.xxxx)*(r0.xyzx)).xyz;
    // 165: mul r0.xyz, r0.xyzx, l(3.141593, 3.141593, 3.141593, 0.000000)
    r0.xyz = ((r0.xyzx)*(float4(3.141593,3.141593,3.141593,0.000000))).xyz;
    // 166: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 167: mul o0.xyz, r0.xyzx, cb0[12].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[12].xyzx)).xyz;
    // 168: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 169: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 170: ret
    return output;
}

// source.character.static-map-native-1164.v1 / source program ce6651747017204a8ddbed9111c2af58
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1164(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=g_SourceCharacterLightConstants[2];
    source[3]=g_SourceCharacterLightConstants[3];
    source[4]=g_SourceCharacterLightConstants[4];
    source[5]=g_SourceCharacterLightConstants[5];
    source[6]=g_SourceCharacterLightConstants[6];
    source[7]=g_SourceCharacterLightConstants[7];
    source[8]=float4(input.lightColor,1.f);
    source[9].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f, r5=0.f, r6=0.f, r7=0.f, r8=0.f, r9=0.f, r10=0.f, r11=0.f, r12=0.f;
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
    // 12: dp3 r1.w, v5.xyzx, v5.xyzx
    r1.w = (dot((v5.xyzx).xyz,(v5.xyzx).xyz).xxxx).w;
    // 13: rsq r1.w, r1.w
    r1.w = (rsqrt(r1.wwww)).w;
    // 14: mul r3.xyz, r1.wwww, v5.xyzx
    r3.xyz = ((r1.wwww)*(v5.xyzx)).xyz;
    // 15: ne r1.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[9].x
    r1.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[9].xxxx)) * 0xffffffffu)).w;
    // 16: if_nz r1.w
    if ((asuint(r1.wwww)).x != 0u) {
    // 17: div r4.xy, v8.xyxx, v8.wwww
    r4.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 18: mad r4.xy, r4.xyxx, cb2[0].xyxx, cb2[0].wzww
    r4.xy = ((r4.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 19: sample_indexable(texture2d)(float,float,float,float) r4.xyz, r4.xyxx, t3.xyzw, s0
    r4.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 20: mul r4.xyz, r4.xyzx, r4.xyzx
    r4.xyz = ((r4.xyzx)*(r4.xyzx)).xyz;
    // 21: else
    } else {
    // 22: mov r4.xyz, l(1.000000,1.000000,1.000000,0)
    r4.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 23: endif
    }
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r5.xyzw, v4.xyxx, t1.xyzw, s2, l(0.000000)
    r5.xyzw = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 25: dp3 r1.w, r5.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r5.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 26: add r6.xyz, -r5.xyzx, r1.wwww
    r6.xyz = ((-(r5.xyzx))+(r1.wwww)).xyz;
    // 27: mad r6.xyz, cb0[6].xxxx, r6.xyzx, r5.xyzx
    r6.xyz = ((source[6].xxxx)*(r6.xyzx)+(r5.xyzx)).xyz;
    // 28: mul r7.xyz, cb0[1].xyzx, cb0[6].yyyy
    r7.xyz = ((source[1].xyzx)*(source[6].yyyy)).xyz;
    // 29: mul r8.xyz, r6.xyzx, r7.xyzx
    r8.xyz = ((r6.xyzx)*(r7.xyzx)).xyz;
    // 30: mul r9.xyz, cb0[2].xyzx, cb0[6].zzzz
    r9.xyz = ((source[2].xyzx)*(source[6].zzzz)).xyz;
    // 31: mul r10.xy, v4.xyxx, cb0[5].zzzz
    r10.xy = ((v4.xyxx)*(source[5].zzzz)).xy;
    // 32: sample_b_indexable(texture2d)(float,float,float,float) r10.xyzw, r10.xyxx, t2.xyzw, s3, l(0.000000)
    r10.xyzw = ((g_SourceCharacterTexture2.SampleBias(SourceCharacterSampler, (r10.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyzw;
    // 33: mul r11.xyz, r9.xyzx, r10.xyzx
    r11.xyz = ((r9.xyzx)*(r10.xyzx)).xyz;
    // 34: dp3 r1.w, r11.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r11.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 35: mad r9.xyz, -r9.xyzx, r10.xyzx, r1.wwww
    r9.xyz = ((-(r9.xyzx))*(r10.xyzx)+(r1.wwww)).xyz;
    // 36: mad r9.xyz, cb0[7].xxxx, r9.xyzx, r11.xyzx
    r9.xyz = ((source[7].xxxx)*(r9.xyzx)+(r11.xyzx)).xyz;
    // 37: max r1.w, cb0[4].y, l(0.000000)
    r1.w = (max(source[4].yyyy,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 38: min r1.w, r1.w, l(0.990000)
    r1.w = (min(r1.wwww,float4(0.990000,0.990000,0.990000,0.990000))).w;
    // 39: add r2.w, -r1.w, l(1.000000)
    r2.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 40: div r2.w, l(1.000000, 1.000000, 1.000000, 1.000000), r2.w
    r2.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r2.wwww)).w;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r11.xy, v4.xyxx, t0.xyzw, s1, l(0.000000)
    r11.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 42: mad r11.xy, r11.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r11.xy = ((r11.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 43: dp2 r3.w, r11.xyxx, r11.xyxx
    r3.w = (dot((r11.xyxx).xy,(r11.xyxx).xy).xxxx).w;
    // 44: add r3.w, -r3.w, l(1.000000)
    r3.w = ((-(r3.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 45: max r3.w, r3.w, l(0.000000)
    r3.w = (max(r3.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 46: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 47: add r12.z, r3.w, l(0.000010)
    r12.z = ((r3.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 48: mul r11.xy, r11.xyxx, cb0[4].xxxx
    r11.xy = ((r11.xyxx)*(source[4].xxxx)).xy;
    // 49: mul r12.xy, r11.xyxx, v2.wwww
    r12.xy = ((r11.xyxx)*(v2.wwww)).xy;
    // 50: dp3 r3.w, r12.xyzx, r12.xyzx
    r3.w = (dot((r12.xyzx).xyz,(r12.xyzx).xyz).xxxx).w;
    // 51: sqrt r3.w, r3.w
    r3.w = (sqrt(r3.wwww)).w;
    // 52: div r11.xyz, r12.xyzx, r3.wwww
    r11.xyz = ((r12.xyzx)/(r3.wwww)).xyz;
    // 53: dp3 r1.x, r1.xyzx, r11.xyzx
    r1.x = (dot((r1.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 54: dp3 r1.y, r2.xyzx, r11.xyzx
    r1.y = (dot((r2.xyzx).xyz,(r11.xyzx).xyz).xxxx).y;
    // 55: dp3 r1.z, r0.xyzx, r11.xyzx
    r1.z = (dot((r0.xyzx).xyz,(r11.xyzx).xyz).xxxx).z;
    // 56: mul r0.xy, cb0[0].xyxx, cb0[4].zzzz
    r0.xy = ((source[0].xyxx)*(source[4].zzzz)).xy;
    // 57: max r0.xy, -r0.xyxx, l(-1.000000, -1.000000, 0.000000, 0.000000)
    r0.xy = (max(-(r0.xyxx),float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 58: min r0.xy, r0.xyxx, l(1.000000, 1.000000, 0.000000, 0.000000)
    r0.xy = (min(r0.xyxx,float4(1.000000,1.000000,0.000000,0.000000))).xy;
    // 59: mov r0.z, l(1.000000)
    r0.z = (float4(1.000000,1.000000,1.000000,1.000000)).z;
    // 60: dp3 r0.x, r1.xyzx, r0.xyzx
    r0.x = (dot((r1.xyzx).xyz,(r0.xyzx).xyz).xxxx).x;
    // 61: add r0.x, r0.x, l(1.000000)
    r0.x = ((r0.xxxx)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 62: mad r0.x, r0.x, l(0.500000), cb0[5].y
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))+(source[5].yyyy)).x;
    // 63: mul r0.y, r11.z, r11.z
    r0.y = ((r11.zzzz)*(r11.zzzz)).y;
    // 64: mul_sat r0.y, r0.y, r5.w
    r0.y = (saturate((r0.yyyy)*(r5.wwww))).y;
    // 65: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 66: mul r0.z, r10.w, r10.w
    r0.z = ((r10.wwww)*(r10.wwww)).z;
    // 67: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 68: mul r0.z, r0.y, r1.w
    r0.z = ((r0.yyyy)*(r1.wwww)).z;
    // 69: mad r0.x, r0.x, r0.z, r0.x
    r0.x = ((r0.xxxx)*(r0.zzzz)+(r0.xxxx)).x;
    // 70: add r0.z, -r1.w, r0.x
    r0.z = ((-(r1.wwww))+(r0.xxxx)).z;
    // 71: mul r1.x, r0.z, r2.w
    r1.x = ((r0.zzzz)*(r2.wwww)).x;
    // 72: mad r0.x, -r2.w, r0.z, r0.x
    r0.x = ((-(r2.wwww))*(r0.zzzz)+(r0.xxxx)).x;
    // 73: mad_sat r0.x, r0.y, r0.x, r1.x
    r0.x = (saturate((r0.yyyy)*(r0.xxxx)+(r1.xxxx))).x;
    // 74: mad r1.xyz, -r6.xyzx, r7.xyzx, r9.xyzx
    r1.xyz = ((-(r6.xyzx))*(r7.xyzx)+(r9.xyzx)).xyz;
    // 75: mad r1.xyz, r0.xxxx, r1.xyzx, r8.xyzx
    r1.xyz = ((r0.xxxx)*(r1.xyzx)+(r8.xyzx)).xyz;
    // 76: mul r0.y, r0.x, l(0.650000)
    r0.y = ((r0.xxxx)*(float4(0.650000,0.650000,0.650000,0.650000))).y;
    // 77: add r2.xyz, -r11.xyzx, l(0.000000, 0.000000, 1.000000, 0.000000)
    r2.xyz = ((-(r11.xyzx))+(float4(0.000000,0.000000,1.000000,0.000000))).xyz;
    // 78: mad r2.xyz, r0.yyyy, r2.xyzx, r11.xyzx
    r2.xyz = ((r0.yyyy)*(r2.xyzx)+(r11.xyzx)).xyz;
    // 79: dp3 r0.y, r2.xyzx, r3.xyzx
    r0.y = (dot((r2.xyzx).xyz,(r3.xyzx).xyz).xxxx).y;
    // 80: max r0.y, r0.y, l(0.000000)
    r0.y = (max(r0.yyyy,float4(0.000000,0.000000,0.000000,0.000000))).y;
    // 81: mul r2.xyz, cb0[3].xyzx, cb0[7].yyyy
    r2.xyz = ((source[3].xyzx)*(source[7].yyyy)).xyz;
    // 82: mul r2.xyz, r2.xyzx, r5.xyzx
    r2.xyz = ((r2.xyzx)*(r5.xyzx)).xyz;
    // 83: dp3 r1.w, r10.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r10.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 84: add r5.xyz, -r10.xyzx, r1.wwww
    r5.xyz = ((-(r10.xyzx))+(r1.wwww)).xyz;
    // 85: mad r5.xyz, cb0[7].xxxx, r5.xyzx, r10.xyzx
    r5.xyz = ((source[7].xxxx)*(r5.xyzx)+(r10.xyzx)).xyz;
    // 86: mad r5.xyz, cb0[7].zzzz, r5.xyzx, -r2.xyzx
    r5.xyz = ((source[7].zzzz)*(r5.xyzx)+(-(r2.xyzx))).xyz;
    // 87: mad r2.xyz, r0.xxxx, r5.xyzx, r2.xyzx
    r2.xyz = ((r0.xxxx)*(r5.xyzx)+(r2.xyzx)).xyz;
    // 88: mad r3.xyz, v7.xyzx, r0.wwww, r3.xyzx
    r3.xyz = ((v7.xyzx)*(r0.wwww)+(r3.xyzx)).xyz;
    // 89: dp3 r0.x, r3.xyzx, r3.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r3.xyzx).xyz).xxxx).x;
    // 90: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 91: div r3.xyz, r3.xyzx, r0.xxxx
    r3.xyz = ((r3.xyzx)/(r0.xxxx)).xyz;
    // 92: dp3 r0.x, r3.xyzx, r11.xyzx
    r0.x = (dot((r3.xyzx).xyz,(r11.xyzx).xyz).xxxx).x;
    // 93: lt r0.w, |r0.x|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 94: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 95: mul r0.x, r0.x, cb0[7].w
    r0.x = ((r0.xxxx)*(source[7].wwww)).x;
    // 96: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 97: min r0.xz, r0.xxyx, l(1.000000, 0.000000, 1.000000, 0.000000)
    r0.xz = (min(r0.xxyx,float4(1.000000,0.000000,1.000000,0.000000))).xz;
    // 98: movc r0.x, r0.w, l(0), r0.x
    r0.x = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 99: mul r2.xyz, r2.xyzx, r0.xxxx
    r2.xyz = ((r2.xyzx)*(r0.xxxx)).xyz;
    // 100: mad r0.xzw, r0.zzzz, r1.xxyz, r2.xxyz
    r0.xzw = ((r0.zzzz)*(r1.xxyz)+(r2.xxyz)).xzw;
    // 101: mul r1.xyz, r0.yyyy, cb2[3].xyzx
    r1.xyz = ((r0.yyyy)*(passValues[3].xyzx)).xyz;
    // 102: mad r0.xyz, r0.xzwx, cb2[3].wwww, r1.xyzx
    r0.xyz = ((r0.xzwx)*(passValues[3].wwww)+(r1.xyzx)).xyz;
    // 103: mul r0.xyz, r4.xyzx, r0.xyzx
    r0.xyz = ((r4.xyzx)*(r0.xyzx)).xyz;
    // 104: mul o0.xyz, r0.xyzx, cb0[8].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[8].xyzx)).xyz;
    // 105: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 106: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 107: ret
    return output;
}

// source.character.static-map-native-1165.v1 / source program 73058304a62ce84abc801a42b12e1206
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1165(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[0];
    source[1]=g_SourceCharacterLightConstants[1];
    source[2]=float4(input.lightColor,1.f);
    source[3].x=1.f;
    float4 projection[4]; [unroll] for(uint i=0u;i<4u;++i)projection[i]=input.projection[i];
    float4 passValues[5]={float4(.5,-.5,.5,.5),float4(0,0,0,0),float4(0,0,0,0),float4(0,0,0,1),float4(1,1,1,1)};
    float4 v0=input.values[0], v1=input.values[1], v2=input.values[2], v3=input.values[3], v4=input.values[4], v5=input.values[5], v6=input.values[6], v7=input.values[7], v8=input.values[8], v9=input.values[9];
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
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
    // 7: mad r0.xyz, r0.zzzz, l(0.000000, 0.000000, 2.000000, 0.000000), -r0.xyzx
    r0.xyz = ((r0.zzzz)*(float4(0.000000,0.000000,2.000000,0.000000))+(-(r0.xyzx))).xyz;
    // 8: sample_b_indexable(texture2d)(float,float,float,float) r0.w, v2.xyxx, t0.xyzw, s1, l(0.000000)
    r0.w = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).w;
    // 9: add r0.w, r0.w, l(-0.333300)
    r0.w = ((r0.wwww)+(float4(-0.333300,-0.333300,-0.333300,-0.333300))).w;
    // 10: lt r0.w, r0.w, l(0.000000)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).w;
    // 11: discard_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) { output.discarded = true; return output; }
    // 12: ne r0.w, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[3].x
    r0.w = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[3].xxxx)) * 0xffffffffu)).w;
    // 13: if_nz r0.w
    if ((asuint(r0.wwww)).x != 0u) {
    // 14: div r2.xy, v6.xyxx, v6.wwww
    r2.xy = ((v6.xyxx)/(v6.wwww)).xy;
    // 15: mad r2.xy, r2.xyxx, cb2[0].xyxx, cb2[0].wzww
    r2.xy = ((r2.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 16: sample_indexable(texture2d)(float,float,float,float) r2.xyz, r2.xyxx, t1.xyzw, s0
    r2.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 17: mul r2.xyz, r2.xyzx, r2.xyzx
    r2.xyz = ((r2.xyzx)*(r2.xyzx)).xyz;
    // 18: else
    } else {
    // 19: mov r2.xyz, l(1.000000,1.000000,1.000000,0)
    r2.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 20: endif
    }
    // 21: add r3.xyz, -cb0[0].xyzx, l(1.000000, 1.000000, 1.000000, 0.000000)
    r3.xyz = ((-(source[0].xyzx))+(float4(1.000000,1.000000,1.000000,0.000000))).xyz;
    // 22: mul r3.xyz, r3.xyzx, cb0[1].xyzx
    r3.xyz = ((r3.xyzx)*(source[1].xyzx)).xyz;
    // 23: mad r3.xyz, r3.xyzx, cb2[3].wwww, cb2[3].xyzx
    r3.xyz = ((r3.xyzx)*(passValues[3].wwww)+(passValues[3].xyzx)).xyz;
    // 24: mov_sat r0.w, r1.z
    r0.w = (saturate(r1.zzzz)).w;
    // 25: lt r1.w, r0.w, l(0.000001)
    r1.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 26: movc r0.w, r1.w, l(0), r0.w
    r0.w = ((asuint(r1.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 27: dp3_sat r0.x, r0.xyzx, r1.xyzx
    r0.x = (saturate(dot((r0.xyzx).xyz,(r1.xyzx).xyz).xxxx)).x;
    // 28: lt r0.y, r0.x, l(0.000001)
    r0.y = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 29: log r0.x, r0.x
    r0.x = (log2(r0.xxxx)).x;
    // 30: mul r0.x, r0.x, l(15.000000)
    r0.x = ((r0.xxxx)*(float4(15.000000,15.000000,15.000000,15.000000))).x;
    // 31: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 32: mul r1.xyz, r0.xxxx, cb2[4].xyzx
    r1.xyz = ((r0.xxxx)*(passValues[4].xyzx)).xyz;
    // 33: movc r0.xyz, r0.yyyy, l(0,0,0,0), r1.xyzx
    r0.xyz = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xyzx)).xyz;
    // 34: mad r0.xyz, r3.xyzx, r0.wwww, r0.xyzx
    r0.xyz = ((r3.xyzx)*(r0.wwww)+(r0.xyzx)).xyz;
    // 35: mul r0.xyz, r2.xyzx, r0.xyzx
    r0.xyz = ((r2.xyzx)*(r0.xyzx)).xyz;
    // 36: mul o0.xyz, r0.xyzx, cb0[2].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[2].xyzx)).xyz;
    // 37: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 38: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 39: mov oMask, vCoverage.x
    // Coverage is owned by the product rasterizer.
    // 40: ret
    return output;
}

// source.character.static-map-native-1166.v1 / source program 40a03c32003c0c4abd7c97c37cc5017d
SOURCE_CHARACTER_NATIVE_OUTPUT SourceCharacterLight1166(SOURCE_CHARACTER_NATIVE_INPUT input)
{
    SOURCE_CHARACTER_NATIVE_OUTPUT output=(SOURCE_CHARACTER_NATIVE_OUTPUT)0;
    float4 source[64]; [unroll] for(uint i=0u;i<64u;++i) source[i]=0.f;
    source[0]=g_SourceCharacterLightConstants[1];
    source[1]=g_SourceCharacterLightConstants[2];
    source[2]=g_SourceCharacterLightConstants[3];
    source[3]=g_SourceCharacterLightConstants[4];
    source[4]=g_SourceCharacterLightConstants[5];
    source[5]=float4(input.lightColor,1.f);
    source[6].x=1.f;
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
    // 6: ne r1.x, l(0.000000, 0.000000, 0.000000, 0.000000), cb0[6].x
    r1.x = (asfloat((uint4)((float4(0.000000,0.000000,0.000000,0.000000))!=(source[6].xxxx)) * 0xffffffffu)).x;
    // 7: if_nz r1.x
    if ((asuint(r1.xxxx)).x != 0u) {
    // 8: div r1.xy, v8.xyxx, v8.wwww
    r1.xy = ((v8.xyxx)/(v8.wwww)).xy;
    // 9: mad r1.xy, r1.xyxx, cb2[0].xyxx, cb2[0].wzww
    r1.xy = ((r1.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // 10: sample_indexable(texture2d)(float,float,float,float) r1.xyz, r1.xyxx, t2.xyzw, s0
    r1.xyz = ((float4(sqrt(saturate(input.shadow)).xxx,1.0)).xyzw).xyz;
    // 11: mul r1.xyz, r1.xyzx, r1.xyzx
    r1.xyz = ((r1.xyzx)*(r1.xyzx)).xyz;
    // 12: else
    } else {
    // 13: mov r1.xyz, l(1.000000,1.000000,1.000000,0)
    r1.xyz = (float4(1.000000,1.000000,1.000000,asfloat(0u))).xyz;
    // 14: endif
    }
    // 15: mul r2.xy, v4.xyxx, cb0[0].xyxx
    r2.xy = ((v4.xyxx)*(source[0].xyxx)).xy;
    // 16: sample_b_indexable(texture2d)(float,float,float,float) r3.xyz, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r3.xyz = ((g_SourceCharacterTexture1.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xyz;
    // 17: mul r4.xyz, cb0[1].xyzx, cb0[4].xxxx
    r4.xyz = ((source[1].xyzx)*(source[4].xxxx)).xyz;
    // 18: mul r4.xyz, r3.xyzx, r4.xyzx
    r4.xyz = ((r3.xyzx)*(r4.xyzx)).xyz;
    // 19: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = ((g_SourceCharacterTexture0.SampleBias(SourceCharacterSampler, (r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x)).xyzw).xy;
    // 20: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 21: dp2 r1.w, r2.xyxx, r2.xyxx
    r1.w = (dot((r2.xyxx).xy,(r2.xyxx).xy).xxxx).w;
    // 22: add r1.w, -r1.w, l(1.000000)
    r1.w = ((-(r1.wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 23: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 24: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 25: add r5.z, r1.w, l(0.000010)
    r5.z = ((r1.wwww)+(float4(0.000010,0.000010,0.000010,0.000010))).z;
    // 26: mul r2.xy, r2.xyxx, cb0[3].wwww
    r2.xy = ((r2.xyxx)*(source[3].wwww)).xy;
    // 27: mul r5.xy, r2.xyxx, v2.wwww
    r5.xy = ((r2.xyxx)*(v2.wwww)).xy;
    // 28: dp3 r1.w, r5.xyzx, r5.xyzx
    r1.w = (dot((r5.xyzx).xyz,(r5.xyzx).xyz).xxxx).w;
    // 29: sqrt r1.w, r1.w
    r1.w = (sqrt(r1.wwww)).w;
    // 30: div r2.xyz, r5.xyzx, r1.wwww
    r2.xyz = ((r5.xyzx)/(r1.wwww)).xyz;
    // 31: dp3 r1.w, r2.xyzx, r0.yzwy
    r1.w = (dot((r2.xyzx).xyz,(r0.yzwy).xyz).xxxx).w;
    // 32: max r1.w, r1.w, l(0.000000)
    r1.w = (max(r1.wwww,float4(0.000000,0.000000,0.000000,0.000000))).w;
    // 33: min r2.w, r1.w, l(1.000000)
    r2.w = (min(r1.wwww,float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 34: mul r5.xyz, cb0[2].xyzx, cb0[4].yyyy
    r5.xyz = ((source[2].xyzx)*(source[4].yyyy)).xyz;
    // 35: mul r3.xyz, r3.xyzx, r5.xyzx
    r3.xyz = ((r3.xyzx)*(r5.xyzx)).xyz;
    // 36: mad r0.xyz, v7.xyzx, r0.xxxx, r0.yzwy
    r0.xyz = ((v7.xyzx)*(r0.xxxx)+(r0.yzwy)).xyz;
    // 37: dp3 r0.w, r0.xyzx, r0.xyzx
    r0.w = (dot((r0.xyzx).xyz,(r0.xyzx).xyz).xxxx).w;
    // 38: sqrt r0.w, r0.w
    r0.w = (sqrt(r0.wwww)).w;
    // 39: div r0.xyz, r0.xyzx, r0.wwww
    r0.xyz = ((r0.xyzx)/(r0.wwww)).xyz;
    // 40: dp3 r0.x, r0.xyzx, r2.xyzx
    r0.x = (dot((r0.xyzx).xyz,(r2.xyzx).xyz).xxxx).x;
    // 41: lt r0.y, |r0.x|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 42: log r0.x, |r0.x|
    r0.x = (log2(abs(r0.xxxx))).x;
    // 43: mul r0.x, r0.x, cb0[4].z
    r0.x = ((r0.xxxx)*(source[4].zzzz)).x;
    // 44: exp r0.x, r0.x
    r0.x = (exp2(r0.xxxx)).x;
    // 45: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 46: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 47: mul r0.xyz, r3.xyzx, r0.xxxx
    r0.xyz = ((r3.xyzx)*(r0.xxxx)).xyz;
    // 48: mad r0.xyz, r2.wwww, r4.xyzx, r0.xyzx
    r0.xyz = ((r2.wwww)*(r4.xyzx)+(r0.xyzx)).xyz;
    // 49: mul r2.xyz, r1.wwww, cb2[3].xyzx
    r2.xyz = ((r1.wwww)*(passValues[3].xyzx)).xyz;
    // 50: mad r0.xyz, r0.xyzx, cb2[3].wwww, r2.xyzx
    r0.xyz = ((r0.xyzx)*(passValues[3].wwww)+(r2.xyzx)).xyz;
    // 51: mul r0.xyz, r1.xyzx, r0.xyzx
    r0.xyz = ((r1.xyzx)*(r0.xyzx)).xyz;
    // 52: mul o0.xyz, r0.xyzx, cb0[5].xyzx
    output.targets[0].xyz = ((r0.xyzx)*(source[5].xyzx)).xyz;
    // 53: mov o0.w, l(0)
    output.targets[0].w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    // 54: mov o1.xyzw, l(0,0,0,0)
    output.targets[1].xyzw = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).xyzw;
    // 55: ret
    return output;
}

// source.character.static-map-native-1400.v1 / source program 2cf61135d30a0348afb60d4bfc1966ef
